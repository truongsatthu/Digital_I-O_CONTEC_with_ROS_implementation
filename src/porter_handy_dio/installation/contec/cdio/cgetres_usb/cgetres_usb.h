////////////////////////////////////////////////////////////////////////////////
/// @file   cgetres_usb.h
/// @brief  API-DIO(LNX) config(USB) : USB driver resource acquisition header file
/// @author &copy;CONTEC CO.,LTD.
/// @since  2017
////////////////////////////////////////////////////////////////////////////////

#ifndef __CGETRES_USB__
#define __CGETRES_USB__

//---------------------------------------------------------------
///@name Definition
// @{
//---------------------------------------------------------------
#define	MAX_DEVICE_NAME		50				///< Device Name Max Length
#define	MAX_SERIAL_ID		64				///< Device SerialID Max Length
// @}

//---------------------------------------------------------------
/// Structure of USB resource informaion
//---------------------------------------------------------------
typedef struct {
	char				serial_id[MAX_SERIAL_ID];		///< Serial ID
	char				device_name[MAX_DEVICE_NAME];	///< Device Name
	unsigned short		vendor_id;						///< Vender ID
	unsigned short		device_id;						///< Device ID
	long				ret;							///< Return code
} GETUSBRES, *PGETUSBRES;

//---------------------------------------------------------------
///@name Control code
// @{
//---------------------------------------------------------------
#define	CGETRES_IOC_USB_MAGIC	'u'											///< Magic No.
#define	CGETRES_IOC_USB			_IOWR(CGETRES_IOC_USB_MAGIC, 1, PGETUSBRES)	///< Get the USB resource
// @}

//---------------------------------------------------------------
// Function definition
//---------------------------------------------------------------
#ifndef MODULE_USB
long	get_usb_resource(PGETUSBRES param, char *device_head, short index);
#endif // MODULE_USB

#endif // __CGETRES_USB__
