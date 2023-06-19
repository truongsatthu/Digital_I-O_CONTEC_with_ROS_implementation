////////////////////////////////////////////////////////////////////////////////
/// @file   cgetres_usb.c
/// @brief  API-DIO(LNX) config(USB) : USB driver resource acquisition source file
/// @author &copy;CONTEC CO.,LTD.
/// @since  2017
////////////////////////////////////////////////////////////////////////////////
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/usb.h>
#include <linux/mutex.h>

#ifndef LINUX_VERSION_CODE
#include <linux/version.h>
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
#include <linux/smp_lock.h>
#endif

#include "cgetres_usb.h"

//================================================================
///@name Debug macro
// @{
//================================================================
//#define DEBUG							///< Debug print on/off
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,6))

#ifdef DEBUG
#define	PRINT_DBG(fmt, args...)	printk(__FILE__ "(%d)%s:" fmt, __LINE__, __FUNCTION__, ## args);	///< Debug log
#define	PRINT_ERR(fmt, args...)	printk(__FILE__ "(%d)%s:" fmt, __LINE__, __FUNCTION__, ## args);	///< Error log
#define	PRINT_INF(fmt, args...)	printk(__FILE__ "(%d)%s:" fmt, __LINE__, __FUNCTION__, ## args);	///< Information log
#else
#define	PRINT_DBG(fmt, args...)																		///< Debug log
#define	PRINT_ERR(fmt, args...)	printk(__FILE__ "(%d)%s:" fmt, __LINE__, __FUNCTION__, ## args);	///< Error log
#define	PRINT_INF(fmt, args...)	printk(__FILE__ "(%d)%s:" fmt, __LINE__, __FUNCTION__, ## args);	///< Information log
#endif

#else
#define	PRINT_DBG(fmt, args...)
#define	PRINT_ERR(fmt, args...)
#define	PRINT_INF(fmt, args...)
#endif
// @}

//================================================================
///@name Definition
// @{
//================================================================
#define CONTEC_VENDOR_USB		0x06CE				///< CONTEC's device vender ID

#define CGETRES_ENTRY_NAME		"cgetres_usb"		///< Entry name
#define CGETRES_USB_MINOR_BASE	0					///< Minor number

// our private defines. if this grows any larger, use your own .h file
#define MAX_TRANSFER			(PAGE_SIZE - 512)	///< Maximum number of packets
#define WRITES_IN_FLIGHT		8					///< arbitrarily chosen
//@}

//----------------------------------------------------------------
///@name Define these values to match CONTEC's device
//@{
//----------------------------------------------------------------
/// Device table
static struct usb_device_id cgetres_usb_device_table[] = {
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA332) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA302) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA3D2) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA322) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA3E2) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA392) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA3C2) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA3F2) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA352) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA382) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA312) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA362) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA3A2) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA372) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA342) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA3B2) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xB302) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0x830D) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0x831D) },
	{}	/* Terminating entry */
};
/// Device table registration
MODULE_DEVICE_TABLE(usb, cgetres_usb_device_table);
//@}

//----------------------------------------------------------------
///@name Structure to hold all of our device specific stuff
//@{
//----------------------------------------------------------------
struct cgetres_usb {
	struct usb_device *		udev;						///< the usb device for this device
	struct usb_interface *	interface;					///< the interface for this device
	struct semaphore		limit_sem;					///< limiting the number of writes in progress
	struct usb_anchor   	submitted;          		///< in case we need to retract our submissions
	struct urb *			bulk_in_urb;				///< the urb to read data with
	unsigned char *			bulk_in_buffer;				///< the buffer to receive data
	size_t					bulk_in_size;				///< the size of the receive buffer
	size_t      			bulk_in_filled;             ///< number of bytes in the buffer
	size_t      			bulk_in_copied;         	///< already copied to user space
	__u8					bulk_in_endpointAddr;		///< the address of the bulk in endpoint
	__u8					bulk_out_endpointAddr;		///< the address of the bulk out endpoint
	int         			errors;                 	///< the last request tanked
	bool            		ongoing_read;               ///< a read is going on
	spinlock_t      		err_lock;               	///< lock for errors
	struct kref				kref;						///< kref structure
	struct mutex        	io_mutex;           		///< synchronize I/O with disconnect
	wait_queue_head_t		bulk_in_wait;				///< to wait for an ongoing read
};
#define to_cgetres_usb_dev(d) container_of(d, struct cgetres_usb, kref)	///< Get start address
//@}

