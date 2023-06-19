////////////////////////////////////////////////////////////////////////////////
/// @file   Ccom_module.c
/// @brief  API-DIO(LNX) PCI Module - Common driver source file
/// @author &copy;CONTEC CO.,LTD.
/// @since  2002
////////////////////////////////////////////////////////////////////////////////

#ifndef LINUX_VERSION_CODE
#include <linux/version.h>
#endif

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/delay.h>
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
#include <linux/sched.h>

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c)	((a)<<16+(b)<<8+c)
#endif

// Kernel version 2.6.X -
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
#include <linux/moduleparam.h>
#endif

#include "Ccom_module.h"
//================================================================
// External variable
//================================================================
static	CCOM_DEVICE_LIST	device_list;				// Device List
static	CCOM_RAW_FILE_DATA	raw_file_data;				// File raw data
static	CCOM_TIMER_DATA		timer_data[CCOM_TIMER_MAX];	// Timer data
static	int					major = 0;					// Major number

// Kernel version 2.2.X - 2.4.X
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
MODULE_PARM(major, "h");
// Kernel version 2.6.X -
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
module_param(major, int, 0);
#endif

//================================================================
// Subroutine
//================================================================
static int Ccom_sub_add_device(void);
//----------------------------------------------------------------
// For Kernel version 2.2.X, 2.4.X and 2.6.X
//----------------------------------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0))
static int Ccom_file_read_proc(char *buf, char **start, off_t offset, int count, int *eof, void *data);
static int Ccom_file_write_proc(struct file *file, const char *buf, unsigned long count, void *data);
//----------------------------------------------------------------
// For Kernel version 3.10.X -
//----------------------------------------------------------------
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
static ssize_t Ccom_file_read_proc(struct file *flip, char __user *buf, size_t count, loff_t *data);
static ssize_t Ccom_file_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *data);
static struct file_operations  proc_fops = {
	.owner 	=	THIS_MODULE,
	.read	=	Ccom_file_read_proc,
	.write	=	Ccom_file_write_proc,
};
static 	long	tmp_read_off;
#endif

//================================================================
// Load module
//================================================================
int init_module(void)
{
	int			ret;
	CCOM_ENTRY	entry;
	struct		proc_dir_entry	*proc_entry;
	int			tmp_major;

	//--------------------------------------
	// Do not export the symbol
	//--------------------------------------
	//----------------------------------------
	// Kernel version 2.2.X - 2.4.X
	//----------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
	EXPORT_NO_SYMBOLS;
#endif
	//--------------------------------------
	// Get the entry point
	//--------------------------------------
	ret = Ccom_register_entry(&entry);
	if (ret != 0) {
		return ret;
	}
	//--------------------------------------
	// Copy into the entry of device list
	//--------------------------------------
	// Major number
	device_list.entry.major = 0;
	// File operation structure (function entry point)
	device_list.entry.fops.open				= Ccom_open;
	device_list.entry.fops.release			= Ccom_close;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
	device_list.entry.fops.unlocked_ioctl	= Ccom_ioctl;
#else
	device_list.entry.fops.ioctl			= Ccom_ioctl;
#endif
	strcpy(device_list.entry.name	, entry.name);						// Entry name
	device_list.entry.add_device 			= entry.add_device;			// Entry for adding a device
	device_list.entry.delete_device 		= entry.delete_device;		// Entry for deleting a device
	device_list.entry.open					= entry.open;				// Entry for open
	device_list.entry.close					= entry.close;				// Entry for close
	device_list.entry.dispatch				= entry.dispatch;			// Entry for dispatch
	//--------------------------------------
	// Copy into the raw data of the file
	//--------------------------------------
	strcpy(raw_file_data.file_name, entry.file_name);					// Setting file name
	//--------------------------------------
	// Check the set string length
	//--------------------------------------
	if (strlen(device_list.entry.name) == 0 ||
		strlen(raw_file_data.file_name) == 0) {
		return -ENODEV;
	}
	//--------------------------------------
	// Register a character device
	//--------------------------------------
	tmp_major	= register_chrdev(major, device_list.entry.name, &device_list.entry.fops);
	if (tmp_major < 0) {
		return tmp_major;
	}
	if (major == 0) {
		device_list.entry.major	= tmp_major;
	} else {
		device_list.entry.major	= major;
	}
	//--------------------------------------
	// Register the /proc file entry
	//--------------------------------------
	//--------------------------------------
	// Kernel version 2.2.X, 2.4.X and 2.6.X
	//--------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0))
	proc_entry = create_proc_entry(raw_file_data.file_name, 0, 0);
	proc_entry->write_proc	= Ccom_file_write_proc;
	proc_entry->read_proc	= Ccom_file_read_proc;
	//---------------------------------------
	// Kernel version 3.10.X -
	//---------------------------------------
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
	proc_entry	= proc_create(raw_file_data.file_name, 0, 0, &proc_fops);
#endif
	return 0;
}

//================================================================
// Unload module
//================================================================
void cleanup_module(void)
{
	int		i;
	
	//--------------------------------------
	// Delete device information
	//--------------------------------------
	for (i=0; i<device_list.device_num; i++) {
		//--------------------------------------
		// Call the delete entry
		//--------------------------------------
		if (device_list.entry.delete_device != NULL) {
			device_list.entry.delete_device(device_list.device[i].device_data);
		}
		device_list.device[i].device_data = NULL;
		//--------------------------------------
		// Delete file data
		//--------------------------------------
		if (device_list.device[i].file_data) {
			kfree(device_list.device[i].file_data);
			device_list.device[i].file_data = NULL;
		}
	}
	//--------------------------------------
	// Release the file raw data
	//--------------------------------------
	remove_proc_entry(raw_file_data.file_name, NULL);
	if (raw_file_data.file_data) {
		kfree(raw_file_data.file_data);
		raw_file_data.file_data = NULL;
		raw_file_data.file_len = 0;
	}
	//--------------------------------------
	// Unregister the character device
	//--------------------------------------
	unregister_chrdev(device_list.entry.major, device_list.entry.name);
}

