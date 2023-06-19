////////////////////////////////////////////////////////////////////////////////
/// @file   Cdio_ctrl.h
/// @brief  API-DIO(LNX) Driver/Shard library common header file
/// @author &copy;CONTEC CO.,LTD.
/// @since  2002
////////////////////////////////////////////////////////////////////////////////

#ifndef __CDIO_CTRL_H_DEFINED__
#define __CDIO_CTRL_H_DEFINED__

#include <linux/ioctl.h>

//================================================================
// Macro definition
//================================================================
#define	CDIO_MAX_PORTS					256		// The maximum number of I/O ports/bits
#define	CDIO_MAX_TRG_BITS				256		// The maximum number of the trigger monitoring bits
#define	CDIO_MAX_SENCE_NUM				1024	// The size of the interrupt queue
#define	CDIO_MAX_INT_NUM				144		// The maximum number of interrupts
#define	CDIO_MAX_INT_LOGIC				62		// The maximum number of interrupt logic
#ifndef CDIOBI_INT_SENCE_SIZE
	#define	CDIOBI_INT_SENCE_SIZE		144/8	// The size of the interrupt sense port (and set PIO-48D to MAX) BIOS-defined value
#endif

//================================================================
// Error code
//================================================================
#define	DIO_ERR_SUCCESS					0		// Normal Complete
#define	DIO_ERR_INI_RESOURCE			1		// Failed in the acquisition of resource.
#define	DIO_ERR_INI_INTERRUPT			2		// Failed in the registration of interrupt routine.
#define	DIO_ERR_INI_MEMORY				3		// Failed in memory allocation. This error hardly occurs. If it occurred, please increase memory.
#define	DIO_ERR_INI_REGISTRY			4		// Failed in accessing the setting file.
#define	DIO_ERR_INI_BOARD_ID			5		// Board ID is duplicated.
#define	DIO_ERR_DLL_DEVICE_NAME			10000	// Device name which isn't registered in setting file is specified.
#define	DIO_ERR_DLL_INVALID_ID			10001	// Invalid ID is specified. Make sure whether initialization function normally complete. And, make sure the scope of variable in which ID is stored.
#define	DIO_ERR_DLL_CALL_DRIVER			10002	// Driver cannot be called (failed in ioctl)..
#define	DIO_ERR_DLL_CREATE_FILE			10003	// Failed in creating file (open failed).
#define	DIO_ERR_DLL_CLOSE_FILE			10004	// Failed in closing file (close failed).
#define	DIO_ERR_DLL_CREATE_THREAD		10005	// Failed in creating thread.
#define	DIO_ERR_INFO_INVALID_DEVICE		10050	// Specified device name isn't found. Please check the spelling.
#define	DIO_ERR_INFO_NOT_FIND_DEVICE	10051	// The usable device isn't found.
#define	DIO_ERR_INFO_INVALID_INFOTYPE	10052	// The specified device information type is outside the range.
#define	DIO_ERR_DLL_BUFF_ADDRESS		10100	// Invalid data buffer address.

#define	DIO_ERR_DLL_TRG_KIND			10300	// Trigger type is outside the specified range.
#define	DIO_ERR_DLL_CALLBACK			10400	// Invalid address of callback function.
#define DIO_ERR_DLL_DIRECTION			10500	// Input / output switching function setting value is out of the specified range.

#define	DIO_ERR_SYS_MEMORY				20000	// Failed in memory. This error hardly occurs. If this error has occurred, please install more memory.
#define	DIO_ERR_SYS_NOT_SUPPORTED		20001	// This function cannot be used in this device.
#define	DIO_ERR_SYS_BOARD_EXECUTING		20002	// It cannot perform because the device is executing.
#define	DIO_ERR_SYS_USING_OTHER_PROCESS	20003	// It cannot perform because the other process is using the device.
#define	DIO_ERR_SYS_NOT_SUPPORT_KERNEL	20004	// It is not supporting in the version of kernel.

