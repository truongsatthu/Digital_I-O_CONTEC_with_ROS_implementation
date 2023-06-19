////////////////////////////////////////////////////////////////////////////////
/// @file   Cdio_dispatch.h
/// @brief  API-DIO(LNX) PCI Module - Dispatch header file
/// @author &copy;CONTEC CO.,LTD.
/// @since  2002
////////////////////////////////////////////////////////////////////////////////

#ifndef __CDIO_DISPATCH_H_DEFINED__
#define __CDIO_DISPATCH_H_DEFINED__
#include "../library/Cdio_ctrl.h"
#include "Ccom_module.h"
#include "Cdio_bios.h"
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
	#include "Cdio_dm_dispatch.h"
#endif

//================================================================
// Definition values
//================================================================
#define	CDIO_MAX_PROCESS			16							// The maximum number of processes
//================================================================
// Structure
//================================================================
//----------------------------------------------------------------
// Trigger information structure
// : Background data structure
//----------------------------------------------------------------
typedef struct {
	short				logic[CDIO_MAX_TRG_BITS];				// Detected logic
	short				comp[CDIO_MAX_TRG_BITS];				// Comparison result
	short				comp_valid;								// Store comparison result Y/N (0:N, 1:Y)
	short				num;									// The maximum number of compare bits
	unsigned long		tim;									// Comparison cycle
	unsigned short		timer_id;								// Timer ID
	unsigned char		back_data[CDIO_MAX_TRG_BITS/8];			// Previous input value
	void				*dio_data;								// Parent DIO_DATA
} CDIO_TRG_DATA, *PCDIO_TRG_DATA;
//----------------------------------------------------------------
// Interrupt information structure
// : Background data structure
//----------------------------------------------------------------
typedef struct {
	short				max_bit;								// The maximum interrupt bit
	short				sence_num;								// The number of sence in the queue
	unsigned char		sence_arry[CDIO_MAX_SENCE_NUM][CDIOBI_INT_SENCE_SIZE];	// Sence queue
	unsigned char		mask[CDIOBI_INT_SENCE_SIZE];
												
} CDIO_INT_DATA, *PCDIO_INT_DATA;
//----------------------------------------------------------------
// Process information structure
// : Background data structure
//----------------------------------------------------------------
typedef struct {
	CCOM_TASK_QUEUE		wait_obj;								// Wait structure
	int					process_id;								// Process ID
	CDIO_TRG_DATA		trg_data;								// Trigger data
	CDIO_INT_DATA		int_data;								// Interrupt data
} CDIO_PROC_DATA, *PCDIO_PROC_DATA;
//----------------------------------------------------------------
// Device information structure
// : Dispatch data structure
//----------------------------------------------------------------
typedef struct {
	char				board_name[CDIOBI_BOARD_NAME_MAX];		// Board name
	char				device_name[CDIOBI_DEVICE_NAME_MAX];	// Device name
	char				display_name[CDIOBI_DEVICE_NAME_MAX];	// Display name
	unsigned short		minor;									// Minor number
	CDIOBI				bios_d;									// BIOS information
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
	BMBOARD				bm_board;								// Data structure for DM
#endif
	unsigned short		ref_count;								// Reference count of each device
	unsigned short		port_base;								// Base address of port
	unsigned short		port_num;								// Number of occupied ports
	unsigned short		irq_no;									// IRQ
	unsigned long		board_id;								// Board ID
	CCOM_ISR_OBJ		isr_obj;								// Interrupt structure
	long				g_ret;									// Global error
	CDIO_PROC_DATA		proc_data[CDIO_MAX_PROCESS];			// Data of each process
	spinlock_t			spinlock_flag;							// Spinlock flag
	struct semaphore	mutex_flag;								// Metux flag
} CDIO_DATA, *PCDIO_DATA;
//================================================================
// Prototype declaration
//================================================================
int Cdio_add_device(void **pdevice_ext, PCCOM_FILE_DATA fdata);
int Cdio_delete_device(void *device_ext);
int Cdio_open(void *device_ext);
int Cdio_close(void *device_ext);
int Cdio_dispatch(void *device_ext, unsigned int ctl_code, void *param);

long Cdio_init(void *device_ext, PCDIO_INIT param);
long Cdio_exit(void *device_ext, PCDIO_EXIT param);
long Cdio_reset(void *device_ext, PCDIO_RESET param);
long Cdio_set_digital_filter(void *device_ext, PCDIO_DFILTER param);
long Cdio_inout(void *device_ext, PCDIO_INOUT param);
long Cdio_set_interrupt_event(void *device_ext, PCDIO_SET_INT param);
long Cdio_set_trigger_event(void *device_ext, PCDIO_SET_TRG param);
long Cdio_get_device_info(void *device_ext, PCDIO_GET_INFO param);
long Cdio_wait_event(void *device_ext, PCDIO_TH_WAIT param);
long Cdio_wake_up_event(void *device_ext, PCDIO_TH_WAKE_UP param);
long Cdio_get_interrupt_factor(void *device_ext, PCDIO_TH_GET_INT param);
long Cdio_get_trigger_factor(void *device_ext, PCDIO_TH_GET_TRG param);
long Cdio_set_8255_mode(void *device_ext, PCDIO_SET_8255MODE param);
long Cdio_get_8255_mode(void *device_ext, PCDIO_GET_8255MODE param);
void Cdio_trigger_timer(void *param);
// Kernel version 2.2.X - 2.4.X
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
void Cdio_isr(int irq, void *param);
// Kernel version 2.6.X and 3.10.X
#elif ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
irqreturn_t Cdio_isr(int irq, void *param);
#endif
#endif