//================================================================
// Open entry
//================================================================
int	Ccom_open(struct inode *inode, struct file *file)
{
	int		major, minor;
	int		i;
	int		iret;

	//--------------------------------------
	// Retrieve the major and minor number
	//--------------------------------------
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
	major = MAJOR(inode->i_rdev);
	minor = MINOR(inode->i_rdev);
#else
	major = MAJOR(file->f_inode->i_rdev);
	minor = MINOR(file->f_inode->i_rdev);
#endif	
	//--------------------------------------
	// Check the major number
	//--------------------------------------
	if (major != device_list.entry.major) {
		return -ENODEV;
	}
	//--------------------------------------
	// Check the minor number
	//--------------------------------------
	for (i=0; i<device_list.device_num; i++) {
		if (minor == device_list.device[i].minor) {
			//--------------------------------------
			// Call the open entry if it exsit
			//--------------------------------------
			if (device_list.entry.open != NULL) {
				iret = device_list.entry.open(device_list.device[i].device_data);
			}
			//--------------------------------------
			// The process of module usage count
			//--------------------------------------
			//----------------------------------------
			// Kernel version 2.2.X - 2.4.X
			//----------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
			MOD_INC_USE_COUNT;
			//----------------------------------------
			// Kernel version 2.6.X -
			//----------------------------------------
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
			try_module_get(THIS_MODULE);
#endif
			return 0;
		}
	}
	return -ENODEV;
}

//================================================================
// Close entry
//================================================================
int	Ccom_close(struct inode *inode, struct file *file)
{
	int		major, minor;
	int		i;
	int		iret;
	
	//--------------------------------------
	// Retrieve the major and minor number
	//--------------------------------------
#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
	major = MAJOR(inode->i_rdev);
	minor = MINOR(inode->i_rdev);
#else
	major = MAJOR(file->f_inode->i_rdev);
	minor = MINOR(file->f_inode->i_rdev);
#endif	
	//--------------------------------------
	// Check the major number
	//--------------------------------------
	if (major != device_list.entry.major) {
		return -ENODEV;
	}
	//--------------------------------------
	// Check the minor number
	//--------------------------------------
	for (i=0; i<device_list.device_num; i++) {
		if (minor == device_list.device[i].minor) {
			//--------------------------------------
			// Call the close entry if it exsit
			//--------------------------------------
			if (device_list.entry.close != NULL) {
				iret = device_list.entry.close(device_list.device[i].device_data);
			}
			//--------------------------------------
			// The process of module usage count
			//--------------------------------------
			//----------------------------------------
			// Kernel version 2.2.X - 2.4.X
			//----------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
			MOD_DEC_USE_COUNT;
			//----------------------------------------
			// Kernel version 2.6.X -
			//----------------------------------------
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
			module_put(THIS_MODULE);
#endif
			return 0;
		}
	}
	return -ENODEV;
}

//================================================================
// Dispatch entry
//================================================================
//----------------------------------------
// Kernel version 2.2.X, 2.4.X and 2.6.x
//----------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0))
int	Ccom_ioctl(struct inode *inode, struct file *file, unsigned int ctl_code, unsigned long param)
//----------------------------------------
// Kernel version 3.10.X -
//----------------------------------------
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
long	Ccom_ioctl(struct file *file, unsigned int ctl_code, unsigned long param)
#endif
{
	int		major, minor;
	int		i;
	int		iret;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0))
	//--------------------------------------
	// Retrieve the major and minor number
	//--------------------------------------
	major = MAJOR(inode->i_rdev);
	minor = MINOR(inode->i_rdev);
#else
	major = MAJOR(file->f_inode->i_rdev);
	minor = MINOR(file->f_inode->i_rdev);
#endif
	//--------------------------------------
	// Check the major number
	//--------------------------------------
	if (major != device_list.entry.major) {
		return -ENODEV;
	}
	//--------------------------------------
	// Check the minor number
	//--------------------------------------
	for (i=0; i<device_list.device_num; i++) {
		if (minor == device_list.device[i].minor) {
			//--------------------------------------
			// Call the dispatch entry if it exsit
			//--------------------------------------
			iret = 0;
			if (device_list.entry.dispatch != NULL) {
				iret = device_list.entry.dispatch(device_list.device[i].device_data, ctl_code, (void *)param);
			}
			//--------------------------------------
			// There is an error if the return value is other than 0
			//--------------------------------------
			if (iret != 0) {
				return -EINVAL;
			}
			return 0;
		}
	}
	return -ENODEV;
}

//================================================================
// Interrupt service routine
//================================================================
// Kernel version 2.2.X - 2.4.X
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
static void Ccom_isr(int irq, void *param, struct pt_regs *preg)
// Kernel version 2.6.X
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19))
static irqreturn_t Ccom_isr(int irq, void *param, struct pt_regs *preg)
// Kernel version 2.6.19 -
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19))
static irqreturn_t Ccom_isr(int irq, void *param)
#endif
{
	PCCOM_ISR_OBJ	pisr = (PCCOM_ISR_OBJ)param;
	//--------------------------------------
	// Retrieve and call the service routine and the parameters
	// in the interrupt object
	//--------------------------------------
	if (pisr != NULL && pisr->handler != NULL) {
	//--------------------------------------
	// Kernel version 2.2.X - 2.4.X
	//--------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
		pisr->handler(irq, pisr->param);
	//--------------------------------------
	// Kernel version 2.6.X -
	//--------------------------------------
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
		return pisr->handler(irq, pisr->param);
#endif
	}else{
	//--------------------------------------
	// Kernel version 2.2.X - 2.4.X
	//--------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
		return;
	//--------------------------------------
	// Kernel version 2.6.X -
	//--------------------------------------
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
		return IRQ_NONE;
#endif
	}
}