#define STATUS_SYS_USB_CRC				20020	// CRC mismatch
#define STATUS_SYS_USB_BTSTUFF			20021	// BITSTUFF error
#define STATUS_SYS_USB_NO_ENTRY			20022	// URB stop (stop request to endpoint)
#define STATUS_SYS_USB_CONNECT_RESET	20023	// URB is an unlink
#define STATUS_SYS_USB_INPROGRESS		20024	// During URB processing
#define STATUS_SYS_USB_COMMUNICATION	20025	// Receive URB, receive data faster than write data to memory
#define STATUS_SYS_USB_STREAM_RESOURCE	20026	// At the time of transmission URB, the data rate is faster than the transmission data rate, data can not be taken out from the memory
#define STATUS_SYS_USB_DATA_OVERRUN		20027	// Receive more data than the maximum packet size
#define STATUS_SYS_USB_REMOTE_IO		20028	// The total amount of URB request data was not received
#define STATUS_SYS_USB_NO_DEVICE		20029	// No USB device in system
#define STATUS_SYS_USB_XDEV				20030	// Partial completion of isochronous transfer
#define STATUS_SYS_USB_ENDPOINT_HALTED	20031	// End point stall
#define STATUS_SYS_USB_INVALID			20032	// Argument error etc. Fatal error
#define STATUS_SYS_USB_SHUTDOWN			20033	// Device disconnects from system
#define STATUS_SYS_USB_TIMEOUT			20034	// Transfer timeout

#define	DIO_ERR_SYS_PORT_NO				20100	// Port number is outside of the available range.
#define	DIO_ERR_SYS_PORT_NUM			20101	// Number of ports is outside of the available range.
#define	DIO_ERR_SYS_BIT_NO				20102	// Bit number is outside of the available range.
#define	DIO_ERR_SYS_BIT_NUM				20103	// Number of bits is outside of the available range.
#define	DIO_ERR_SYS_BIT_DATA			20104	// Bit data is neither 0 nor 1.
#define	DIO_ERR_SYS_INT_BIT				20200	// Interrupt bit is outside of the available range.
#define	DIO_ERR_SYS_INT_LOGIC			20201	// Interrupt logic is outside of the available range.
#define	DIO_ERR_SYS_TRG_LOGIC			20202	// Trigger logic is outside of the range.
#define	DIO_ERR_SYS_TIM					20300	// Timer value is outside of the range. Error in trigger function.
#define	DIO_ERR_SYS_FILTER				20400	// Filter time constant is outside of the range.
#define	DIO_ERR_SYS_8255				20500	// 8255 chip number is outside of the range.

// Error for NET device 
#define	DIO_ERR_NET_BASE				22000	// Access error
#define	DIO_ERR_NET_ACCESS				22001	// Access violation
#define	DIO_ERR_NET_AREA				22002	// Area error
#define	DIO_ERR_NET_SIZE				22003	// Access size error
#define	DIO_ERR_NET_PARAMETER			22004	// Parameter error
#define	DIO_ERR_NET_LENGTH				22005	// Length error
#define	DIO_ERR_NET_RESOURCE			22006	// Insufficient resources
#define	DIO_ERR_NET_TIMEOUT				22016	// Communications timeout
#define	DIO_ERR_NET_HANDLE				22017	// Handle error
#define	DIO_ERR_NET_CLOSE				22018	// Close error
#define	DIO_ERR_NET_TIMEOUT_WIO			22064	// Wireless communications timeout

//================================================================
// Control structure
//================================================================
// Initialization function
typedef struct {
	long			ret;						// Return value
} CDIO_INIT, *PCDIO_INIT;

// Exit function
typedef struct {
	int				process_id;					// Process ID
	long			ret;						// Return value
} CDIO_EXIT, *PCDIO_EXIT;

// Reset function
typedef struct {
	long			ret;						// Return value
} CDIO_RESET, *PCDIO_RESET;

// Digital filter setting function
typedef struct {
	short			filter_value;				// Filter value
	long			ret;						// Return value
} CDIO_DFILTER, *PCDIO_DFILTER;

// I/O direction
typedef struct {
	short			dir;						// Direction
	long			ret;						// Return value
} CDIO_DIR, *PCDIO_DIR;

// I/O function
typedef struct {
	short			cmd;						// Command
	short			num;						// The number of ports/bits
	short			port[CDIO_MAX_PORTS];		// Port/Bit number
	unsigned char	data[CDIO_MAX_PORTS];		// Port/Bit data
	long			ret;						// Return value
} CDIO_INOUT, *PCDIO_INOUT;

