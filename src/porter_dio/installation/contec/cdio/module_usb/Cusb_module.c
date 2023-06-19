////////////////////////////////////////////////////////////////////////////////
/// @file   Cusb_module.c
/// @brief  API-DIO(LNX) USB Module - USB driver source file
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
#include <linux/poll.h> 
#include <linux/usb.h>

#ifndef LINUX_VERSION_CODE
#include <linux/version.h>
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
#include <linux/smp_lock.h>
#endif

#include "Cusb_module.h"
#include "Cusb_ctrl.h"
#include "Cusb_firm.h"

//================================================================
///@name Define these values to match CONTEC's device
//@{
//================================================================
/// Device table
static struct usb_device_id cdio_usb_device_table[] = {
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA302) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA312) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA322) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA332) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA342) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA352) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA362) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA372) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA382) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA392) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA3A2) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA3B2) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA3C2) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA3D2) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA3E2) },
	{ USB_DEVICE(CONTEC_VENDOR_USB, 0xA3F2) },
	{}	// Terminating entry
};
/// Device table registration
MODULE_DEVICE_TABLE(usb, cdio_usb_device_table);
// @}

//================================================================
// External variable
//================================================================
static struct usb_driver	cdio_usb_driver;

//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Poll entry
/// @param	file				file management structure
/// @param	wait				polling table
/// @return	function result <br>
/// 			0: Success
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
static unsigned int Cusb_poll(struct file* file, poll_table* wait)
{
	unsigned int		mask = 0;
	struct cdio_usb		*dev;

	PRINT_DBG("call Cusb_poll\n");

	dev = (struct cdio_usb *)file->private_data;
	if (dev == NULL) {
		PRINT_ERR("error code[%d]\n", ENODEV);
		return -ENODEV;
	}

	// waiting for interrupt callback reception
	poll_wait(file, &dev->int_wait, wait);

	if(atomic_read(&dev->int_flag) == 1) {
		mask = POLLIN | POLLRDNORM;
	}

	return mask;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Delete entry
/// @param	kref				kref structure
/// @return	none
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
static void Cusb_delete(struct kref *kref)
{
	struct cdio_usb *dev = to_cdiousb_dev(kref);

	PRINT_DBG("call Cusb_delete\n");

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
static int Cusb_open(struct inode *inode, struct file *file)
{
	struct cdio_usb *dev;
	struct usb_interface *interface;
	int subminor;

	subminor = iminor(inode);

	PRINT_DBG("call Cusb_open\n");

	interface = usb_find_interface(&cdio_usb_driver, subminor);
	if (!interface) {
		PRINT_ERR("usb_find_interface : error code[%d]\n", ENODEV);
		return -ENODEV;
	}

	dev = usb_get_intfdata(interface);
	if (!dev) {
		PRINT_ERR("usb_get_intfdata : error code[%d]\n", ENODEV);
		return -ENODEV;
	}

	//----------------------------------------
	// increment our usage count for the device
	//----------------------------------------
	kref_get(&dev->kref);

	//----------------------------------------
	// save our object in the file's private structure
	//----------------------------------------
	file->private_data = dev;

	return 0;
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
static int Cusb_release(struct inode *inode, struct file *file)
{
	struct cdio_usb *dev;

	PRINT_DBG("call Cusb_release\n");

	dev = (struct cdio_usb *)file->private_data;
	if (dev == NULL) {
		PRINT_ERR("error code[%d]\n", ENODEV);
		return -ENODEV;
	}

	//----------------------------------------
	// decrement the count on our device
	//----------------------------------------
	kref_put(&dev->kref, Cusb_delete);
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
static long Cusb_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
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
static int Cusb_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
#endif
{
	struct cdio_usb					*dev = NULL;
	struct usb_interface			*interface = NULL;
	int								device_major;
	int								sub_device_minor;
	int								retval = 0;
	long							lret = 0;
	CDIO_USB_CTRL					param_ctl;
	CDIO_USB_RES					param_res;
	struct usb_device_descriptor	*desc = NULL;

	//--------------------------------------
	// Retrieve major and minor numbers
	//--------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
	device_major = MAJOR(file->f_inode->i_rdev);
	sub_device_minor = iminor(file->f_inode);
#else
	device_major = MAJOR(inode->i_rdev);
	sub_device_minor = iminor(inode);
#endif

	//--------------------------------------
	// Retrieve USB device information from the system
	//--------------------------------------
	interface = usb_find_interface(&cdio_usb_driver, sub_device_minor);
	if (!interface) {
		PRINT_ERR("usb_find_interface : error code[%d]\n", ENODEV);
		return -ENODEV;
	}

	dev = usb_get_intfdata(interface);
	if (!dev) {
		PRINT_ERR("usb_get_intfdata : error code[%d]\n", ENODEV);
		return -ENODEV;
	}

	//--------------------------------------
	// Specified command execution
	//--------------------------------------
	switch (cmd) {
	case CDIO_IOC_USB_CTRL_IN:
		//----------------------------------------
		// Control transfer (in/read)
		//----------------------------------------
		lret = copy_from_user(&param_ctl, (PCDIO_USB_CTRL)arg, sizeof(CDIO_USB_CTRL));
		if (0 != lret) {
			PRINT_ERR("copy_from_user : error code[%d]\n", ENODEV);
			return -ENODEV;
		}

		retval = Cusb_firm_read_urb(dev,							// Device handle
									param_ctl.request,				// Control code of the action to be executed
									param_ctl.req_value,			// Request packet Value
									param_ctl.req_index,			// Request packet Index
									param_ctl.req_size,				// Data size
									param_ctl.req_data.data);		// Transfer buffer for data
		param_ctl.ret = (long)retval;

		lret = copy_to_user((PCDIO_USB_CTRL)arg, &param_ctl, sizeof(CDIO_USB_CTRL));
		if (0 != lret) {
			PRINT_ERR("copy_to_user : error code[%d]\n", ENODEV);
			return -ENODEV;
		}
		break;

	case CDIO_IOC_USB_CTRL_OUT:
		//----------------------------------------
		// Control transfer (out/write)
		//----------------------------------------
		lret = copy_from_user(&param_ctl, (PCDIO_USB_CTRL)arg, sizeof(CDIO_USB_CTRL));
		if (0 != lret) {
			PRINT_ERR("copy_from_user : error code[%d]\n", ENODEV);
			return -ENODEV;
		}

		retval = Cusb_firm_write_urb(dev,							// Device handle
									 param_ctl.request,				// Control code of the action to be executed
									 param_ctl.req_value,			// Request packet Value
									 param_ctl.req_index,			// Request packet Index
									 param_ctl.req_size,			// Data size
									 param_ctl.req_data.data);		// Transfer buffer for data
		param_ctl.ret = (long)retval;

		lret = copy_to_user((PCDIO_USB_CTRL)arg, &param_ctl, sizeof(CDIO_USB_CTRL));
		if (0 != lret) {
			PRINT_ERR("copy_to_user : error code[%d]\n", ENODEV);
			return -ENODEV;
		}
		break;

	case CDIO_IOC_USB_CTRL_INT:
		//----------------------------------------
		// Interrupt transfer
		//----------------------------------------
		lret = copy_from_user(&param_ctl, (PCDIO_USB_CTRL)arg, sizeof(CDIO_USB_CTRL));
		if (0 != lret) {
			PRINT_ERR("copy_from_user : error code[%d]\n", ENODEV);
			return -ENODEV;
		}

		retval = Cusb_firm_interrupt_urb(dev,						// Device handle
										 param_ctl.req_size,		// Data size
										 param_ctl.req_data.data);	// Transfer buffer for data
		param_ctl.ret = (long)retval;

		lret = copy_to_user((PCDIO_USB_CTRL)arg, &param_ctl, sizeof(CDIO_USB_CTRL));
		if (0 != lret) {
			PRINT_ERR("copy_to_user : error code[%d]\n", ENODEV);
			return -ENODEV;
		}
		break;

	case CDIO_IOC_USB_RES:
		//----------------------------------------
		// Resource information acquisition
		//----------------------------------------
		desc = kmalloc(sizeof(*desc), GFP_NOIO);
		if (desc == NULL) {
			PRINT_ERR("kmalloc : error code[%d]\n", ENODEV);
			return -ENODEV;
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
				PRINT_ERR("vendor_id mismatch : error code[%d]\n", ENODEV);
				retval = -ENODEV;
			} else {
				retval = 0;
			}
		} else {
			PRINT_ERR("usb_get_descriptor = [%d]\n", ENODEV);
			retval = -ENODEV;
		}
		kfree(desc);

		param_res.ret = (long)retval;
		lret = copy_to_user((PCDIO_USB_RES)arg, &param_res, sizeof(CDIO_USB_RES));
		if (0 != lret) {
			PRINT_ERR("copy_to_user : error code[%d]\n", ENODEV);
			retval = -ENODEV;
		}
		break;

	default:
		retval = -ENODEV;
		PRINT_ERR("unknown control code [%d]\n", cmd);
		break;
	}

	return retval;
}

//================================================================
/// File operation structure (function entry point)
//================================================================
static struct file_operations cdio_usb_fops = {
	.owner			= THIS_MODULE,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
	.unlocked_ioctl	= Cusb_ioctl,
#else
	.ioctl			= Cusb_ioctl,
#endif
	.poll			= Cusb_poll,
	.open			= Cusb_open,
	.release		= Cusb_release,
};

//================================================================
/// USB Class structure
//  usb class driver info in order to get a minor number from the usb core,
//  and to have the device registered with the driver core
//================================================================
static struct usb_class_driver cdio_usb_class = {
	.name		= "cdiousb%03d",
	.fops		= &cdio_usb_fops,
	.minor_base = CUSB_MINOR_BASE,
};

//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Probe entry
/// @param	interface	USB interface
/// @param	id			USB device ID
/// @return	function result <br>
/// 			0: Success  <br>
/// 			Less than 0: Error (-errno)
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
static int Cusb_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct cdio_usb					*dev = NULL;
	struct usb_host_interface		*iface_desc;
	struct usb_endpoint_descriptor	*endpoint;
	size_t							buffer_size;
	int								idx;
	int								retval = -ENOMEM;

	PRINT_DBG("Cusb_probe call.\n");

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (dev == NULL) {
		PRINT_ERR("kzalloc : error code[%d]\n", ENOMEM);
		return retval;
	}
	kref_init(&dev->kref);
	sema_init(&dev->limit_sem, WRITES_IN_FLIGHT);
	init_waitqueue_head(&dev->int_wait);
	atomic_set(&dev->int_flag, 0);

	dev->udev = usb_get_dev(interface_to_usbdev(interface));
	dev->interface = interface;

	//----------------------------------------
	// Set endpoint information
	//----------------------------------------
	iface_desc = interface->cur_altsetting;
	for (idx = 0; idx < iface_desc->desc.bNumEndpoints; ++idx) {
		endpoint = &iface_desc->endpoint[idx].desc;

		PRINT_DBG("[%d]\tbmAttributes=%d bEndpointAddress=%d bInterval=%d\n",
				idx, endpoint->bmAttributes, endpoint->bEndpointAddress, endpoint->bInterval);

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
				kref_put(&dev->kref, Cusb_delete);
				PRINT_ERR("kmalloc : error code[%d]\n", ENOMEM);
				return retval;
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

		if (!dev->int_in_endpointAddr &&
			((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK)
			== USB_DIR_IN) &&
			((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
			== USB_ENDPOINT_XFER_INT)) {
			// we found a interrupt in endpoint
			dev->int_in_endpointAddr = endpoint->bEndpointAddress;
			dev->int_in_interval = endpoint->bInterval;

			PRINT_DBG("[INT]\tbmAttributes=%d bEndpointAddress=%d bInterval=%d\n",
					endpoint->bmAttributes, endpoint->bEndpointAddress, endpoint->bInterval);
		}

		if (!dev->int_out_endpointAddr &&
			((endpoint->bEndpointAddress & USB_ENDPOINT_DIR_MASK)
			== USB_DIR_OUT) &&
			((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
			== USB_ENDPOINT_XFER_INT)) {
			// we found a interrupt out endpoint
			dev->int_out_endpointAddr = endpoint->bEndpointAddress;
			dev->int_out_interval = endpoint->bInterval;

			PRINT_DBG("[INT]\tbmAttributes=%d bEndpointAddress=%d bInterval=%d\n",
					endpoint->bmAttributes, endpoint->bEndpointAddress, endpoint->bInterval);
		}
	}
	if (!(dev->bulk_in_endpointAddr && dev->bulk_out_endpointAddr)) {
		kref_put(&dev->kref, Cusb_delete);
		PRINT_ERR("endpoint address failed : error code[%d]\n", ENOMEM);
		return retval;
	}

	//----------------------------------------
	// Save our data pointer in this interface device
	//----------------------------------------
	usb_set_intfdata(interface, dev);

	//----------------------------------------
	// Register USB device
	//----------------------------------------
	retval = usb_register_dev(interface, &cdio_usb_class);
	if (retval) {
		usb_set_intfdata(interface, NULL);
		if (dev) {
			kref_put(&dev->kref, Cusb_delete);
		}
		PRINT_ERR("usb_register_dev = [%d]\n", retval);
		return retval;
	}

	PRINT_INF("CDIO USB #%d now attached.\n", interface->minor);
	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Disconnect entry
/// @param	interface		USB interface
/// @return	none
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
static void Cusb_disconnect(struct usb_interface *interface)
{
	struct cdio_usb		*dev;
	int					minor = interface->minor;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
	spinlock_t			lock;
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
	spin_lock_init(&lock);
	spin_lock(&lock);
#else
	lock_kernel();
#endif
	dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);

	//----------------------------------------
	// Unregister USB device
	//----------------------------------------
	usb_deregister_dev(interface, &cdio_usb_class);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
	spin_unlock(&lock);
#else
	unlock_kernel();
#endif

	//----------------------------------------
	// Subtract reference count
	//----------------------------------------
	kref_put(&dev->kref, Cusb_delete);

	PRINT_INF("CDIO USB #%d now disconnected.\n", minor);
}

//================================================================
/// USB driver structure
//================================================================
static struct usb_driver cdio_usb_driver = {
	.name		= CUSB_ENTRY_NAME,
	.probe		= Cusb_probe,
	.disconnect	= Cusb_disconnect,
	.id_table	= cdio_usb_device_table,
};

//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Load module
/// @param	none
/// @return	function result <br>
/// 			0: Success  <br>
/// 			Less than 0: Error (-errno)
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
static int __init Cusb_init(void)
{
	int						ret = 0;

	//--------------------------------------
	// Register USB device
	//--------------------------------------
	ret = usb_register(&cdio_usb_driver);
	if (ret) {
		PRINT_ERR("usb_register = [%d]\n", ret);
	}

	PRINT_DBG("module_init : call Cusb_init.\n");
	return ret;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Unload module
/// @param	none
/// @return	none
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
static void __exit Cusb_exit(void)
{
	//--------------------------------------
	// Unregister USB device
	//--------------------------------------
	usb_deregister(&cdio_usb_driver);

	PRINT_DBG("module_exit : call Cusb_exit.\n");
}

module_init(Cusb_init);
module_exit(Cusb_exit);

MODULE_DESCRIPTION("API-DIO(LNX) USB Device Driver");
MODULE_AUTHOR("CONTEC CO.,LTD.");
MODULE_VERSION("6.50");
MODULE_LICENSE("GPL");