//================================================================
// Interrupt registration routine
//================================================================
// Kernel version 2.2.X - 2.4.X
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
long Ccom_connect_interrupt(PCCOM_ISR_OBJ pisr, unsigned long irq, unsigned int share, void (*handler)(int, void *), void *param, char *name)
// Kernel version 2.6.X -
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
long Ccom_connect_interrupt(PCCOM_ISR_OBJ pisr, unsigned long irq, unsigned int share, irqreturn_t (*handler)(int, void *), void *param, char *name)
#endif
{
	unsigned long	flags;

	//--------------------------------------
	// Check the parameters
	//--------------------------------------
	if (pisr == NULL ||
		(share != CCOM_IRQ_NOT_SHARE && share != CCOM_IRQ_SHARE) ||
		handler == NULL ||
		name == NULL ||
		irq == 0
		) {
		return CCOM_ERROR_PARAM;
	}
	//--------------------------------------
	// Store the parameters
	//--------------------------------------
	pisr->handler	= handler;
	pisr->param		= param;
	pisr->irq		= irq;
	//--------------------------------------
	// Set the flag
	//--------------------------------------
	if (share ==  CCOM_IRQ_SHARE) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
		flags	= IRQF_SHARED;
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22) && LINUX_VERSION_CODE < KERNEL_VERSION(4,1,0) )
		flags	= IRQF_DISABLED | IRQF_SHARED;
#else
		flags	= SA_INTERRUPT | SA_SHIRQ;
#endif
	} else {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,1,0))
		flags	= 0x0;
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22) && LINUX_VERSION_CODE < KERNEL_VERSION(4,1,0) )
		flags	= IRQF_DISABLED;
#else
		flags	= SA_INTERRUPT;
#endif
	}
	//--------------------------------------
	// Call the function
	//--------------------------------------
	if (request_irq((short)(irq & 0xffff), Ccom_isr, flags, name, pisr)) {
		return 	CCOM_ERROR_IRQ;
	}
	pisr->flag	= 1;
	return CCOM_NO_ERROR;
}

//================================================================
// Interrupt unregistration routine
//================================================================
void Ccom_disconnect_interrupt(PCCOM_ISR_OBJ pisr)
{
	//--------------------------------------
	// Check the parameters
	//--------------------------------------
	if (pisr == NULL || pisr->flag != 1) {
		return;
	}
	//--------------------------------------
	// Call the function
	//--------------------------------------
	free_irq((short)(pisr->irq & 0xffff), pisr);
}

//================================================================
// Interrupt synchronous execution routine
//================================================================
void Ccom_sync_interrupt(unsigned long irq, void (*func)(void *), void *param)
{
	unsigned long flags;

	//--------------------------------------
	// Kernel version 2.2.X - 2.4.X
	//--------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
	save_flags(flags);
	cli();
	//--------------------------------------
	// Kernel version 2.6.X -
	//--------------------------------------
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
	local_save_flags(flags);
	local_irq_disable();
#endif
	if (func != NULL) {
		func(param);
	}
	//----------------------------------------
	// Kernel version 2.2.X - 2.4.X
	//----------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
	restore_flags(flags);
	//----------------------------------------
	// Kernel version 2.6.X -
	//----------------------------------------
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
	local_irq_restore(flags);
#endif
}

//================================================================
// Wake up routine
//================================================================
void Ccom_wake_up(PCCOM_TASK_QUEUE tq)
{
	//--------------------------------------
	// If in the sleep state, wake up and set the flag
	//--------------------------------------
	if (tq->flag == CCOM_TQ_SLEEP_ON) {
		wake_up_interruptible(&tq->wq);
	}
	tq->flag = CCOM_TQ_NOT_SLEEP_ON;
}

//================================================================
// Memory allocation routine @note Used for allocating up to 128kbyte memory
//================================================================
void *Ccom_alloc_mem(unsigned int size)
{
	return kmalloc(size, GFP_KERNEL);
}

//================================================================
// Memory free routine
//================================================================
void Ccom_free_mem(void *obj)
{
	if (obj != NULL) {
		kfree(obj);
	}
}

//========================================================================
// Function name: Com_Alloc_pages
// Function     : Allocate memory. [get_free_pages()]
// In	        : flags	: Priority GFP_KERNEL or GFP_ATOMIC (Used when allocation will not sleep)
//		          size	: Memory size to allocate
// Return value : A pointer to the buffer
//========================================================================
void *Ccom_alloc_pages(int priority, unsigned int size)
{
	unsigned long page_log;

	//--------------------------------------
	// Kernel version 2.2.X
	//--------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0))
	size = (size-1) >> (PAGE_SHIFT-1);
	page_log = -1;
	do{
		size >>= 1;
		page_log++;
	}while(size);
	//--------------------------------------
	// Kernel version 2.4.X -
	//--------------------------------------
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0))
	page_log = get_order(size);
#endif
	//---------------------------
	//	Memory allocation
	//---------------------------
	return (void *)__get_free_pages(priority, page_log);
}

