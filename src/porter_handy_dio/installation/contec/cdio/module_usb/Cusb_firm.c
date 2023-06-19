////////////////////////////////////////////////////////////////////////////////
/// @file   Cusb_firm.c
/// @brief  API-DIO(LNX) USB Module - URB access source file
/// @author &copy;CONTEC CO.,LTD.
/// @since  2017
////////////////////////////////////////////////////////////////////////////////

#define __NO_VERSION__
#include <linux/module.h>
#include <linux/version.h>
#include <linux/fs.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0))
#include <linux/uaccess.h>
#else
#include <asm/io.h>
#include <asm/uaccess.h>
#endif
#include <linux/pci.h>		
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/completion.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/usb.h>

#include "Cusb_firm.h"

/// Completion callback structure
struct comp_cb {
	struct completion	comp_done;		///< Completion
	int					status;			///< URB status
	wait_queue_head_t	*int_wait;		///< Wait for an interrupt
	atomic_t			*int_flag;		///< interrupt flag
};

//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Receive URB interrupt callback
/// @param	urb		URB structure pointer
/// @param	regs	register storage method structure
/// @return	none
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
static void Cusb_firm_int_callback(struct urb *urb, struct pt_regs *regs)
{
	struct comp_cb *ctx = (struct comp_cb *)urb->context;

	// comple a transfer request for an endpoint
	ctx->status = urb->status;
	complete(&ctx->comp_done);

	// wake up
	wake_up_interruptible(ctx->int_wait);
	atomic_set(ctx->int_flag, 1);
	PRINT_DBG("call interrupt complete. status=[%d]\n", urb->status);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Receive URB completion notice
/// @param	urb		URB structure pointer
/// @param	regs	register storage method structure
/// @return	none
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
static void Cusb_firm_ctrl_completion(struct urb *urb, struct pt_regs *regs)
{
	struct comp_cb *ctx = (struct comp_cb *)urb->context;

	// comple a transfer request for an endpoint
	ctx->status = urb->status;
	complete(&ctx->comp_done);
	PRINT_DBG("call complete. status=[%d]\n", urb->status);
}

//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Start URB transfer and wait for completion
/// @param	urb				URB structure pointer
/// @param	timeout			timeout value
/// @param	actual_length	actual transferred size
/// @return	function result <br>
/// 			0: Success  <br>
/// 			Other than 0: URB error number
/// @note	Starts urb and waits for completion or timeout note that this call is NOT interruptible,<br>
///			while many device driver i/o requests should be interruptible.
//////////////////////////////////////////////////////////////////////////////////////////
static int Cusb_firm_start_wait_urb(struct urb *urb, int timeout, int* actual_length)
{
	struct comp_cb		*ctx = (struct comp_cb *)urb->context;
	unsigned long		expire;
	int					status = 0;

	PRINT_DBG("start : timeout = [%d]\n", timeout);

	// URB transmit
	urb->actual_length = 0;
	status = usb_submit_urb(urb, GFP_KERNEL);

	if (unlikely(status)) {
		goto EXIT;
	}

	expire = timeout ? msecs_to_jiffies(timeout) : MAX_SCHEDULE_TIMEOUT;
	if (!wait_for_completion_timeout(&ctx->comp_done, expire)) {
		// Timeout occurrence
		usb_kill_urb(urb);
		status = (ctx->status == -ENOENT ? -ETIMEDOUT : ctx->status);

		PRINT_DBG(
			"%s timed out on ep %d %s len=%u/%u : status=%d(%d)\n",
			current->comm,
			usb_endpoint_num(&urb->ep->desc),
			usb_urb_dir_in(urb) ? "IN" : "OUT",
			urb->actual_length,
			urb->transfer_buffer_length,
			status, ctx->status);
	} else {
		// Transmit completion
		status = ctx->status;
	}

EXIT:
	if (actual_length) {
		*actual_length = urb->actual_length;
	}
	PRINT_DBG("actual_length = [%d]\n", *actual_length);
	PRINT_DBG("end[%d]\n", status);

	return status;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Control transfer (URB function to write to device firmware)
/// @param	dev				URB structure pointer
/// @param	request			request code
/// @param	value			request value
/// @param	index			request index
/// @param	size			request data size
/// @param	data			request data pointer
/// @return	function result <br>
/// 			0: Success <br>
/// 			Other than 0: URB error number
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
long Cusb_firm_write_urb(struct cdio_usb *dev, __u8 request, __u16 value, __u16 index, __u16 size, void *data)
{
	struct urb				*write_urb = NULL;
	struct usb_ctrlrequest	*setup = NULL;
	long					retval = 0;
	int						length = 0;
	char					*trans_buf = NULL;
	struct comp_cb			comp_cb_param;
	void					*context = NULL;
#ifdef DEBUG // debug print
	int						idx = 0;
	short					work = 0;
	char					*pbuf = NULL;
#endif // debug print

	PRINT_DBG("start\n");
	PRINT_DBG("request[0x%04X]\n", request);
	PRINT_DBG("value  [0x%04x]\n", value);
	PRINT_DBG("index  [0x%04x]\n", index);
	PRINT_DBG("size   [%d]\n", size);

	// Buffer initialization
	setup = kmalloc(sizeof(struct usb_ctrlrequest), GFP_KERNEL);
	if (setup == NULL) {
		PRINT_ERR("kmalloc : errcord[%d]\n", ENOMEM);
		return -ENOMEM;
	}
	memset(setup, 0, sizeof(struct usb_ctrlrequest));
	trans_buf = kmalloc(size, GFP_KERNEL);
	if (trans_buf == NULL) {
		PRINT_ERR("kmalloc : errcord[%d]\n", ENOMEM);
		kfree(setup);
		return -ENOMEM;
	}
	memcpy(trans_buf, (char *)data, size);

#ifdef DEBUG // debug print
	pbuf = (char *)trans_buf;
	PRINT_DBG("data : [%d]byte", size);
	for (idx = 0; idx < size; idx++) {
		if (idx % 16 == 0) {
			printk("\n");
		}
		memcpy(&work, pbuf, 1);
		printk("0x%02x,", work);
		pbuf++;
	}
	printk("\n");
#endif // debug print

	// Initialization of transfer completion object
	init_completion(&comp_cb_param.comp_done);
	context = (void *)&comp_cb_param;

	// Creates a new URB
	write_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!write_urb) {
		PRINT_ERR("usb_alloc_urb : errcord[%d]\n", ENOMEM);
		retval = (long)(-ENOMEM);
		kfree(trans_buf);
		kfree(setup);
		return retval;
	}
	usb_init_urb(write_urb);

	//----------------------------------------
	// Set URB request packet (endpoint = 0 : Control transfer)
	//----------------------------------------
	setup->bRequestType = USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_RECIP_ENDPOINT;
	setup->bRequest = request;
	setup->wValue = cpu_to_le16(value);
	setup->wIndex = cpu_to_le16(index);
	setup->wLength = cpu_to_le16(size);

	usb_fill_control_urb(write_urb, dev->udev,
						 usb_sndctrlpipe(dev->udev, 0),
						 (unsigned char*)setup, trans_buf, size,
						 (usb_complete_t)Cusb_firm_ctrl_completion, context);

	//----------------------------------------
	// Transfer the URB request packet
	//----------------------------------------
	retval = (long)Cusb_firm_start_wait_urb(write_urb, USB_CTRL_SET_TIMEOUT, &length);

	// Release of URB
	usb_free_urb(write_urb);
	kfree(trans_buf);
	kfree(setup);
	PRINT_DBG("end[0x%08lx]\n", retval);
	return retval;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Control transfer (URB function to read from device firmware)
/// @param	dev				URB structure pointer
/// @param	request			request code
/// @param	value			request value
/// @param	index			request index
/// @param	size			request data size
/// @param	data			request data pointer
/// @return	function result <br>
/// 			0: Success <br>
/// 			Other than 0: URB error number
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
long Cusb_firm_read_urb(struct cdio_usb *dev, __u8 request, __u16 value, __u16 index, __u16 size, void *data)
{
	struct urb				*read_urb = NULL;
	struct usb_ctrlrequest	*setup = NULL;
	long					retval = 0;
	int						length = 0;
	char					*trans_buf = NULL;
	struct comp_cb			comp_cb_param;
	void					*context = NULL;
#ifdef DEBUG // debug print
	int						idx = 0;
	short					work = 0;
	char					*pbuf = NULL;
#endif // debug print

	PRINT_DBG("start\n");
	PRINT_DBG("request[0x%04X]\n", request);
	PRINT_DBG("value  [0x%04x]\n", value);
	PRINT_DBG("index  [0x%04x]\n", index);
	PRINT_DBG("size   [%d]\n", size);

	// Buffer initialization
	setup = kmalloc(sizeof(struct usb_ctrlrequest), GFP_KERNEL);
	if (setup == NULL) {
		PRINT_ERR("kmalloc : errcord[%d]\n", ENOMEM);
		return -ENOMEM;
	}
	memset(setup, 0, sizeof(struct usb_ctrlrequest));
	trans_buf = kmalloc(size, GFP_KERNEL);
	if (trans_buf == NULL) {
		PRINT_ERR("kmalloc : errcord[%d]\n", ENOMEM);
		kfree(setup);
		return (long)(-ENOMEM);
	}

	// Initialization of transfer completion object
	init_completion(&comp_cb_param.comp_done);
	context = (void *)&comp_cb_param;

	// Creates a new URB
	read_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!read_urb) {
		PRINT_ERR("usb_alloc_urb : errcord[%d]\n", ENOMEM);
		retval = (long)(-ENOMEM);
		kfree(trans_buf);
		kfree(setup);
		return retval;
	}
	usb_init_urb(read_urb);

	//----------------------------------------
	// Set URB request packet (endpoint = 0 : Control transfer)
	//----------------------------------------
	setup->bRequestType = USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_RECIP_ENDPOINT;
	setup->bRequest = request;
	setup->wValue = cpu_to_le16(value);
	setup->wIndex = cpu_to_le16(index);
	setup->wLength = cpu_to_le16(size);

	usb_fill_control_urb(read_urb, dev->udev,
						 usb_rcvctrlpipe(dev->udev, 0),
						 (unsigned char*)setup, trans_buf, size,
						 (usb_complete_t)Cusb_firm_ctrl_completion, context);

	//----------------------------------------
	// Transfer the URB request packet
	//----------------------------------------
	retval = (long)Cusb_firm_start_wait_urb(read_urb, USB_CTRL_GET_TIMEOUT, &length);
	if ((retval == 0) && (length > 0)) {
		memcpy((char *)data, trans_buf, length);

#ifdef DEBUG // debug print
		pbuf = (char *)data;
		PRINT_DBG("data : [%d]byte", length);
		for (idx = 0; idx < length; idx++) {
			if (idx % 16 == 0) {
				printk("\n");
			}
			memcpy(&work, pbuf, 1);
			printk(" 0x%02x", work);
			pbuf++;
		}
		printk("\n");
#endif // debug print
	}

	// Release of URB
	usb_free_urb(read_urb);
	kfree(trans_buf);
	kfree(setup);
	PRINT_DBG("end[0x%08lx]\n", retval);
	return retval;
}

//////////////////////////////////////////////////////////////////////////////////////////
/// @brief	Interrupt transfer
/// @param	dev				URB structure pointer
/// @param	size			request data size
/// @param	data			request data pointer
/// @return	function result <br>
/// 			0: Success <br>
/// 			Other than 0: URB error number
/// @note	
//////////////////////////////////////////////////////////////////////////////////////////
long Cusb_firm_interrupt_urb(struct cdio_usb *dev, __u16 size, void *data)
{
	struct urb			*int_urb = NULL;
	long				retval = 0;
	int					length = 0;
	char				*trans_buf = NULL;
	struct comp_cb		comp_cb_param;
	void				*context = NULL;
#ifdef DEBUG // debug print
	int					idx = 0;
	short				work = 0;
	char				*pbuf = NULL;
#endif // debug print

	PRINT_DBG("start\n");
	PRINT_DBG("size[%d]\n", size);

	// Buffer initialization
	trans_buf = kmalloc(size, GFP_KERNEL);
	if (trans_buf == NULL) {
		PRINT_ERR("kmalloc : errcord[%d]\n", ENOMEM);
		return (long)(-ENOMEM);
	}

	// Initialization of transfer completion object
	init_completion(&comp_cb_param.comp_done);
	comp_cb_param.int_wait = &dev->int_wait;
	comp_cb_param.int_flag = &dev->int_flag;
	context = (void *)&comp_cb_param;

	// Creates a new URB
	int_urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!int_urb) {
		retval = (long)(-ENOMEM);
		kfree(trans_buf);
		PRINT_DBG("usb_alloc_urb : errcord[%d]\n", ENOMEM);
		return retval;
	}
	usb_init_urb(int_urb);

	//----------------------------------------
	// Set URB request packet (endpoint = 1 : Interrupt transfer)
	//----------------------------------------
	usb_fill_int_urb(int_urb, dev->udev,
					 usb_rcvintpipe(dev->udev, dev->int_in_endpointAddr),
					 trans_buf, size, (usb_complete_t)Cusb_firm_int_callback,
					 context, dev->int_in_interval);

	//----------------------------------------
	// Transfer the URB request packet
	//----------------------------------------
	retval = (long)Cusb_firm_start_wait_urb(int_urb, USB_CTRL_GET_TIMEOUT, &length);
	if ((retval == 0) && (length > 0)) {
		memcpy((char *)data, trans_buf, length);
#ifdef DEBUG // debug print
		pbuf = (char *)data;
		PRINT_DBG("data : [%d]byte", length);
		for (idx = 0; idx < length; idx++) {
			if (idx % 16 == 0) {
				printk("\n");
			}
			memcpy(&work, pbuf, 1);
			printk(" 0x%02x", work);
			pbuf++;
		}
		printk("\n");
#endif // debug print
	}

	// Release of URB
	usb_free_urb(int_urb);
	kfree(trans_buf);
	PRINT_DBG("end[0x%08lx]\n", retval);
	return retval;
}
