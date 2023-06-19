////////////////////////////////////////////////////////////////////////////////
/// @file   Ccom_module.h
/// @brief  API-DIO(LNX) PCI Module - Common driver header file
/// @author &copy;CONTEC CO.,LTD.
/// @since  2002
////////////////////////////////////////////////////////////////////////////////

#ifndef __CCOM_MODULE_H_DEFINED__
#define __CCOM_MODULE_H_DEFINED__

//================================================================
// Common library error code
//================================================================
#define	CCOM_NO_ERROR				0		// Normal completed
#define	CCOM_ERROR_PARAM			-1		// Error in parameters.
#define	CCOM_ERROR_NO_VALUE			-2		// No value in setting file.
#define	CCOM_ERROR_NO_KEY			-3		// No key in setting file.
#define	CCOM_ERROR_NO_BOARD			-4		// No board found in getting resource.
#define	CCOM_ERROR_BOARDID			-5		// Board ID duplicated in getting resource.
#define	CCOM_ERROR_ARRY_SIZE		-6		// The array size is not enough in getting resource.
#define	CCOM_ERROR_IO_ADDRESS		-7		// I/O address is duplicated.
#define	CCOM_ERROR_IRQ				-8		// IRQ is duplicated.
#define	CCOM_ERROR_TIMER_ID			-9		// Invalid Timer ID. (0 to 255)
//================================================================
// Common library value definition
//================================================================
#define	CCOM_DEVICE_MAX				256		// The supported maximum number of devices that (Limitations of minor number up to 255)
#define	CCOM_ENTRY_NAME				50		// The maximum length of the entry name
#define	CCOM_SET_FILE_NAME			50		// The maximum length of the setting file name
#define	CCOM_FILE_KEY_LEN			50		// The length of the key in the setting file
#define	CCOM_FILE_VALUE_NAME_LEN	50		// The length of the value name in the setting file
#define	CCOM_FILE_VALUE_LEN			50		// The length of the value in the setting file
#define	CCOM_FILE_VALUE_TYPE_NUM	0		// The numeric value of the value in the setting file
#define	CCOM_FILE_VALUE_TYPE_STR	1		// The string of the value in the setting file
#define	CCOM_FILE_MAX_VALUE			30		// The maximum number of values in the same key in the setting file
#define	CCOM_FILE_MAX_KEY			10		// The maximum number of keys in the same device in the setting file
#define	CCOM_RAW_FILE_FLAG_NONE		0		// File read state (0: No data)
#define	CCOM_RAW_FILE_FLAG_READ		1		// File read state (1: Reading)
#define	CCOM_RAW_FILE_FLAG_COMPLETE	2		// File read state (2: Read completed)
#define	CCOM_DEVICE_NAME_MAX		50		// The maximum length of the device name
#define	CCOM_FILE_NODE_NAME_MAX		256		// The maximum length of the file node name
#define	CCOM_TIMER_MAX				256		// The maximum number of timers
#define	CCOM_TIMER_NOT_INITIALIZED	0		// Before timer initialization
#define	CCOM_TIMER_NOT_STARTED		1		// The stop state after timer initialization
#define	CCOM_TIMER_MOVING			2		// During the operation after timer initialization
#define	CCOM_IRQ_NOT_SHARE			0		// Interrupt registration: Do not share the IRQ
#define	CCOM_IRQ_SHARE				1		// Interrupt registration: Share IRQ
#define	CCOM_TQ_NOT_INITIALIZED		0		// Wait state 0: Before initialization
#define	CCOM_TQ_NOT_SLEEP_ON		1		// Wait state 1: Before sleep
#define	CCOM_TQ_SLEEP_ON			2		// Wait state 2: During sleep
//================================================================
// Device type
//================================================================
#define	DEVICE_TYPE_ISA			0				// ISA or C bus
#define	DEVICE_TYPE_PCI			1				// PCI Bus
#define	DEVICE_TYPE_PCMCIA		2				// PCMCIA
#define	DEVICE_TYPE_USB			3				// USB
#define	DEVICE_TYPE_FIT			4				// FIT
#define DEVICE_TYPE_EZUSB		10				// EZ-USB
//================================================================
// Common library macro
//================================================================
#define	DEBUG
//----------------------------------------------------------------
// Debug macro
//----------------------------------------------------------------
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,6))
#ifdef DEBUG
	#define	Ccom_debug_print(fmt, args...) \
		printk(__FILE__ "(%d):" fmt, __LINE__, ## args);
#else
	#define	Ccom_debug_print(fmt, args,...)
#endif
#endif
//================================================================
// Common library structure
//================================================================
//----------------------------------------------------------------
// Raw file data structure
// : Use for retaining the information of setting files
//----------------------------------------------------------------
typedef struct {
	char					file_name[CCOM_SET_FILE_NAME];				// Setting file name
	int						flag;										// File read state (0: No data, 1: Reading, 2: read completed
	char					*file_data;									// File contents
	unsigned long			file_len;									// The length of the file
} CCOM_RAW_FILE_DATA, *PCCOM_RAW_FILE_DATA;
//----------------------------------------------------------------
// Setting file information structure
// : Use for retaining the information for each device
//----------------------------------------------------------------
typedef struct {
	char					value_name[CCOM_FILE_VALUE_NAME_LEN];		// The name of value in the setting file
	short					type;										// Numeric or string
	char					str[CCOM_FILE_VALUE_LEN];					// The value in the setting file (string)
	unsigned long			num;										// The value in the setting file (Numeric)
} CCOM_FILE_DATA_VALUE, *PCCOM_FILE_DATA_VALUE;

typedef struct {
	char					key[CCOM_FILE_KEY_LEN];						// Key
	int						value_num;									// The number of values
	CCOM_FILE_DATA_VALUE	value_data[CCOM_FILE_MAX_VALUE];			// The array of value
} CCOM_FILE_DATA_KEY, *PCCOM_FILE_DATA_KEY;

typedef struct {
	short					key_num;									// The number of keys
	CCOM_FILE_DATA_KEY		key_data[CCOM_FILE_MAX_KEY];				// The array of key
} CCOM_FILE_DATA, *PCCOM_FILE_DATA;
//----------------------------------------------------------------
// Entry data structure
// : Use for retaining the function entries
//----------------------------------------------------------------
typedef struct {
	int						major;										// Major number
	struct file_operations	fops;										// File operation structure (function entry points)
	char					name[CCOM_ENTRY_NAME];						// Entry name
	int						(*add_device)(void **, PCCOM_FILE_DATA);	// Entry for adding a device
	int						(*delete_device)(void *);					// Entry for deleting a device
	int						(*open)(void *);							// Entry for open
	int						(*close)(void *);							// Entry for close
	int						(*dispatch)(void *, unsigned int, void *);	// Entry for dispatch
} CCOM_ENTRY_DATA, *PCCOM_ENTRY_DATA;
//----------------------------------------------------------------
// Entry structure
// : Use for passing function entry
//----------------------------------------------------------------
typedef struct {
	char					name[CCOM_ENTRY_NAME];						// Entry name
	int						(*add_device)(void **, CCOM_FILE_DATA *);	// Entry for adding a device
	int						(*delete_device)(void *);					// Entry for deleting a device
	int						(*open)(void *);							// Entry for open
	int						(*close)(void *);							// Entry for close
	int						(*dispatch)(void *, unsigned int, void *);	// Entry for dispatch
	char					file_name[CCOM_SET_FILE_NAME];				// Setting file name
} CCOM_ENTRY, *PCCOM_ENTRY;
//----------------------------------------------------------------
// Device list structure
// : Use for retaining the device information
//----------------------------------------------------------------
typedef struct {
	int						minor;										// Minor number
	char					device_name[CCOM_DEVICE_NAME_MAX];			// Device name
	char					file_node[CCOM_FILE_NODE_NAME_MAX];			// File node
	void					*device_data;								// Device Information (each category specific data)
	CCOM_FILE_DATA			*file_data;									// Setting file information
} CCOM_DEVICE, *PCCOM_DEVICE;

typedef struct {
	CCOM_ENTRY_DATA			entry;										// Entry data structure (including the major number, the function entry)
	int						device_num;									// The number of devices
	CCOM_DEVICE				device[CCOM_DEVICE_MAX];					// Device Information
} CCOM_DEVICE_LIST, *PCCOM_DEVICE_LIST;
//----------------------------------------------------------------
// Timer data structure
// : Use to implement a periodic timer
//----------------------------------------------------------------
typedef struct {
	struct timer_list		tm;											// OS timer structure
	short					initialized;								// Has been initialized or not? (0: before initialization, 1: stopped state after initialization, 2: during operation after initialization)
	void					(*func)(void *);							// Timer function
	void					*data;										// Timer function parameter
	unsigned long			tim;										// Timer value (ms)
} CCOM_TIMER_DATA, *PCCOM_TIMER_DATA;
//----------------------------------------------------------------
// Interrupt structure
// : Use to register an interrupt handler
//----------------------------------------------------------------
typedef struct {
	unsigned long			irq;										// IRQ
// Kernel version 2.2.X - 2.4.X
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
	void					(*handler)(int, void *);
// Kernel version 2.6.X -
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
	irqreturn_t				(*handler)(int, void *);
#endif
	void					*param;										// Parameter
	short					flag;										// 0: Not used, 1: Used
} CCOM_ISR_OBJ, *PCCOM_ISR_OBJ;
//----------------------------------------------------------------
// Task queue structure
// : Use for sleeping the process
//----------------------------------------------------------------
typedef struct {
	short					flag;										// Wait state 0: before initialization, 1: before sleep, 2: During sleep
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
	wait_queue_head_t		wq;											// Wait queue (2,4,0)
#else
	struct wait_queue		*wq;										// Wait queue (before (2,4,0)
#endif
} CCOM_TASK_QUEUE,  *PCCOM_TASK_QUEUE;

//================================================================
// Sleep routine (argument: It will sleep if condition is 0)
//================================================================
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
#define Ccom_sleep_on(tq, condition)															\
({																								\
	if ((&tq)->flag == CCOM_TQ_NOT_INITIALIZED) {												\
		init_waitqueue_head(&(&tq)->wq);														\
	}																							\
	(&tq)->flag = CCOM_TQ_SLEEP_ON;																\
	wait_event_interruptible((&tq)->wq, (condition) | ((&tq)->flag == CCOM_TQ_NOT_SLEEP_ON));	\
})
#else
#define Ccom_sleep_on(tq, condition)															\
({																								\
	(&tq)->flag = CCOM_TQ_SLEEP_ON;																\
	wait_event_interruptible((&tq)->wq, (condition) | ((&tq)->flag == CCOM_TQ_NOT_SLEEP_ON));	\
})
#endif

//================================================================
// Common library function prototype
//================================================================
int Ccom_register_entry(CCOM_ENTRY *ccom_entry);
int Ccom_open(struct inode *inode, struct file *file);
int Ccom_close(struct inode *inode, struct file *file);
// Kernel version 2.2.X, 2.4.X and 2.6.X
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0))
int Ccom_ioctl(struct inode *inode, struct file *file, unsigned int ctl_code, unsigned long param);
// Kernel version 3.10.X -
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
long Ccom_ioctl(struct file *file, unsigned int ctl_code, unsigned long param);
#endif
long Ccom_read_value(PCCOM_FILE_DATA fdata, char *key, char *value_name, int *type, void *value);
long Ccom_enum_key(PCCOM_FILE_DATA fdata, int index, char *key);
PCCOM_FILE_DATA Ccom_get_file_ptr(void *device_ext);
long Ccom_set_timer(unsigned int timer_id, unsigned long tim, void (*func)(void *), void *param);
void Ccom_kill_timer(unsigned int timer_id);
long Ccom_report_io(unsigned long port, unsigned long range, char *name);
long Ccom_unreport_io(unsigned long port, unsigned long range);
void Ccom_disconnect_interrupt(PCCOM_ISR_OBJ pisr);
void Ccom_sync_interrupt(unsigned long irq, void (*func)(void *), void *param);
void Ccom_wake_up(PCCOM_TASK_QUEUE tq);
void *Ccom_alloc_mem(unsigned int size);
void Ccom_free_mem(void *obj);
void *Ccom_alloc_pages(int priority, unsigned int size);
void Ccom_free_pages(unsigned long addr, unsigned int size);
void Ccom_udelay(unsigned long usec);
void Ccom_sleep_on_timeout(long  msecs);
long Ccom_get_pci_resource(unsigned short vendor_id, unsigned short device_id,unsigned short board_id, unsigned long *io_addr, unsigned long *io_num, unsigned long *mem_addr, unsigned long *mem_num, unsigned long *irq);

// Kernel version 2.2.X - 2.4.X
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
long Ccom_connect_interrupt(PCCOM_ISR_OBJ pisr, unsigned long irq, unsigned int share, void (*handler)(int, void *), void *param, char *name);
// Kernel version 2.6.X -
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
long Ccom_connect_interrupt(PCCOM_ISR_OBJ pisr, unsigned long irq, unsigned int share, irqreturn_t (*handler)(int, void *), void *param, char *name);
#endif

#endif