// Interrupt factor setting function
typedef struct {
	short			bit_no;						// Bit number
	short			logic;						// Interrupt logic
	int				process_id;					// Process ID
	short			int_bit_num;				// Interruptable number of bits
	unsigned char	int_logic[CDIO_MAX_INT_LOGIC];
												// Interrupt logic 0:Rising 1:Falling
	long			ret;						// Return value
} CDIO_SET_INT, *PCDIO_SET_INT;

// Trigger factor setting function
typedef struct {
	short			bit_no;						// Bit number
	short			logic;						// Trigger logic
	int				process_id;					// Process ID
	unsigned long	tim;						// Monitoring cycle
	long			ret;						// Return value
} CDIO_SET_TRG, *PCDIO_SET_TRG;

// Function	to retrieve the device information
typedef struct {
	short			type;						// Information type
	char			*board_name;				// Board name
	void			*param1;					// Param1
	void			*param2;					// Param2
	void			*param3;					// Param3
	long			ret;						// Return value
} CDIO_GET_INFO, *PCDIO_GET_INFO;

// Get the maximum number of USB device ports
typedef struct {
	short			port_num[2];				// [0] = in max ports, [1] = out max ports
	long			ret;						// Return value
} CDIO_GET_MAX_PORT, *PCDIO_GET_MAX_PORT;

// Wait in thread
typedef struct {
	long			ret;						// Return value
	int				process_id;					// Process ID
} CDIO_TH_WAIT, *PCDIO_TH_WAIT;

// Thread wake-up
typedef struct {
	long			ret;						// Return value
	int				process_id;					// Process ID
} CDIO_TH_WAKE_UP, *PCDIO_TH_WAKE_UP;

// Retrieve interrupt factor in thread
typedef struct {
	unsigned short	max_bit;					// The maximum number of bits
	unsigned short	sence_num;					// The number of sences
	unsigned char	sence_arry[CDIO_MAX_SENCE_NUM][CDIOBI_INT_SENCE_SIZE];
												// Sence queue
	unsigned short	logic[CDIOBI_INT_SENCE_SIZE*8];
												// Interrupt logic
	int				process_id;					// Process ID
	long			ret;						// Return value
} CDIO_TH_GET_INT, *PCDIO_TH_GET_INT;

// Retrieve trigger factor in thread
typedef struct {
	short			data[CDIO_MAX_TRG_BITS];	// Trigger data
	short			num;						// The number of triggers
	int				process_id;					// Process ID
	long			ret;						// Return value
} CDIO_TH_GET_TRG, *PCDIO_TH_GET_TRG;

// Acquisition factor in thread for USB
typedef struct {
	int				process_id;					// Process ID
	unsigned short	int_bit_num;				// Interrupt bit number
	unsigned short	int_count[CDIO_MAX_INT_NUM];// Number of interrupts
	long			event_chk;					// Presence of events
	long			ret;						// Return value
} CDIO_TH_USB_INT, *PCDIO_TH_USB_INT;

// 8255 mode setting
typedef struct {
	unsigned short	chip_no;					// 8255 chip number
	unsigned short	ctrl_word;					// Control word
	long			ret;						// Return value
} CDIO_SET_8255MODE, *PCDIO_SET_8255MODE;

// Retrieve 8255 mode setting
typedef struct {
	unsigned short	chip_no;					// 8255 chip number
	unsigned short	*ctrl_word;		   			// Control word
	long			ret;						// Return value
} CDIO_GET_8255MODE, *PCDIO_GET_8255MODE;