//========================================================================
// Function name: Ccom_free_pages
// Function     : Free the memory. [get_free_pages()]
// In	        : addr		:buffer
//		          size		:size to release
// Out	        :None
// Return value :None
//========================================================================
void Ccom_free_pages(unsigned long addr, unsigned int size)
{
	unsigned long page_log;

	//--------------------------------------
	// Kernel version 2.2.X
	//--------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0))
	size = (size-1) >> (PAGE_SHIFT-1);
	page_log = -1;
	do {
		size >>= 1;
		page_log++;
	} while (size);
	//--------------------------------------
	// Kernel version 2.4.X -
	//--------------------------------------
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0))
	page_log = get_order(size);
#endif
	//---------------------------
	//	Free the memory
	//---------------------------
	free_pages(addr, page_log);
}

//================================================================
//  us Wait routine
//================================================================
void Ccom_udelay(unsigned long usecs)
{
	udelay(usecs);
}

//================================================================
// ms Wait routine (The maximum resolution is 10ms when HZ=100)
//================================================================
void Ccom_sleep_on_timeout(long msecs)
{
	long	resolution;
	long	timeout;

	//--------------------------------------
	// Calculate jiffies
	//--------------------------------------
	resolution	= 1000 / HZ;			//	1000=1sec
	timeout		= msecs / resolution;
	// Round up if the remainder exist
	if(msecs % resolution){
		timeout++;
	}
	//----------------------------------------
	// Kernel version 2.2.X
	//----------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0))
	current->state	= TASK_INTERRUPTIBLE;
#else
	//----------------------------------------
	// Kernel version 2.4.X or later
	//----------------------------------------
	set_current_state(TASK_INTERRUPTIBLE);
#endif
	//--------------------------------------
	// Sleep
	//--------------------------------------
	schedule_timeout(timeout);
}

//================================================================
// Timer service routine
//================================================================
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0))
static void Ccom_timer(struct timer_list *param)
#else
static void Ccom_timer(unsigned long param)
#endif
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0))
	PCCOM_TIMER_DATA	ptimer = container_of(param, CCOM_TIMER_DATA, tm);
#else
	PCCOM_TIMER_DATA	ptimer = (PCCOM_TIMER_DATA)param;
#endif

	//--------------------------------------
	// Do nothing if the timer is stopped.
	//--------------------------------------
	if (ptimer->initialized == CCOM_TIMER_NOT_STARTED) {
		return;
	}
	//--------------------------------------
	// Execute the process
	//--------------------------------------
	if (ptimer->func != NULL) {
		ptimer->func(ptimer->data);
	}
	//--------------------------------------
	// Start the process in the next timer tick
	//--------------------------------------
	ptimer->tm.expires = jiffies + (ptimer->tim * HZ / 1000);
	add_timer(&ptimer->tm);
}

//================================================================
// Timer start entry
//================================================================
long Ccom_set_timer(unsigned int timer_id, unsigned long tim, void (*func)(void *), void *param)
{
	PCCOM_TIMER_DATA	ptimer;

	//--------------------------------------
	// Check the parameters
	//--------------------------------------
	if (timer_id >= CCOM_TIMER_MAX) {
		return CCOM_ERROR_TIMER_ID;
	}
	if (func == NULL) {
		return CCOM_ERROR_PARAM;
	}
	if ((tim * HZ / 1000) == 0) {
		return CCOM_ERROR_PARAM;
	}
	//--------------------------------------
	// Set the parameters
	//--------------------------------------
	ptimer			= &timer_data[timer_id];
	ptimer->func	= func;
	ptimer->tim		= tim;
	ptimer->data	= param;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0))
	//--------------------------------------
	// Initialize the timer if it has not been initialized
	//--------------------------------------
	if (ptimer->initialized == CCOM_TIMER_NOT_INITIALIZED) {
		timer_setup(&ptimer->tm, Ccom_timer, 0);
	}
#else
	//--------------------------------------
	// Initialize the timer if it has not been initialized
	//--------------------------------------
	if (ptimer->initialized == CCOM_TIMER_NOT_INITIALIZED) {
		init_timer(&ptimer->tm);
	}
	//--------------------------------------
	// Set the function
	//--------------------------------------
	ptimer->tm.function	= Ccom_timer;
	ptimer->tm.data		= (unsigned long)&timer_data[timer_id];
#endif
	//--------------------------------------
	// Exit if Timer is working
	//--------------------------------------
	if (ptimer->initialized == CCOM_TIMER_MOVING) {
		return CCOM_NO_ERROR;
	}
	//--------------------------------------
	// Call the timer function
	//--------------------------------------
	ptimer->initialized				= CCOM_TIMER_MOVING;
	timer_data[timer_id].tm.expires	= jiffies + (tim * HZ /1000);
	add_timer(&ptimer->tm);
	return CCOM_NO_ERROR;
}

//================================================================
// Timer stop entry
//================================================================
void Ccom_kill_timer(unsigned int timer_id)
{
	//--------------------------------------
	// Check the parameter
	//--------------------------------------
	if (timer_id >= CCOM_TIMER_MAX) {
		return;
	}
	//--------------------------------------
	// Stop timer (Timer is stopped automatically by setting flag)
	//--------------------------------------
	if (timer_data[timer_id].initialized == CCOM_TIMER_MOVING) {
		timer_data[timer_id].initialized = CCOM_TIMER_NOT_STARTED;
	}
}

