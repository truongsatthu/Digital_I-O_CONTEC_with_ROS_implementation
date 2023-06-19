////////////////////////////////////////////////////////////////////////////////
/// @file   Cusb_ctrl.h
/// @brief  API-DIO(LNX) Driver/Shard library common header file (for USB)
/// @author &copy;CONTEC CO.,LTD.
/// @since  2017
////////////////////////////////////////////////////////////////////////////////

#ifndef __CUSB_CTRL_H__
#define __CUSB_CTRL_H__

#include <linux/ioctl.h>

//================================================================
///@name Definition
// @{
//================================================================
#define CONTEC_VENDOR_USB		0x06CE		///< CONTEC's device vender ID

#define	MAX_USB_DATA			256			///< Control transfer data size
#define	MAX_DEVICE_NAME			50			///< Device name max length
#define	MAX_SERIAL_ID			64			///< Device serialID max length
// @}

//================================================================
// Structure
//================================================================
//----------------------------------------------------------------
/// USB Get device resources structure
//----------------------------------------------------------------
typedef struct {
	char			serial_id[MAX_SERIAL_ID];		///< Serial ID
	char			device_name[MAX_DEVICE_NAME];	///< Device Name
	unsigned short	vendor_id;						///< Vender ID
	unsigned short	device_id;						///< Device ID
	long			ret;							///< Return Code
} CDIO_USB_RES, *PCDIO_USB_RES;

//----------------------------------------------------------------
/// USB Device control data
//----------------------------------------------------------------
union REQ_DATA {
	unsigned char	data[MAX_USB_DATA];				///< Request Data
	char			dc;								///< Request Data (char)
	short			ds;								///< Request Data (short)
	int				di;								///< Request Data (int)
	long			dl;								///< Request Data (long)
};

//----------------------------------------------------------------
/// USB Device control structure
//----------------------------------------------------------------
typedef struct {
	unsigned short	request;						///< Request Code
	unsigned short	req_value;						///< Request Value
	unsigned short	req_index;						///< Request Index
	unsigned short	req_size;						///< Request Data Size
	union REQ_DATA	req_data;						///< Request Data
	long			ret;							///< Return Code
} CDIO_USB_CTRL, *PCDIO_USB_CTRL;

//================================================================
///@name USB Control Code
// @{
//================================================================
#define	CDIO_IOC_USB_MAGIC	'u'														///< USB Magic No.
#define	CDIO_IOC_USB_RES		_IOWR(CDIO_IOC_USB_MAGIC, 1, PCDIO_USB_RES) 		///< USB Get Device Resources
#define	CDIO_IOC_USB_CTRL_IN	_IOWR(CDIO_IOC_USB_MAGIC, 2, PCDIO_USB_CTRL)		///< USB Control Device (input)
#define	CDIO_IOC_USB_CTRL_OUT	_IOWR(CDIO_IOC_USB_MAGIC, 3, PCDIO_USB_CTRL)		///< USB Control Device (Output)
#define	CDIO_IOC_USB_CTRL_INT	_IOWR(CDIO_IOC_USB_MAGIC, 4, PCDIO_USB_CTRL)		///< USB Control Device (Interrupt)
// @}

#endif // __CUSB_CTRL_H__