//================================================================
// Control code
//================================================================
#define	CDIO_IOC_MAGIC			'D'												// Magic No.
#define	CDIO_IOC_CLEAR			_IOWR(CDIO_IOC_MAGIC, 0, void *)
#define	CDIO_IOC_INIT			_IOWR(CDIO_IOC_MAGIC, 1, PCDIO_INIT)			// Initialization function
#define	CDIO_IOC_EXIT			_IOWR(CDIO_IOC_MAGIC, 2, PCDIO_EXIT)			// Exit function
#define	CDIO_IOC_RESET			_IOWR(CDIO_IOC_MAGIC, 3, PCDIO_RESET)			// Reset function
#define	CDIO_IOC_DFILTER		_IOWR(CDIO_IOC_MAGIC, 4, PCDIO_DFILTER)			// Function for setting digital filter
#define	CDIO_IOC_INOUT			_IOWR(CDIO_IOC_MAGIC, 5, PCDIO_INOUT)			// I/O function
#define	CDIO_IOC_SET_INT		_IOWR(CDIO_IOC_MAGIC, 6, PCDIO_SET_INT)			// Function for setting interrupt factor
#define	CDIO_IOC_SET_TRG		_IOWR(CDIO_IOC_MAGIC, 7, PCDIO_SET_TRG)			// Function for setting trigger factor
#define	CDIO_IOC_GET_INFO		_IOWR(CDIO_IOC_MAGIC, 8, PCDIO_GET_INFO)		// Function for retrieving the device information
#define	CDIO_IOC_TH_WAIT		_IOWR(CDIO_IOC_MAGIC, 9, PCDIO_TH_WAIT)			// Wait in thread
#define	CDIO_IOC_TH_WAKE_UP		_IOWR(CDIO_IOC_MAGIC, 10, PCDIO_TH_WAKE_UP)		// Thread wake-up		//Ver140
#define	CDIO_IOC_TH_GET_INT		_IOWR(CDIO_IOC_MAGIC, 11, PCDIO_TH_GET_INT)		// Retrieve interrupt factor in thread
#define	CDIO_IOC_TH_GET_TRG		_IOWR(CDIO_IOC_MAGIC, 12, PCDIO_TH_GET_TRG)		// Retrieve trigger factor in thread
#define	CDIO_IOC_SET_8255MODE	_IOWR(CDIO_IOC_MAGIC, 13, PCDIO_SET_8255MODE)	// 8255 mode setting
#define	CDIO_IOC_GET_8255MODE	_IOWR(CDIO_IOC_MAGIC, 14, PCDIO_GET_8255MODE)	// Retrieve 8255 mode

//================================================================
// Command definition
//================================================================
#define	CDIO_CMD_INP_PORT		0
#define	CDIO_CMD_INP_BIT		1
#define	CDIO_CMD_OUT_PORT		2
#define	CDIO_CMD_OUT_BIT		3
#define	CDIO_CMD_ECHO_PORT		4
#define	CDIO_CMD_ECHO_BIT		5

//================================================================
// Information type
//================================================================
#define	IDIO_DEVICE_TYPE		0				// Device type (short)
#define	IDIO_NUMBER_OF_8255		1				// The number of 8255 (int)
#define	IDIO_IS_8255_BOARD		2				// 8255 type (int)
#define	IDIO_NUMBER_OF_DI_BIT	3				// DI BIT(short)
#define	IDIO_NUMBER_OF_DO_BIT	4				// DO BIT(short)
#define	IDIO_NUMBER_OF_DI_PORT	5				// DI PORT(short)
#define	IDIO_NUMBER_OF_DO_PORT	6				// DO PORT(short)
#define	IDIO_IS_POSITIVE_LOGIC	7				// Positive logic? (int)
#define	IDIO_IS_ECHO_BACK		8				// Echo back available? (int)
#define	IDIO_IS_DIRECTION		9				// DioSetIoDirection function available(int)
#define	IDIO_IS_FILTER			10				// Digital filter available(int)
#define	IDIO_NUMBER_OF_INT_BIT	11				// Interruptable number of bits(short)

#define	IDIO_NUM_OF_PORT		20				// Number of occupied ports(short)
#define	IDIO_BOARD_TYPE			21				// Board type(unsigned long)

//================================================================
// Interrupt, trigger rising, falling
//================================================================
#define	DIO_INT_NONE			0				// Interrupt: Mask
#define	DIO_INT_RISE			1				// Interrupt: Rising
#define	DIO_INT_FALL			2				// Interrupt: Falling
#define	DIO_TRG_NONE			0				// Trigger: Mask
#define	DIO_TRG_RISE			1				// Trigger: Rising
#define	DIO_TRG_FALL			2				// Trigger: Falling

//================================================================
// Device type
//================================================================
#define	DEVICE_TYPE_ISA			0				// ISA or C bus
#define	DEVICE_TYPE_PCI			1				// PCI Bus
#define	DEVICE_TYPE_PCMCIA		2				// PCMCIA
#define	DEVICE_TYPE_USB			3				// USB
#define	DEVICE_TYPE_FIT			4				// FIT
#define DEVICE_TYPE_EZUSB		10				// EZ-USB
#define DEVICE_TYPE_M3USB		11              // M3-USB
#define	DEVICE_TYPE_NET			20				// Ethernet, Wireless etc
#endif