static struct usb_driver cgetres_usb_driver;

//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Delete entry
/// @param	kref				kref structure
/// @return	none
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
static void cgetres_usb_delete(struct kref *kref)
{
	struct cgetres_usb *dev = to_cgetres_usb_dev(kref);

	PRINT_DBG("call cgetres_usb_delete\n");

	usb_free_urb(dev->bulk_in_urb);
	usb_put_dev(dev->udev);
	kfree(dev->bulk_in_buffer);
	kfree(dev);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Open entry
/// @param	inode				i-node management structure
/// @param	file				file management structure
/// @return	function result <br>
/// 			0: Success <br>
/// 			Less than 0: Error (-errno)
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
static int cgetres_usb_open(struct inode *inode, struct file *file)
{
	struct cgetres_usb *dev;
	struct usb_interface *interface;
	int subminor;
	int retval = 0;

	subminor = iminor(inode);
	PRINT_DBG("cgetres_usb_open subminor = %d\n", subminor);

	interface = usb_find_interface(&cgetres_usb_driver, subminor);
	if (!interface) {
		PRINT_ERR("error, can't find device for minor %d\n", subminor);
		retval = -ENODEV;
		goto EXIT;
	}

	dev = usb_get_intfdata(interface);
	if (!dev) {
		retval = -ENODEV;
		goto EXIT;
	}

#if 0
	retval = usb_autopm_get_interface(interface);
	if (retval)
		goto EXIT;
#endif
	
	/* increment our usage count for the device */
	kref_get(&dev->kref);

	/* save our object in the file's private structure */  
	file->private_data = dev; 
	
EXIT:
	return retval;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Release entry
/// @param	inode				i-node management structure
/// @param	file				file management structure
/// @return	function result <br>
/// 			0: Success <br>
/// 			Less than 0: Error (-errno)
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
static int cgetres_usb_release(struct inode *inode, struct file *file)
{
	struct cgetres_usb *dev;

	PRINT_DBG("call cgetres_usb_release\n");

	dev = (struct cgetres_usb *)file->private_data;
	if (dev == NULL) {
		return -ENODEV;
	}

#if 0
	/* allow the device to be autosuspended */
	mutex_lock(&dev->io_mutex);
	if (dev->interface)
		usb_autopm_put_interface(dev->interface);
	mutex_unlock(&dev->io_mutex);
#endif

	/* decrement the count on our device */
	kref_put(&dev->kref, cgetres_usb_delete);
	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	I/O Control entry
/// @param	file				file management structure
/// @param	cmd					ioctl command code
/// @param	arg					ioctl parameter data
/// @return	function result <br>
/// 			0: Success <br>
/// 			Less than 0: Error (-errno)
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
static long cgetres_usb_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	I/O Control entry
/// @param	inode				i-node management structure
/// @param	file				file management structure
/// @param	cmd					ioctl command code
/// @param	arg					ioctl parameter data
/// @return	function result <br>
/// 			0: Success <br>
/// 			Less than 0: Error (-errno)
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
static int  cgetres_usb_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
	struct cgetres_usb				*dev = NULL;
	struct usb_interface			*interface;
	int								device_major;
	int								sub_device_minor;
	GETUSBRES						param_res;
	struct usb_device_descriptor	*desc = NULL;
	long							lret = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
	long							retval = 0;
#else
	int								retval = 0;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
	device_major = MAJOR(file->f_inode->i_rdev);
	sub_device_minor = iminor(file->f_inode);
#else
	device_major = MAJOR(inode->i_rdev);
	sub_device_minor = iminor(inode);
#endif

	interface = usb_find_interface(&cgetres_usb_driver, sub_device_minor);
	if (!interface) {
		PRINT_ERR("ioctl usb_find_interface error.\n");
		retval = -ENODEV;
		return retval;
	}

	dev = usb_get_intfdata(interface);
	if (!dev) {
		PRINT_ERR("ioctl usb_get_intfdata error.\n");
		retval = -ENODEV;
		return retval;
	}

	//--------------------------------------
	// Specified command execution
	//--------------------------------------
	switch (cmd) {
	case CGETRES_IOC_USB:
		//----------------------------------------
		// Resource information acquisition
		//----------------------------------------
		desc = kmalloc(sizeof(*desc), GFP_NOIO);
		if (desc == NULL) {
			retval = -ENODEV;
			PRINT_ERR("kmalloc error = [%d]\n", ENODEV);
			return retval;
		}

		retval = usb_get_descriptor(dev->udev,
									USB_DT_DEVICE,
									0x0,
									desc, sizeof(*desc));
		if (retval >= 0) {
			param_res.vendor_id = le16_to_cpu(desc->idVendor);
			param_res.device_id = le16_to_cpu(desc->idProduct);
			usb_string(dev->udev, desc->iSerialNumber, param_res.serial_id, sizeof(param_res.serial_id));
			usb_string(dev->udev, desc->iProduct, param_res.device_name, sizeof(param_res.device_name));
			PRINT_DBG("vendor_id   [0x%04x]\n", param_res.vendor_id);
			PRINT_DBG("device_id   [0x%04x]\n", param_res.device_id);
			PRINT_DBG("serial_id   [%s]\n", param_res.serial_id);
			PRINT_DBG("device_name [%s]\n", param_res.device_name);

			if (!(param_res.vendor_id == CONTEC_VENDOR_USB)) {
				PRINT_ERR("vendor_id mismatch : errcord[%d]\n", ENODEV);
				retval = -ENODEV;
			} else {
				retval = 0;
			}
		} else {
			PRINT_ERR("usb_get_descriptor error = [%d]\n", ENODEV);
			retval = -ENODEV;
		}
		kfree(desc);

		param_res.ret = (long)retval;
		lret = copy_to_user((PGETUSBRES)arg, &param_res, sizeof(GETUSBRES));
		if (0 != lret) {
			retval = -ENODEV;
		}
		break;
	default:
		break;
	}

	return retval;
}
//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Read callback entry
/// @param	urb					URB structure pointer
/// @return	none
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
static void cgetres_usb_read_bulk_callback(struct urb *urb)  
{  
    struct cgetres_usb *dev;  
  
    dev = urb->context;  
  
    spin_lock(&dev->err_lock);  
    /* sync/async unlink faults aren't errors */  
    if (urb->status) {
        if (!(urb->status == -ENOENT ||  
            urb->status == -ECONNRESET ||  
            urb->status == -ESHUTDOWN))  
			PRINT_ERR("%s - nonzero write bulk status received: %d",  
                __func__, urb->status);				
  
        dev->errors = urb->status;  
    } else {  
        dev->bulk_in_filled = urb->actual_length;
    }  
    dev->ongoing_read = 0;
    spin_unlock(&dev->err_lock);
  
    wake_up_interruptible(&dev->bulk_in_wait);
}  
//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Read sub entry
/// @param	dev					our device specific stuff structure
/// @param	count				size of data to read
/// @return	function result <br>
/// 			0: Success <br>
/// 			Less than 0: Error (-errno)
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
static int cgetres_usb_do_read_io(struct cgetres_usb *dev, size_t count)  
{  
	int rv;  
      
	/* prepare a read */  
	usb_fill_bulk_urb(dev->bulk_in_urb,
		dev->udev,
		usb_rcvbulkpipe(dev->udev,
			dev->bulk_in_endpointAddr),
		dev->bulk_in_buffer,  
        min(dev->bulk_in_size, count),
		cgetres_usb_read_bulk_callback,
		dev);
	/* tell everybody to leave the URB alone */  
	spin_lock_irq(&dev->err_lock);  
	dev->ongoing_read = 1;
	spin_unlock_irq(&dev->err_lock);  

	/* submit bulk in urb, which means no data to deliver */
	dev->bulk_in_filled = 0;
	dev->bulk_in_copied = 0;

	/* do it */
	rv = usb_submit_urb(dev->bulk_in_urb, GFP_KERNEL);
	if (rv < 0) {  
		PRINT_ERR("%s - failed submitting read urb, error %d",  
			__func__, rv);		
		dev->bulk_in_filled = 0;  
		rv = (rv == -ENOMEM) ? rv : -EIO;  
		spin_lock_irq(&dev->err_lock);  
		dev->ongoing_read = 0;  
		spin_unlock_irq(&dev->err_lock);  
	}  
	return rv;  
}
//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Read entry
/// @param	file				file management structure
/// @param	buffer				buffer of data to read
/// @param	count				size of data to read
/// @param	ppos				offset
/// @return	function result <br>
/// 			0~: actual size read <br>
/// 			Less than 0: Error (-errno)
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
static ssize_t cgetres_usb_read(struct file *file, char *buffer, size_t count, loff_t *ppos)  
{  
    struct cgetres_usb *dev;  
    int rv;  
    bool ongoing_io;
    unsigned short data_total_len = 0;
    int total_received = 0;
    int package_num = 0;
  
    dev = file->private_data;
  
    /* if we cannot read at all, return EOF */  
    if (!dev->bulk_in_urb || !count)
        return 0;  
  
    /* no concurrent readers */  
    rv = mutex_lock_interruptible(&dev->io_mutex);  
    if (rv < 0)  
        return rv;  
  
    if (!dev->interface) {       /* disconnect() was called */  
        rv = -ENODEV;  
        goto exit;  
    }  
  
    /* if IO is under way, we must not touch things */  
retry:  
    spin_lock_irq(&dev->err_lock);  
    ongoing_io = dev->ongoing_read;  
    spin_unlock_irq(&dev->err_lock);  
  
    if (ongoing_io) {
        /* nonblocking IO shall not wait */  
        if (file->f_flags & O_NONBLOCK) {  
            rv = -EAGAIN;  
            goto exit;  
        }  
        /* 
         * IO may take forever 
         * hence wait in an interruptible state 
         */  
        rv = wait_event_interruptible(dev->bulk_in_wait, (!dev->ongoing_read));
        if (rv < 0)  
            goto exit;
    }
  
    /* errors must be reported */  
    rv = dev->errors;  
    if (rv < 0) {  
        /* any error is reported once */  
        dev->errors = 0;  
        /* to preserve notifications about reset */  
        rv = (rv == -EPIPE) ? rv : -EIO;
        /* report it */  
        goto exit;  
    }  
  
    /* 
     * if the buffer is filled we may satisfy the read 
     * else we need to start IO 
     */  
  
    if (dev->bulk_in_filled) {
        /* we had read data */  
        size_t available = dev->bulk_in_filled - dev->bulk_in_copied;  
        size_t chunk = min(available, count);
  
        if (!available) {  
            /* 
             * all data has been used 
             * actual IO needs to be done 
             */  
            rv = cgetres_usb_do_read_io(dev, count);  
            if (rv < 0)  
                goto exit;  
            else  
                goto retry;  
        }  
        /* 
         * data is available 
         * chunk tells us how much shall be copied 
         */  
        if (copy_to_user(buffer + total_received,  
                 dev->bulk_in_buffer + dev->bulk_in_copied,  
                 chunk))  
            rv = -EFAULT;  
        else  
            rv = chunk;  
  
        dev->bulk_in_copied += chunk;
  
        // total data length
        if (package_num == 0){
        	data_total_len = (unsigned short)((dev->bulk_in_buffer[3] << 8) + (dev->bulk_in_buffer[2] & 0x00FF) + 4);
        }
        // total received number
        total_received += dev->bulk_in_filled;
        if (total_received < data_total_len)
		{
			// ask for the next package
            package_num ++;
            rv = cgetres_usb_do_read_io(dev, count - total_received);  
            if (rv < 0)  
                goto exit;  
            else if (!(file->f_flags & O_NONBLOCK))  
                goto retry;
		}
	
        /* 
         * if we are asked for more than we have, 
         * we start IO but don't wait 
         */
        if (total_received < count)
        	cgetres_usb_do_read_io(dev, count - total_received);

    } else {  
        /* no data in the buffer */  
        rv = cgetres_usb_do_read_io(dev, count);  
        if (rv < 0)  
            goto exit;  
        else if (!(file->f_flags & O_NONBLOCK))  
            goto retry;
    }  
exit:  
    mutex_unlock(&dev->io_mutex);  
    return rv;  
}
//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Write callback entry
/// @param	urb					URB structure pointer
/// @return	none
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
static void cgetres_usb_write_bulk_callback(struct urb *urb)  
{  
    struct cgetres_usb *dev;  
  
    dev = urb->context;  
  
    /* sync/async unlink faults aren't errors */  
    if (urb->status) {  
        if (!(urb->status == -ENOENT ||  
            urb->status == -ECONNRESET ||  
            urb->status == -ESHUTDOWN)) 
			PRINT_ERR("%s - nonzero write bulk status received: %d",  
				__func__, urb->status);
  
        spin_lock(&dev->err_lock);  
        dev->errors = urb->status;  
        spin_unlock(&dev->err_lock);  
    }  
  
    /* free up our allocated buffer */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
    usb_free_coherent(urb->dev, urb->transfer_buffer_length,  
              urb->transfer_buffer, urb->transfer_dma);
#else
	usb_buffer_free(urb->dev, urb->transfer_buffer_length,  
              urb->transfer_buffer, urb->transfer_dma);
#endif
    up(&dev->limit_sem);  
} 
//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Write entry
/// @param	file				file management structure
/// @param	buffer				buffer of data to write
/// @param	count				size of data to write
/// @param	ppos				offset
/// @return	function result <br>
/// 			0~: actual size write <br>
/// 			Less than 0: Error (-errno)
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
static ssize_t cgetres_usb_write(struct file *file, const char *user_buffer, size_t count, loff_t *ppos)  
{  
    struct cgetres_usb *dev;  
    int retval = 0;  
    struct urb *urb = NULL;  
    char *buf = NULL;  
    size_t writesize = min(count, (size_t)MAX_TRANSFER);
  
    dev = file->private_data;  
  
    /* verify that we actually have some data to write */  
    if (count == 0)  
        goto exit;  
  
    /* 
     * limit the number of URBs in flight to stop a user from using up all 
     * RAM 
     */  
    if (!(file->f_flags & O_NONBLOCK)) { 
        if (down_interruptible(&dev->limit_sem)) { 
            retval = -ERESTARTSYS;  
            goto exit;  
        }  
    } else {  
        if (down_trylock(&dev->limit_sem)) {  
            retval = -EAGAIN;  
            goto exit;  
        }  
    }  
  
    spin_lock_irq(&dev->err_lock); 
    retval = dev->errors;  
    if (retval < 0) {  
        /* any error is reported once */  
        dev->errors = 0;  
        /* to preserve notifications about reset */  
        retval = (retval == -EPIPE) ? retval : -EIO;  
    }  
    spin_unlock_irq(&dev->err_lock);
    if (retval < 0)  
        goto error;  
  
    /* create a urb, and a buffer for it, and copy the data to the urb */  
    urb = usb_alloc_urb(0, GFP_KERNEL);
    if (!urb) {  
        retval = -ENOMEM;  
        goto error;  
    }  

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
    buf = usb_alloc_coherent(dev->udev, writesize, GFP_KERNEL,  
                 &urb->transfer_dma);
#else
	buf = usb_buffer_alloc(dev->udev, writesize, GFP_KERNEL,  
                 &urb->transfer_dma);
#endif
    if (!buf) {  
        retval = -ENOMEM;  
        goto error;  
    }
	
    if (copy_from_user(buf, user_buffer, writesize)) {  
        retval = -EFAULT;  
        goto error;  
    }  
  
    /* this lock makes sure we don't submit URBs to gone devices */  
    mutex_lock(&dev->io_mutex);  
    if (!dev->interface) {       /* disconnect() was called */  
        mutex_unlock(&dev->io_mutex);  
        retval = -ENODEV;  
        goto error;  
    }  
  
    /* initialize the urb properly */  
    usb_fill_bulk_urb(urb, dev->udev,  
              // usb_sndbulkpipe(dev->udev, dev->bulk_out_endpointAddr),  
              usb_sndbulkpipe(dev->udev, 0x01),
              buf, writesize, cgetres_usb_write_bulk_callback, dev);
    urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;
    usb_anchor_urb(urb, &dev->submitted);  
  
    /* send the data out the bulk port */  
    retval = usb_submit_urb(urb, GFP_KERNEL);
    mutex_unlock(&dev->io_mutex);  
    if (retval) {  
		PRINT_ERR("%s - failed submitting write urb, error %d", __func__,  
            retval);			
        goto error_unanchor;  
    }  
  
    /* 
     * release our reference to this urb, the USB core will eventually free 
     * it entirely 
     */  
    usb_free_urb(urb);  
  
  
    return writesize;  
  
error_unanchor:  
    usb_unanchor_urb(urb);  
error:  
    if (urb) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
        usb_free_coherent(dev->udev, writesize, buf, urb->transfer_dma);  
#else
		usb_buffer_free(dev->udev, writesize, buf, urb->transfer_dma);  
#endif
        usb_free_urb(urb);  
    }  
    up(&dev->limit_sem);  
  
exit:  
    return retval;  
}

//----------------------------------------------------------------
/// File operation structure (function entry point)
//----------------------------------------------------------------
static struct file_operations cgetres_usb_fops = {
	.owner			= THIS_MODULE,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
	.unlocked_ioctl	= cgetres_usb_ioctl,
#else
	.ioctl			= cgetres_usb_ioctl,
#endif
	.open			= cgetres_usb_open,
	.release		= cgetres_usb_release,
	.read			= cgetres_usb_read,
	.write			= cgetres_usb_write,
};

//----------------------------------------------------------------
/// USB Class structure
/**
    usb class driver info in order to get a minor number from the usb core,
    and to have the device registered with the driver core
**/
//----------------------------------------------------------------
static struct usb_class_driver cgetres_usb_class = {
	.name		= "cdiousb%03d",
	.fops		= &cgetres_usb_fops,
	.minor_base	= CGETRES_USB_MINOR_BASE,
};

//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Probe entry
/// @param	interface		USB interface
/// @param	id				USB device ID
/// @return	function result <br>
/// 			0: Success  <br>
/// 			Less than 0: Error (-errno)
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
static int cgetres_usb_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct cgetres_usb				*dev = NULL;
	struct usb_host_interface		*iface_desc;
	struct usb_endpoint_descriptor	*endpoint;
	size_t							buffer_size;
	int								i;
	int								retval = -ENOMEM;

	PRINT_DBG("cgetres_usb_probe call.\n");

	// allocate memory for our device state and initialize it
	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (dev == NULL) {
		PRINT_ERR("kzalloc : error code[%d]\n", ENOMEM);
		goto ERROR;
	}
	kref_init(&dev->kref);
	sema_init(&dev->limit_sem, WRITES_IN_FLIGHT);
	mutex_init(&dev->io_mutex);
	spin_lock_init(&dev->err_lock);
	init_usb_anchor(&dev->submitted);
	init_waitqueue_head(&dev->bulk_in_wait);

	dev->udev = usb_get_dev(interface_to_usbdev(interface));
	dev->interface = interface;

	// set up the endpoint information
	// use only the first bulk-in and bulk-out endpoints
	iface_desc = interface->cur_altsetting;
	for (i = 0; i < iface_desc->desc.bNumEndpoints; ++i) {
		endpoint = &iface_desc->endpoint[i].desc;

		if (!dev->bulk_in_endpointAddr &&
			((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK)
			== USB_DIR_IN) &&
			((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
			== USB_ENDPOINT_XFER_BULK)) {
			// we found a bulk in endpoint
			buffer_size = le16_to_cpu(endpoint->wMaxPacketSize);
			dev->bulk_in_size = buffer_size;
			dev->bulk_in_endpointAddr = endpoint->bEndpointAddress;
			dev->bulk_in_buffer = kmalloc(buffer_size, GFP_KERNEL);
			if (!dev->bulk_in_buffer) {
				PRINT_ERR("Could not allocate bulk_in_buffer.\n");
				goto ERROR;
			}
			dev->bulk_in_urb = usb_alloc_urb(0, GFP_KERNEL);
			if (!dev->bulk_in_urb) {
				PRINT_ERR("Could not allocate bulk_in_urb\n");
				goto ERROR;
			}
		}

		if (!dev->bulk_out_endpointAddr &&
			((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK)
			== USB_DIR_OUT) &&
			((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
			== USB_ENDPOINT_XFER_BULK)) {
			// we found a bulk out endpoint
			dev->bulk_out_endpointAddr = endpoint->bEndpointAddress;
		}
	}
	if (!(dev->bulk_in_endpointAddr && dev->bulk_out_endpointAddr)) {
		PRINT_ERR("Could not find both bulk-in and bulk-out endpoints.\n");
		goto ERROR;
	}

	// save our data pointer in this interface device
	usb_set_intfdata(interface, dev);

	// we can register the device now, as it is ready
	retval = usb_register_dev(interface, &cgetres_usb_class);
	if (retval) {
		// something prevented us from registering this driver
		PRINT_ERR("Not able to get a minor for this device.\n");
		usb_set_intfdata(interface, NULL);
		goto ERROR;
	}

	// let the user know what node this device is now attached to
	PRINT_INF("#%d now attached.\n", interface->minor);
	return 0;

ERROR:
	if (dev) {
		kref_put(&dev->kref, cgetres_usb_delete);
	}
	return retval;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Disconnect entry
/// @param	interface		USB interface
/// @return	none
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
static void cgetres_usb_disconnect(struct usb_interface *interface)
{
	struct cgetres_usb	*dev;
	int					minor = interface->minor;

	dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);

	// give back our minor
	usb_deregister_dev(interface, &cgetres_usb_class);

	/* prevent more I/O from starting */  
	mutex_lock(&dev->io_mutex);
	dev->interface = NULL;
	mutex_unlock(&dev->io_mutex);
	
	usb_kill_anchored_urbs(&dev->submitted);

	// decrement our usage count
	kref_put(&dev->kref, cgetres_usb_delete);

	PRINT_INF("#%d now disconnected.\n", minor);
}

//----------------------------------------------------------------
/// USB driver structure
//----------------------------------------------------------------
static struct usb_driver cgetres_usb_driver = {
	.name		= CGETRES_ENTRY_NAME,
	.probe		= cgetres_usb_probe,
	.disconnect	= cgetres_usb_disconnect,
	.id_table	= cgetres_usb_device_table,
};

//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Load module
/// @param	none
/// @return	function result <br>
/// 			0: Success  <br>
/// 			Less than 0: Error (-errno)
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
static int __init cgetres_usb_init(void)
{
	int	result;

	// register this driver with the USB subsystem
	result = usb_register(&cgetres_usb_driver);
	if (result) {
		PRINT_ERR("usb_register failed. Error number=[%d]\n", result);
	}
	PRINT_DBG("module_init : call cgetres_usb_init.\n");
	return result;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Unload module
/// @param	none
/// @return	none
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
static void __exit cgetres_usb_exit(void)
{
	// deregister this driver with the USB subsystem
	usb_deregister(&cgetres_usb_driver);
	PRINT_DBG("module_exit : call cgetres_usb_exit.\n");
}

module_init(cgetres_usb_init);
module_exit(cgetres_usb_exit);

MODULE_DESCRIPTION("API-DIO(LNX) get USB Device resource");
MODULE_AUTHOR("CONTEC CO.,LTD.");
MODULE_VERSION("1.0");
MODULE_LICENSE("GPL");
