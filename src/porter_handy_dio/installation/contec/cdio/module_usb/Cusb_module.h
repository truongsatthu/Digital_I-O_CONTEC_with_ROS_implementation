////////////////////////////////////////////////////////////////////////////////
/// @file   Cusb_module.h
/// @brief  API-DIO(LNX) USB Module - USB driver header file
/// @author &copy;CONTEC CO.,LTD.
/// @since  2017
////////////////////////////////////////////////////////////////////////////////

#ifndef __CUSB_MODULE_H__
#define __CUSB_MODULE_H__

//================================================================
///@name Definition
//@{
//================================================================
#define CUSB_ENTRY_NAME				"cdiousb"			///< Entry name
#define CUSB_ENTRY_FILENAME			"contec_dio.conf"	///< Entry file name
#define CUSB_MAJOR_NO				180					///< USB major number
#define CUSB_MINOR_BASE				0					///< Minor number

#define	CUSB_DEVICE_MAX				256					///< Maximum number of supported devices (restrictions that minor numbers come up to 255)
#define	CUSB_ENTRY_NAME_LEN			50					///< Maximum entry name
#define	CUSB_SET_FILE_NAME_LEN		50					///< Maximum value of setting file name
#define	CUSB_FILE_KEY_LEN			50					///< Key length of setting file
#define	CUSB_FILE_VALUE_NAME_LEN	50					///< Length of the value name of the setting file
#define	CUSB_FILE_VALUE_LEN			50					///< Length of value of setting file
#define	CUSB_FILE_VALUE_TYPE_NUM	0					///< If the value of the setting file is numeric
#define	CUSB_FILE_VALUE_TYPE_STR	1					///< When the value of the setting file is a character string
#define	CUSB_FILE_MAX_VALUE			30					///< Maximum number of values in the same key of the configuration file
#define	CUSB_FILE_MAX_KEY			10					///< Maximum number of keys in the same device in the configuration file
#define	CUSB_RAW_FILE_FLAG_NONE		0					///< File read state (0: no data)
#define	CUSB_RAW_FILE_FLAG_READ		1					///< File reading state (1: reading)
#define	CUSB_RAW_FILE_FLAG_COMPLETE	2					///< File read state (2: Read complete)
#define	CUSB_DEVICE_NAME_MAX		50					///< Maximum length of device name
#define	CUSB_FILE_NODE_NAME_MAX		256					///< Maximum length of file node name

#define MAX_TRANSFER				(PAGE_SIZE - 512)	///< Maximum number of packets
#define WRITES_IN_FLIGHT			8					///< arbitrarily chosen
//@}

//================================================================
///@name Error code
//@{
//================================================================
#define	CUSB_NO_ERROR				0		///< Successful completion
#define	CUSB_ERROR_PARAM			-1		///< The parameter is incorrect
#define	CUSB_ERROR_NO_VALUE			-2		///< There is no value in the setting file
#define	CUSB_ERROR_NO_KEY			-3		///< There is no key in the configuration file
//@}

//================================================================
// Structures
//================================================================
//----------------------------------------------------------------
///@name USB device structure
//@{
//----------------------------------------------------------------
struct cdio_usb {
	struct usb_device				*udev;					///< USB device information
	struct usb_interface			*interface;				///< USB device interface
	struct semaphore				limit_sem;				///< Limiting the number of writes in progress

	unsigned char					*bulk_in_buffer;		///< Read data buffer
	size_t							bulk_in_size;			///< Read data buffer size
	__u8							bulk_in_endpointAddr;	///< BULK IN endpoint address
	__u8							bulk_out_endpointAddr;	///< BULK OUT endpoint address

	__u8							int_in_endpointAddr;	///< INTERRUPT IN endpoint address
	__u8							int_in_interval;		///< INTERRUPT IN endpoint interval
	__u8							int_out_endpointAddr;	///< INTERRUPT OUT endpoint address
	__u8							int_out_interval;		///< INTERRUPT OUT endpoint interval

	struct kref						kref;					///< Kref structure
	struct semaphore				mutex_flag;				///< Mutex flag
	wait_queue_head_t				int_wait;				///< Wait for an interrupt
	atomic_t						int_flag;				///< interrupt flag
};
#define to_cdiousb_dev(d) container_of(d, struct cdio_usb, kref)	///< Get start address
//@}

//----------------------------------------------------------------
///@name Raw file data structure
//       Used to hold information of configuration file
// @{
//----------------------------------------------------------------
typedef struct {
	char					file_name[CUSB_SET_FILE_NAME_LEN];	///< Setting file name
	int						flag;								///< File reading state (0: no data, 1: reading in, 2: reading completed)
	char					*file_data;							///< File contents
	unsigned long			file_len;							///< File length
} CUSB_RAW_FILE_DATA, *PCUSB_RAW_FILE_DATA;
//@}

#endif // __CUSB_MODULE_H__