//================================================================
// File read entry
//================================================================
//----------------------------------------
// Kernel version 2.2.X, 2.4.X and 2.6.x
//----------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0))
static int Ccom_file_read_proc(char *buf, char **start, off_t offset, int count, int *eof, void *data)
{
	int	return_length;

	//--------------------------------------
	// If no content in the file, do corresponding process
	//--------------------------------------
	if (raw_file_data.file_data == NULL) {
		*eof	= 1;
		return 0;
	}
	//--------------------------------------
	// Calculate the length of the string to copy
	//--------------------------------------
	if ((offset + count) > raw_file_data.file_len) {
		*eof	= 1;
		return_length	= raw_file_data.file_len - offset;
		if (return_length <= 0) {
			return 0;
		}
	} else {
		return_length = count;
	}
	//--------------------------------------
	// Copy buffer and return
	//--------------------------------------
	memcpy(buf, &raw_file_data.file_data[offset], return_length);
	*start	= buf;
	return return_length;
}
//--------------------------------------------
// Kernel version 3.10.X -
//--------------------------------------------
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
static ssize_t Ccom_file_read_proc(struct file *file, char __user *buf, size_t count, loff_t *data)
{
	long	return_length;
	int		iret;

	if ((tmp_read_off + count) > raw_file_data.file_len) {
		return_length	= raw_file_data.file_len - tmp_read_off;
		if (return_length <= 0) {
			tmp_read_off	= 0;
			return 0;
		}
	} else {
		return_length	= count;
	}
	//--------------------------------------
	// Copy buffer and return
	//--------------------------------------
	iret = copy_to_user(buf, &raw_file_data.file_data[tmp_read_off], return_length);
	if (iret != 0)
	{
		tmp_read_off	= 0;
		return 0;
	} else {
		tmp_read_off	+= return_length;
	}
	return return_length;
}
#endif

//================================================================
// File write entry
//================================================================
//----------------------------------------------
// Kernel version 2.2.x, 2.4.X and 2.6.X
//----------------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0))
static int Ccom_file_write_proc(struct file *file, const char *buf, unsigned long count, void *data)
//----------------------------------------------
// Kernel version 3.10.X -
//----------------------------------------------
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))
static ssize_t Ccom_file_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *data)
#endif
{
	unsigned long	copy_len;
	char			*prev = NULL;
	int				iret;
	int				cret;
	char			*pcbuf;
	
	copy_len	= count;
	pcbuf		= (char *)kmalloc(copy_len, GFP_KERNEL);
	cret		= copy_from_user(pcbuf, buf, copy_len);
	//--------------------------------------
	// If it is the beginning of the buffer,
	//--------------------------------------
	if (strncmp(pcbuf, "#Start", strlen("#Start")) == 0 ||
		strncmp(pcbuf, "#start", strlen("#start")) == 0) {
		//--------------------------------------
		// Clear the previous buffer
		//--------------------------------------
		kfree(raw_file_data.file_data);
		raw_file_data.file_data	= NULL;
		raw_file_data.file_len	= 0;
		raw_file_data.flag		= CCOM_RAW_FILE_FLAG_NONE;
	}
	kfree(pcbuf);
	//--------------------------------------
	// The module usage count is not 0,
	// The flag is read completed
	//--------------------------------------
	// Kernel version 2.2.X - 2.4.X
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
	if (MOD_IN_USE || raw_file_data.flag == CCOM_RAW_FILE_FLAG_COMPLETE) {
	// Kernel version 2.6.X -
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
	if (module_refcount(THIS_MODULE) || raw_file_data.flag == CCOM_RAW_FILE_FLAG_COMPLETE) {
#endif
		return 0;
	}
	//--------------------------------------
	// Allocate a new buffer and copy the previous data
	//--------------------------------------
	raw_file_data.flag	= CCOM_RAW_FILE_FLAG_READ;
	if (raw_file_data.file_data == NULL) {
		raw_file_data.file_data	= (char *)kmalloc(count+1, GFP_KERNEL);
		if (raw_file_data.file_data == NULL) {
			return ENOMEM;
		}
	} else {
		prev	= raw_file_data.file_data;
		raw_file_data.file_data	= (char *)kmalloc(count + raw_file_data.file_len + 1, GFP_KERNEL);
		if (raw_file_data.file_data == NULL) {
			return ENOMEM;
		}
		memcpy(raw_file_data.file_data, prev, raw_file_data.file_len);
		kfree(prev);
	}
	//--------------------------------------
	// Copy the current data
	//--------------------------------------
	copy_len	= count;
	iret		= copy_from_user(&raw_file_data.file_data[raw_file_data.file_len], buf, copy_len);
	if (iret) {
		return -EFAULT;
	}
	raw_file_data.file_len += copy_len;
	raw_file_data.file_data[raw_file_data.file_len] = '\0';
	//--------------------------------------
	// If it is the end of the buffer,
	//--------------------------------------
	if (strstr(raw_file_data.file_data, "#End") != NULL ||
		strstr(raw_file_data.file_data, "#end") != NULL) {
		//--------------------------------------
		// Set the flag
		//--------------------------------------
		raw_file_data.flag = CCOM_RAW_FILE_FLAG_COMPLETE;
		//--------------------------------------
		// Parse the file and add device
		//--------------------------------------
		iret = Ccom_sub_add_device();
		if (iret != 0) {
			return iret;
		}
	}
	return copy_len;
}

//================================================================
// Subroutine: Convert string to hexadecimal
//================================================================
unsigned long Ccom_str_to_hex(char *str)
{
	unsigned long	num;
	int				i;
	int				len;

	len	= strlen(str);
	if (len > 8) {
		len = 8;
	}
	num	= 0;
	for (i=0; i<len; i++) {
		if (str[i] >= '0' && str[i] <= '9') {
			num = num * 16 + (str[i] - '0');
		} else if (str[i] >= 'a' && str[i] <= 'f') {
			num = num * 16 + (str[i] - 'a' + 10);
		} else if (str[i] >= 'A' && str[i] <= 'F') {
			num = num * 16 + (str[i] - 'A' + 10);
		} else {
			num = 0;
			break;
		}
	}
	return num;
}

//================================================================
// Subroutine: Convert string to decimal
//================================================================
unsigned long Ccom_str_to_dec(char *str)
{
	unsigned long	num;
	int				i;
	int				len;

	len	= strlen(str);
	num	= 0;
	for (i=0; i<len; i++) {
		if (str[i] >= '0' && str[i] <= '9') {
			num	= num * 10 + (str[i] - '0');
		} else {
			num	= 0;
			break;
		}
	}
	return num;
}

//================================================================
// Subroutine: Parser of the file raw data
//================================================================
int Ccom_sub_add_device(void)
{
	int						i;
	unsigned long			ul;
	char					line_data[256];
	int						line_pos;
	char					*file_data;
	char					value_name[CCOM_FILE_VALUE_NAME_LEN];
	char					str[CCOM_FILE_VALUE_LEN];
	unsigned long			num;
	short					type;
	unsigned long			file_len;
	int						st;
	int						value_num;
	int						key_num;
	long					lret;
	int						itype;
	static	CCOM_FILE_DATA	com_file_data;
	PCCOM_FILE_DATA 		ptmp;
	unsigned short			device_type;
	char					device_name[CCOM_DEVICE_NAME_MAX];
	char					file_node[CCOM_FILE_NODE_NAME_MAX];
	long					minor;
	int						len;
	int						iret = 0;

	//--------------------------------------
	// Delete the device information
	//--------------------------------------
	for (i=0; i<device_list.device_num; i++) {
		//--------------------------------------
		// Call the delete entry
		//--------------------------------------
		if (device_list.entry.delete_device != NULL) {
			device_list.entry.delete_device(device_list.device[i].device_data);
		}
		device_list.device[i].device_data	= NULL;
		//--------------------------------------
		// Delete the file data
		//-------------------------------------
		if (device_list.device[i].file_data != NULL) {
			kfree(device_list.device[i].file_data);
			device_list.device[i].file_data	= NULL;
		}
	}
	device_list.device_num	= 0;
	//--------------------------------------
	// Retrieve the file data by line
	//--------------------------------------
	file_data	= &raw_file_data.file_data[0];
	file_len	= raw_file_data.file_len;
	line_pos	= 0;	// The number of characters per line
	value_num	= 0;	// The number of values per key
	key_num		= 0;	// The number of keys per device
	memset(&com_file_data, 0, sizeof(com_file_data));
	// There is one key at least
	com_file_data.key_num	= 1;
	for (ul=0; ul<file_len; ul++) {
		//--------------------------------------
		// Process when a new line come
		//--------------------------------------
		if (file_data[ul] == '\n') {
			line_data[line_pos] = '\0';
			//--------------------------------------
			// Determine the type
			//--------------------------------------
			switch (line_data[0]) {
			case '#':		// Comment
				// Do nothing
				break;
			case '{':		// Block start
				key_num		= 0;
				value_num	= 0;
				break;
			case '}':		// Block end
				// Allocate memory
				ptmp	= kmalloc(sizeof(com_file_data), GFP_KERNEL);
				if (ptmp == NULL) {
					goto end_loop;
				}
				// Copy,
				memcpy(ptmp, &com_file_data, sizeof(com_file_data));

				// If the device type is DEVICE_TYPE_PCICadd into the device list.
				lret	= Ccom_read_value(ptmp, "", "DeviceType", &itype, (void *)&device_type);
				if (device_type == DEVICE_TYPE_PCI)
				{
					// Add into the device list
					device_list.device[device_list.device_num].file_data = ptmp;
					device_list.device_num++;
				}
				value_num	= 0;	// The number of values per key
				key_num		= 0;
				memset(&com_file_data, 0, sizeof(com_file_data));
				com_file_data.key_num = 1;
				break;
			case '[':		// Key
				// Check the limit
				if (com_file_data.key_num >= CCOM_FILE_MAX_KEY) {
					break;
				}
				// Create a key string
				for (i=1; strlen(line_data); i++) {
					if (line_data[i] == ']') {
						line_data[i]	= '\0';
						break;
					}
				}
				// Copy the key
				key_num++;
				com_file_data.key_num++;
				value_num	= 0;	// The number of values per key
				strcpy(com_file_data.key_data[key_num].key, &line_data[1]);
				break;
			default:		// Value
				// Check the limit
				if (value_num >= CCOM_FILE_MAX_VALUE) {
					break;
				}
				// Initialize variables
				type = CCOM_FILE_VALUE_TYPE_NUM;
				num	= 0;
				strcpy(value_name, "");
				strcpy(str, "");
				st	= 0;
				len	= strlen(line_data);
				// Parse the value names
				for (i=st; i<len; i++) {
					if (line_data[i] == ':') {
						line_data[i]	= '\0';
						strcpy(value_name, &line_data[st]);
						st	= i+1;
						break;
					}
				}
				// Parse the types
				for (i=st; i<len; i++) {
					if (line_data[i] == ':') {
						line_data[i]	= '\0';
						if (strcmp(&line_data[st], "String") == 0 ||
							strcmp(&line_data[st], "string") == 0) {
							type	= CCOM_FILE_VALUE_TYPE_STR;
						}
						st	= i+1;
						break;
					}
				}
				// Parse the value
				strcpy(str, &line_data[st]);
				// Numeric hexadecimal
				if ((str[0] == '0' && str[1] == 'x') ||
					(str[0] == '0' && str[1] == 'X')) {
					num	= Ccom_str_to_hex(&str[2]);
				}
				// Numeric decimal
				else {
					num	= Ccom_str_to_dec(str);
				}
				// Copy
				com_file_data.key_data[key_num].value_data[value_num].type	= type;
				com_file_data.key_data[key_num].value_data[value_num].num	= num;
				strcpy(com_file_data.key_data[key_num].value_data[value_num].value_name, value_name);
				strcpy(com_file_data.key_data[key_num].value_data[value_num].str, str);
				value_num++;
				com_file_data.key_data[key_num].value_num++;
				break;
			}
			//--------------------------------------
			// Determine the type
			//--------------------------------------
			line_pos	= 0;
		}
		//--------------------------------------
		// Accumulate in the buffer if that is not a new line
		//--------------------------------------
		else {
			line_data[line_pos]	= file_data[ul];
			line_pos++;
		}
	}
end_loop:
	//--------------------------------------
	// Finally, add the found devices
	//--------------------------------------
	for (i=0; i<device_list.device_num; i++) {
		ptmp	= device_list.device[i].file_data;
		// Search for device name
		lret	= Ccom_read_value(ptmp, "", "DeviceName", &itype, (void *)device_name);
		if (lret != 0) {
			continue;
		}
		strcpy(device_list.device[i].device_name, device_name);
		// Search for file node
		lret	= Ccom_read_value(ptmp, "", "FileNode",   &itype, (void *)file_node);
		if (lret != 0) {
			continue;
		}
		strcpy(device_list.device[i].file_node, file_node);
		// Search for minor number
		lret	= Ccom_read_value(ptmp, "", "Minor",      &itype, (void *)&minor);
		if (lret != 0) {
			continue;
		}
		device_list.device[i].minor	= (int)minor;
		// Add device
		if (device_list.entry.add_device) {
			iret	= device_list.entry.add_device(&device_list.device[i].device_data, ptmp);
			if (iret != 0) {
				return iret;
			}
		}
	}

	return iret;
}

//================================================================
// File data: Read values
//================================================================
long Ccom_read_value(PCCOM_FILE_DATA fdata, char *key, char *value_name, int *type, void *value)
{
	int		i, j;
	long	lret;

	//--------------------------------------
	// Check the parameters
	//--------------------------------------
	if (fdata == NULL ||
		key == NULL ||
		value_name == NULL ||
		type == NULL ||
		value == NULL) {
		return CCOM_ERROR_PARAM;
	}
	//--------------------------------------
	// Loop the key
	//--------------------------------------
	lret	= CCOM_ERROR_NO_KEY;
	for (i=0; i<fdata->key_num; i++) {
		//--------------------------------------
		// Begin the next iteration of the loop if the key is not matched
		//--------------------------------------
		if (strcmp(fdata->key_data[i].key, key) != 0) {
			continue;
		}
		//--------------------------------------
		// If the key is matched
		//--------------------------------------
		lret	= CCOM_ERROR_NO_VALUE;
		//--------------------------------------
		// Loop the value
		//--------------------------------------
		for (j=0; j<fdata->key_data[i].value_num; j++) {
			//--------------------------------------
			// If the value is matched, copy and return
			//--------------------------------------
			if (strcmp(fdata->key_data[i].value_data[j].value_name, value_name) == 0) {
				*type = fdata->key_data[i].value_data[j].type;
				if (*type == CCOM_FILE_VALUE_TYPE_NUM) {
					*(unsigned long *)value = fdata->key_data[i].value_data[j].num;
				} else {
					strcpy((char *)value, fdata->key_data[i].value_data[j].str);
				}
				return CCOM_NO_ERROR;
			}
		}
	}
	return lret;
}

//================================================================
// File data: Enumerate the keys
//================================================================
long Ccom_enum_key(PCCOM_FILE_DATA fdata, int index, char *key)
{
	//--------------------------------------
	// Check the parameters
	//--------------------------------------
	if (fdata == NULL ||
		key == NULL) {
		return CCOM_ERROR_PARAM;
	}
	if (index >= fdata->key_num) {
		return CCOM_ERROR_NO_KEY;
	}
	//--------------------------------------
	// Copy the key and return
	//--------------------------------------
	strcpy(key, fdata->key_data[index].key);
	return CCOM_NO_ERROR;
}

//================================================================
// File data: Retrieve the setting file information
//================================================================
PCCOM_FILE_DATA Ccom_get_file_ptr(void *device_ext)
{
	int		i;

	//--------------------------------------
	// Cheack the parameters
	//--------------------------------------
	if (device_ext == NULL) {
		return NULL;
	}
	//--------------------------------------
	// Search the device list
	//--------------------------------------
	for (i=0; i<device_list.device_num; i++) {
		//--------------------------------------
		// Return the address of the file information structure when the device information is matched
		//--------------------------------------
		if (device_ext == device_list.device[i].device_data) {
			return device_list.device[i].file_data;
		}
	}
	return NULL;
}

//================================================================
// Function for retrieving PCI resource
//================================================================
long Ccom_get_pci_resource(
	unsigned short vendor_id,
 	unsigned short device_id,
	unsigned short board_id,
	unsigned long *io_addr,
	unsigned long *io_num,
	unsigned long *mem_addr,
	unsigned long *mem_num,
	unsigned long *irq)
{
	struct pci_dev	*ppci_dev = NULL;
	unsigned char	revision;
	unsigned char	irq_pin;
	int				found_num;
	unsigned long	io_tmp[6];
	unsigned long	io_num_tmp;
	unsigned long	mem_tmp[6];
	unsigned long	mem_num_tmp;
	unsigned long	irq_tmp;
	unsigned long	i;

	//----------------------------------------
	// If PCI bus is not present, return No Board Error
	//----------------------------------------
	//----------------------------------------
	// Kernel version 2.2.X - 2.4.X
	//----------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
	if (!pci_present()) {
		return CCOM_ERROR_NO_BOARD;
	}
#endif
	//----------------------------------------
	// If the area is not sufficient, return Array Size Error
	//----------------------------------------
	if (io_addr == NULL || *io_num < 6) {
		return CCOM_ERROR_ARRY_SIZE;
	}
	if (mem_addr == NULL || *mem_num < 6) {
		return CCOM_ERROR_ARRY_SIZE;
	}
	if (irq == NULL) {
		return CCOM_ERROR_ARRY_SIZE;
	}
	//----------------------------------------
	// Prepare and initialize the return data
	// for the case of no board found
	//----------------------------------------
	*io_num		= 0;
	*mem_num	= 0;
	*irq		= 0xffffffff;
	//----------------------------------------
	// Initialize the temporary data and the number of boards
	//----------------------------------------
	io_num_tmp	= 0;
	mem_num_tmp	= 0;
	irq_tmp		= 0xffffffff;
	found_num	= 0;
	//----------------------------------------
	// For the found PCI board
	//----------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21))
	while ((ppci_dev = pci_get_device(vendor_id, device_id, ppci_dev))) {
#else
	while ((ppci_dev = pci_find_device(vendor_id, device_id, ppci_dev))) {
#endif
		//----------------------------------------
		// Pass the case that the vendor ID and the device ID do not match with each one you want
		//----------------------------------------
		if (!(ppci_dev->vendor == vendor_id && ppci_dev->device == device_id)) {
			continue;
		}
		//----------------------------------------
		// Pass the case that the revision ID (Board ID) does not match with that you want
		//----------------------------------------
		pci_read_config_byte(ppci_dev,PCI_REVISION_ID,&revision);
		if (revision != board_id) {
			continue;
		}
		//----------------------------------------
		// Enable the PCI device
		//----------------------------------------
		if(pci_enable_device(ppci_dev)) {
			return CCOM_ERROR_NO_BOARD;
		}
		//----------------------------------------
		// Board is found!
		//----------------------------------------
		found_num++;
		io_num_tmp = 0;
		mem_num_tmp = 0;
		irq_tmp = 0xffffffff;
		//----------------------------------------
		// Loop for retrieving I/O and memory
		//----------------------------------------
		for (i=0; i<6 ;i++) {
			//----------------------------------------
			// Kernel version 2.4.X - 
			//----------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0))
			if (!pci_resource_start(ppci_dev, i)) {
   				break;
			}
			if (pci_resource_flags(ppci_dev, i) & IORESOURCE_IO) {
				io_tmp[io_num_tmp]	= pci_resource_start(ppci_dev, i);
				io_num_tmp++;
			} else {
				mem_tmp[mem_num_tmp]	= pci_resource_start(ppci_dev, i) & PCI_BASE_ADDRESS_MEM_MASK;
				mem_num_tmp++;
			}
#endif
			//----------------------------------------
			// Kernel version 2.2.X
			//----------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0))
			if ( ppci_dev->base_address[i] & PCI_BASE_ADDRESS_SPACE_IO) {
				io_tmp[io_num_tmp]	= ppci_dev->base_address[i] & PCI_BASE_ADDRESS_IO_MASK;
				io_num_tmp++;
			} else if ( ppci_dev->base_address[i] & PCI_BASE_ADDRESS_SPACE_MEMORY) {
				mem_tmp[mem_num_tmp]	=  ppci_dev->base_address[i] & PCI_BASE_ADDRESS_MEM_MASK;
				mem_num_tmp++;
			}
#endif
		}
		//----------------------------------------
		// Retrieve IRQ
		//----------------------------------------
		pci_read_config_byte(ppci_dev, PCI_INTERRUPT_PIN, &irq_pin);
		if (irq_pin != 0 && ppci_dev->irq != 0) {
			irq_tmp = ppci_dev->irq;
		}
	}
	//----------------------------------------
	// Return an error if the found number is equal to 0 or more than 1
	//----------------------------------------
	if (found_num == 0) {
		return CCOM_ERROR_NO_BOARD;
	}
	if (found_num > 1) {
		return CCOM_ERROR_BOARDID;
	}
	//----------------------------------------
	// If the found number is equal to 1, copy the resource and return
	//----------------------------------------
	for (i=0; i<io_num_tmp; i++) {
		io_addr[i]	= io_tmp[i];
	}
	*io_num = io_num_tmp;
	for (i=0; i<mem_num_tmp; i++) {
		mem_addr[i]	= mem_tmp[i];
	}
	*mem_num	= mem_num_tmp;
	*irq		= irq_tmp;
	return CCOM_NO_ERROR;
}

//================================================================
// Report I/O address
//================================================================
long Ccom_report_io(unsigned long port, unsigned long range, char *name)
{
	//--------------------------------------
	// Check the parameters
	//--------------------------------------
	if (name == NULL) {
		return CCOM_ERROR_PARAM;
	}
	//--------------------------------------
	// Check the range
	//--------------------------------------
	// Kernel version 2.2.X
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0))
	if (check_region(port, range) < 0) {
		return CCOM_ERROR_IO_ADDRESS;
	}
#endif
	//--------------------------------------
	// Report
	//--------------------------------------
	request_region(port, range, name);
	return CCOM_NO_ERROR;
}

//================================================================
// Unreport I/O address
//================================================================
long Ccom_unreport_io(unsigned long port, unsigned long range)
{
	release_region(port, range);
	return CCOM_NO_ERROR;
}
