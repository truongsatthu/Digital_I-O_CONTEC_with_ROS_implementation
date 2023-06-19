////////////////////////////////////////////////////////////////////////////////
/// @file   Cdio_dispatch.c
/// @brief  API-DIO(LNX) PCI Module - Dispatch source file
/// @author &copy;CONTEC CO.,LTD.
/// @since  2002
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
#include <linux/sched.h>

#include "Ccom_module.h"
#include "Cdio_dispatch.h"
#include "Cdio_bios.h"
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	(LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
	#include "Cdio_dm_dispatch.h"
#endif
void Cdio_user_irq(char *board_name, unsigned char board_id, unsigned short io_addr, unsigned char *status);

//================================================================
// Function for registering entry point
//================================================================
int Ccom_register_entry(CCOM_ENTRY *ccom_entry)
{
	//----------------------------------------
	// Register entry point and data
	//----------------------------------------
	strcpy(ccom_entry->name,	"cdio");				// Entry name
	ccom_entry->add_device		= Cdio_add_device;		// Entry for adding a device
	ccom_entry->delete_device	= Cdio_delete_device;	// Entry for deleting a device
	ccom_entry->open			= Cdio_open;			// Entry for open
	ccom_entry->close			= Cdio_close;			// Entry for close
	ccom_entry->dispatch		= Cdio_dispatch;		// Enfor for dispatch
	strcpy(ccom_entry->file_name, "contec_dio.conf");	// Setting file name
	return 0;
}

//================================================================
// Entry for adding a device
//================================================================
int Cdio_add_device(void **pdevice_ext, PCCOM_FILE_DATA fdata)
{
	static	char 	device_name[CDIOBI_DEVICE_NAME_MAX];
	static	char	device[CDIOBI_BOARD_NAME_MAX];
	int				itype;
	long			lret;
	unsigned long	vendor_id, device_id, board_id;
	void			*device_ext;
	PCDIO_DATA		dio_data;
	unsigned long	io_addr[6];
	unsigned long	io_num;
	unsigned long	mem_addr[6];
	unsigned long	mem_num;
	unsigned long	irq;
	unsigned long	minor;

#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	(LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
   	struct pci_dev	*PciDev;
	PciDev = (struct pci_dev *)kmalloc(sizeof(struct pci_dev), GFP_KERNEL);
	if (PciDev == NULL) {
		return -ENOMEM;
	}
#endif

	//----------------------------------------
	// Allocate the memory for device information and initialize the memory with zero
	//----------------------------------------
	device_ext = Ccom_alloc_pages(GFP_KERNEL, sizeof(CDIO_DATA));
	if (device_ext == NULL) {
		return -ENOMEM;
	}
	*pdevice_ext = device_ext;
	memset(device_ext, 0, sizeof(CDIO_DATA));
	dio_data = (PCDIO_DATA)device_ext;
	//----------------------------------------
	// Initialize the spinlock
	//----------------------------------------
	spin_lock_init(&dio_data->spinlock_flag);
	//----------------------------------------
	// Initialize the mutex
	//----------------------------------------
//	init_MUTEX(&dio_data->mutex_flag);
	//----------------------------------------
	// Read information from the setting file
	//----------------------------------------
	// Device name
	lret = Ccom_read_value(fdata, "", "DeviceName", &itype, (void *)device_name);
	if (lret != CCOM_NO_ERROR) {
		dio_data->g_ret = DIO_ERR_INI_REGISTRY;
		return 0;
	}
	strcpy(dio_data->device_name, device_name);
	// Device
	lret = Ccom_read_value(fdata, "", "Device", &itype, (void *)device);
	if (lret != CCOM_NO_ERROR) {
		dio_data->g_ret = DIO_ERR_INI_REGISTRY;
		return 0;
	}
	strcpy(dio_data->board_name, device);
	// Vendor ID
	lret = Ccom_read_value(fdata, "", "VendorID", &itype, (void *)&vendor_id);
	if (lret != CCOM_NO_ERROR) {
		dio_data->g_ret = DIO_ERR_INI_REGISTRY;
		return 0;
	}
	// Device ID
	lret = Ccom_read_value(fdata, "", "DeviceID", &itype, (void *)&device_id);
	if (lret != CCOM_NO_ERROR) {
		dio_data->g_ret = DIO_ERR_INI_REGISTRY;
		return 0;
	}
	// Board ID
	lret = Ccom_read_value(fdata, "", "BoardID", &itype, (void *)&board_id);
	if (lret != CCOM_NO_ERROR) {
		dio_data->g_ret = DIO_ERR_INI_REGISTRY;
		return 0;
	}
	// Minor number
	lret = Ccom_read_value(fdata, "", "Minor", &itype, (void *)&minor);
	if (lret != CCOM_NO_ERROR) {
		dio_data->g_ret = DIO_ERR_INI_REGISTRY;
		return 0;
	}
	//----------------------------------------
	// If the BusMaster library is supported in the kernel
	//----------------------------------------
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
	// BusMaster transfer  Output delay time
	lret = Ccom_read_value(fdata, "", "WaitTimeWhenOutput", &itype, (void *)&dio_data->bm_board.WaitTimeWhenOutput);
	if (lret != CCOM_NO_ERROR) {
		dio_data->g_ret = DIO_ERR_INI_REGISTRY;
		return 0;
	}
#endif
	//----------------------------------------
	// Search the resources
	//----------------------------------------
	memset(io_addr, 0, sizeof(io_addr));
	memset(mem_addr, 0, sizeof(mem_addr));
	io_num	= 6;
	mem_num	= 6;
	//----------------------------------------
	// If the BusMaster library is supported in the kernel
	//----------------------------------------
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
	//----------------------------------------
	// Use the BusMaster library edition
	//----------------------------------------
	lret = ApiBmGetPciResource(	(unsigned short)vendor_id,
								(unsigned short)device_id,
								(unsigned short)board_id,
								io_addr, &io_num,
								mem_addr, &mem_num,
								&irq, PciDev);
	if (lret != BM_ERROR_SUCCESS) {
		if (lret == BM_ERROR_BOARDID) {
			dio_data->g_ret	= DIO_ERR_INI_BOARD_ID;
		} else {
			dio_data->g_ret	= DIO_ERR_INI_RESOURCE;
		}
		return 0;
	}
#else
	//----------------------------------------
	// Use the common library edition
	//----------------------------------------
	lret = Ccom_get_pci_resource(	(unsigned short)vendor_id,
									(unsigned short)device_id,
									(unsigned short)board_id,
									io_addr, &io_num,
									mem_addr, &mem_num,
									&irq);
	if (lret != CCOM_NO_ERROR) {
		if (lret == CCOM_ERROR_BOARDID) {
			dio_data->g_ret	= DIO_ERR_INI_BOARD_ID;
		} else {
			dio_data->g_ret	= DIO_ERR_INI_RESOURCE;
		}
		return 0;
	}
#endif
	//----------------------------------------
	// Initialize the BIOS
	//----------------------------------------
	lret = Cdiobi_init_bios(&dio_data->bios_d, dio_data->board_name, io_addr, irq);
	if (lret != 0) {
		dio_data->g_ret	= DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	dio_data->port_base	= (unsigned short)io_addr[0];
	dio_data->board_id	= board_id;
	
	Cdiobi_get_info(dio_data->board_name, IDIO_NUM_OF_PORT, (void *)&dio_data->port_num, NULL, NULL);
	dio_data->irq_no	= (unsigned short)irq;
	dio_data->minor		= (unsigned short)minor;
	//----------------------------------------
	// Report the I/O address
	//----------------------------------------
	sprintf(dio_data->display_name, "%s_%ld", dio_data->board_name, board_id);
	lret	= Ccom_report_io(dio_data->port_base, dio_data->port_num, dio_data->display_name);
	if (lret != CCOM_NO_ERROR) {
		dio_data->g_ret	= DIO_ERR_INI_RESOURCE;
		return 0;
	}
	//----------------------------------------
	// If the BusMaster library is supported in the kernel
	//----------------------------------------
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
	//----------------------------------------
	// If it is DM board
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type == CDIOBI_BT_PCI_DM){
		dio_data->bm_board.dwAddr	= io_addr[0];
		dio_data->bm_board.dwAddrBM	= io_addr[1];
		//----------------------------------------
		// Report the I/O address of BusMaster
		//----------------------------------------
		sprintf(dio_data->display_name, "%s_%ld", dio_data->board_name, board_id);
		lret	= Ccom_report_io(dio_data->bm_board.dwAddrBM, 64, dio_data->display_name);
		if (lret != CCOM_NO_ERROR) {
			dio_data->g_ret	= DIO_ERR_INI_RESOURCE;
			return 0;
		}
		//----------------------------------------
		// Initialize the BusMaster library
		//----------------------------------------
		lret = ApiBmCreate(&dio_data->bm_board.Master, PciDev, dio_data->bm_board.dwAddrBM, dio_data->bm_board.dwAddr);
		if (lret == BM_ERROR_MEM) {
			dio_data->g_ret	= DIO_ERR_INI_MEMORY;
			return 0;
		}else if(lret == BM_ERROR_PARAM){
			dio_data->g_ret	= DIO_ERR_INI_RESOURCE;
			return 0;
		}
	}
#endif
	//----------------------------------------
	// Register the interrupt
	//----------------------------------------
	if (irq != 0xffffffff) {
		lret	= Ccom_connect_interrupt(&dio_data->isr_obj, irq, CCOM_IRQ_SHARE, Cdio_isr, dio_data, dio_data->display_name);
		if (lret != CCOM_NO_ERROR) {
			dio_data->g_ret	= DIO_ERR_INI_RESOURCE;
			return 0;
		}
	}
	return 0;
}

//================================================================
// Entry for deleting a device
//================================================================
int Cdio_delete_device(void *device_ext)
{
	PCDIO_DATA	dio_data;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data	= (PCDIO_DATA)device_ext;
	if (dio_data == NULL) {
		return -ENODEV;
	}
	//----------------------------------------
	// If the BusMaster library is supported in the kernel
	//----------------------------------------
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
	//----------------------------------------
	// If it is DM board
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type == CDIOBI_BT_PCI_DM){
		ApiBmDestroy(&dio_data->bm_board.Master);
		Ccom_unreport_io(dio_data->bm_board.dwAddrBM, 64);
	}
#endif
	//----------------------------------------
	// Mask the interrupt
	//----------------------------------------
	Cdiobi_set_all_int_mask(&dio_data->bios_d, CDIOBI_MASK_ALL, NULL);
	//----------------------------------------
	// Unreport the I/O address
	//----------------------------------------
	Ccom_unreport_io(dio_data->port_base, dio_data->port_num);
	//----------------------------------------
	// Unregister the interrupt
	//----------------------------------------
	Ccom_disconnect_interrupt(&dio_data->isr_obj);
	//----------------------------------------
	// Disable the BIOS
	//----------------------------------------
	Cdiobi_clear_bios(&dio_data->bios_d);
	//----------------------------------------
	// Free the memory
	//----------------------------------------
	Ccom_free_pages((unsigned long)dio_data, sizeof(CDIO_DATA));
	return 0;
}

//================================================================
// Entry for open
//================================================================
int Cdio_open(void *device_ext)
{
	PCDIO_DATA	dio_data;
	short		i;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data = (PCDIO_DATA)device_ext;
	if (dio_data == NULL) {
		return -ENODEV;
	}
	//----------------------------------------
	// Check the global error
	//----------------------------------------
	if (dio_data->g_ret != DIO_ERR_SUCCESS) {
		return 0;
	}
	//----------------------------------------
	// Check the reference count
	//----------------------------------------
	if (dio_data->ref_count == 0) {
		//----------------------------------------
		// Initialize the work of process
		//----------------------------------------
		memset(&dio_data->proc_data, 0, sizeof(dio_data->proc_data));
		for (i=0; i<CDIO_MAX_PROCESS; i++) {
			// Determine the timer ID
			dio_data->proc_data[i].trg_data.timer_id = (unsigned short)((dio_data->minor * CDIO_MAX_PROCESS) + i + 1);
			// Mask all the interrupts
			memset(&dio_data->proc_data[i].int_data.mask, 0xff, sizeof(dio_data->proc_data[i].int_data.mask));
		}
		//----------------------------------------
		// Mask the interrupt
		//----------------------------------------
		Cdiobi_set_all_int_mask(&dio_data->bios_d, CDIOBI_MASK_ALL, NULL);
	}
	//----------------------------------------
	// Plus the reference count
	//----------------------------------------
	dio_data->ref_count++;
	return 0;
}

//================================================================
// Entry for close
//================================================================
int Cdio_close(void *device_ext)
{
	PCDIO_DATA	dio_data;
	short		i;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data = (PCDIO_DATA)device_ext;
	if (dio_data == NULL) {
		return -ENODEV;
	}
	//----------------------------------------
	// Minus the reference count
	//----------------------------------------
	dio_data->ref_count--;
	//----------------------------------------
	// Check the reference count
	//----------------------------------------
	if (dio_data->ref_count == 0) {
		//----------------------------------------
		// Check the process ID
		//----------------------------------------
		for (i=0; i<CDIO_MAX_PROCESS; i++) {
			//----------------------------------------
			// If it isn't initialized
			//----------------------------------------
			if (dio_data->proc_data[i].process_id != 0) {
				//----------------------------------------
				// Stop the timer
				//----------------------------------------
				Ccom_kill_timer(dio_data->proc_data[i].trg_data.timer_id);
			}
		}
		//----------------------------------------
		// If the BusMaster library is supported in the kernel
		//----------------------------------------
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
		//----------------------------------------
		// If it is DM board
		//----------------------------------------
		if(dio_data->bios_d.b_info->board_type == CDIOBI_BT_PCI_DM){
			//--------------------------------------------
			// Stop the transfer (both directions)
			//--------------------------------------------
			ApiBmStop(&dio_data->bm_board.Master, BM_DIR_IN | BM_DIR_OUT);
			//---------------------------------------------
			// Unlock the momery
			//---------------------------------------------
			ApiBmUnlockMem(&dio_data->bm_board.Master, BM_DIR_IN);
			ApiBmUnlockMem(&dio_data->bm_board.Master, BM_DIR_OUT);
			//----------------------------------------
			// Mask all the interrupts
			//----------------------------------------
			dio_data->bm_board.PioMask	= 0xffffffff;
		}
#endif
		//----------------------------------------
		// Mask all the interrupts
		//----------------------------------------
		for (i=0; i<CDIO_MAX_PROCESS; i++) {
			memset(&dio_data->proc_data[i].int_data.mask, 0xff, sizeof(dio_data->proc_data[i].int_data.mask));
		}
		//----------------------------------------
		// Mask the interrupt
		//----------------------------------------
		Cdiobi_set_all_int_mask(&dio_data->bios_d, CDIOBI_MASK_ALL, NULL);
	}
	return 0;
}

//================================================================
// Enfor for dispatch
//================================================================
int Cdio_dispatch(void *device_ext, unsigned int ctl_code, void *param)
{
	long	lret = 0;
	void	*param_copy;
	int		iret;
	
	//----------------------------------------
	// Branch process according to the control code
	//----------------------------------------
	switch (ctl_code) {
	case CDIO_IOC_CLEAR:
	//----------------------------------------
	// Kernel version 2.2.X - 2.4.X
	//----------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
		for (;;) {
			if (MOD_IN_USE) {
				MOD_DEC_USE_COUNT;
			} else {
				break;
			}
		}
		MOD_INC_USE_COUNT;
	//----------------------------------------
	// Kernel version 2.6.X and 3.10.X
	//----------------------------------------
#elif ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
		for (;;) {
			if (module_refcount(THIS_MODULE)) {
				module_put(THIS_MODULE);
			} else {
				break;
			}
		}
		try_module_get(THIS_MODULE);
#endif
		break;
	case CDIO_IOC_INIT:			// Initialization function
		param_copy	= kmalloc(sizeof(CDIO_INIT), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_INIT)param, sizeof(CDIO_INIT));
		lret		= Cdio_init(device_ext, (PCDIO_INIT)param_copy);
		iret		= copy_to_user((PCDIO_INIT)param, param_copy, sizeof(CDIO_INIT));
		kfree(param_copy);
		break;
	case CDIO_IOC_EXIT:			// Exit function
		param_copy	= kmalloc(sizeof(CDIO_EXIT), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_EXIT)param, sizeof(CDIO_EXIT));
		lret		= Cdio_exit(device_ext, (PCDIO_EXIT)param_copy);
		iret		= copy_to_user((PCDIO_EXIT)param, param_copy, sizeof(CDIO_EXIT));
		kfree(param_copy);
		break;
	case CDIO_IOC_RESET:		// Reset function
		param_copy	= kmalloc(sizeof(CDIO_RESET), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_RESET)param, sizeof(CDIO_RESET));
		lret		= Cdio_reset(device_ext, (PCDIO_RESET)param_copy);
		iret		= copy_to_user((PCDIO_RESET)param, param_copy, sizeof(CDIO_RESET));
		kfree(param_copy);
		break;
	case CDIO_IOC_DFILTER:		// Function for setting digital filter
		param_copy	= kmalloc(sizeof(CDIO_DFILTER), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DFILTER)param, sizeof(CDIO_DFILTER));
		lret		= Cdio_set_digital_filter(device_ext, (PCDIO_DFILTER)param_copy);
		iret		= copy_to_user((PCDIO_DFILTER)param, param_copy, sizeof(CDIO_DFILTER));
		kfree(param_copy);
		break;
	case CDIO_IOC_INOUT:		// Input/Output function
		param_copy	= kmalloc(sizeof(CDIO_INOUT), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_INOUT)param, sizeof(CDIO_INOUT));
		lret		= Cdio_inout(device_ext, (PCDIO_INOUT)param_copy);
		iret		= copy_to_user((PCDIO_INOUT)param, param_copy, sizeof(CDIO_INOUT));
		kfree(param_copy);
		break;
	case CDIO_IOC_SET_INT:		// Function for setting interrupt factor
		param_copy	= kmalloc(sizeof(CDIO_SET_INT), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_SET_INT)param, sizeof(CDIO_SET_INT));
		lret		= Cdio_set_interrupt_event(device_ext, (PCDIO_SET_INT)param_copy);
		iret		= copy_to_user((PCDIO_SET_INT)param, param_copy, sizeof(CDIO_SET_INT));
		kfree(param_copy);
		break;
	case CDIO_IOC_SET_TRG:		// Function for setting trigger factor
		param_copy	= kmalloc(sizeof(CDIO_SET_TRG), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_SET_TRG)param, sizeof(CDIO_SET_TRG));
		lret		= Cdio_set_trigger_event(device_ext, (PCDIO_SET_TRG)param_copy);
		iret		= copy_to_user((PCDIO_SET_TRG)param, param_copy, sizeof(CDIO_SET_TRG));
		kfree(param_copy);
		break;
	case CDIO_IOC_GET_INFO:		// Function for retrieving device information
		param_copy	= kmalloc(sizeof(CDIO_GET_INFO), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_GET_INFO)param, sizeof(CDIO_GET_INFO));
		lret		= Cdio_get_device_info(device_ext, (PCDIO_GET_INFO)param_copy);
		iret		= copy_to_user((PCDIO_GET_INFO)param, param_copy, sizeof(CDIO_GET_INFO));
		kfree(param_copy);
		break;
	case CDIO_IOC_TH_WAIT:		// Wait in thread
		param_copy	= kmalloc(sizeof(CDIO_TH_WAIT), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_TH_WAIT)param, sizeof(CDIO_TH_WAIT));
		lret		= Cdio_wait_event(device_ext, (PCDIO_TH_WAIT)param_copy);
		iret		= copy_to_user((PCDIO_TH_WAIT)param, param_copy, sizeof(CDIO_TH_WAIT));
		kfree(param_copy);
		break;
	case CDIO_IOC_TH_WAKE_UP:	// Thread wake-up
		param_copy	= kmalloc(sizeof(CDIO_TH_WAKE_UP), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_TH_WAKE_UP)param, sizeof(CDIO_TH_WAKE_UP));
		lret		= Cdio_wake_up_event(device_ext, (PCDIO_TH_WAKE_UP)param_copy);
		iret		= copy_to_user((PCDIO_TH_WAKE_UP)param, param_copy, sizeof(CDIO_TH_WAKE_UP));
		kfree(param_copy);
		break;
	case CDIO_IOC_TH_GET_INT:	// Retrieve interrupt factor in thread
		param_copy	= kmalloc(sizeof(CDIO_TH_GET_INT), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_TH_GET_INT)param, sizeof(CDIO_TH_GET_INT));
		lret		= Cdio_get_interrupt_factor(device_ext, (PCDIO_TH_GET_INT)param_copy);
		iret		= copy_to_user((PCDIO_TH_GET_INT)param, param_copy, sizeof(CDIO_TH_GET_INT));
		kfree(param_copy);
		break;
	case CDIO_IOC_TH_GET_TRG:	// Retrieve trigger factor in thread
		param_copy	= kmalloc(sizeof(CDIO_TH_GET_TRG), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_TH_GET_TRG)param, sizeof(CDIO_TH_GET_TRG));
		lret		= Cdio_get_trigger_factor(device_ext, (PCDIO_TH_GET_TRG)param_copy);
		iret		= copy_to_user((PCDIO_TH_GET_TRG)param, param_copy, sizeof(CDIO_TH_GET_TRG));
		kfree(param_copy);
		break;
	case CDIO_IOC_SET_8255MODE:	// Set the 8255 mode
		param_copy	= kmalloc(sizeof(CDIO_SET_8255MODE), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_SET_8255MODE)param, sizeof(CDIO_SET_8255MODE));
		lret		= Cdio_set_8255_mode(device_ext, (PCDIO_SET_8255MODE)param_copy);
		iret		= copy_to_user((PCDIO_SET_8255MODE)param, param_copy, sizeof(CDIO_SET_8255MODE));
		kfree(param_copy);
		break;
	case CDIO_IOC_GET_8255MODE:	// Retrieve the 8255 mode setting
		param_copy	= kmalloc(sizeof(CDIO_GET_8255MODE), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_GET_8255MODE)param, sizeof(CDIO_GET_8255MODE));
		lret		= Cdio_get_8255_mode(device_ext, (PCDIO_GET_8255MODE)param_copy);
		iret		= copy_to_user((PCDIO_GET_8255MODE)param, param_copy, sizeof(CDIO_GET_8255MODE));
		kfree(param_copy);
		break;

	//----------------------------------------
	// Dispatch for DM
	//----------------------------------------
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
	case CDIO_IOC_DM_SET_DIR:					// I/O direction setting
		param_copy	= kmalloc(sizeof(CDIO_DM_SET_DIR), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DM_SET_DIR)param, sizeof(CDIO_DM_SET_DIR));
		lret		= Cdio_dm_set_dir(device_ext, (PCDIO_DM_SET_DIR)param_copy);
		iret		= copy_to_user((PCDIO_DM_SET_DIR)param, param_copy, sizeof(CDIO_DM_SET_DIR));
		kfree(param_copy);
		break;
	case CDIO_IOC_DM_SET_STAND_ALONE:			// Stand alone setting
		param_copy	= kmalloc(sizeof(CDIO_DM_SET_STAND_ALONE), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DM_SET_STAND_ALONE)param, sizeof(CDIO_DM_SET_STAND_ALONE));
		lret		= Cdio_dm_set_stand_alone(device_ext, (PCDIO_DM_SET_STAND_ALONE)param_copy);
		iret		= copy_to_user((PCDIO_DM_SET_STAND_ALONE)param, param_copy, sizeof(CDIO_DM_SET_STAND_ALONE));
		kfree(param_copy);
		break;
	case CDIO_IOC_DM_SET_MASTER:				// Master setting
		param_copy	= kmalloc(sizeof(CDIO_DM_SET_MASTER), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DM_SET_MASTER)param, sizeof(CDIO_DM_SET_MASTER));
		lret		= Cdio_dm_set_master(device_ext, (PCDIO_DM_SET_MASTER)param_copy);
		iret		= copy_to_user((PCDIO_DM_SET_MASTER)param, param_copy, sizeof(CDIO_DM_SET_MASTER));
		kfree(param_copy);
		break;
	case CDIO_IOC_DM_SET_SLAVE:					// Slave setting
		param_copy	= kmalloc(sizeof(CDIO_DM_SET_SLAVE), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DM_SET_SLAVE)param, sizeof(CDIO_DM_SET_SLAVE));
		lret		= Cdio_dm_set_slave(device_ext, (PCDIO_DM_SET_SLAVE)param_copy);
		iret		= copy_to_user((PCDIO_DM_SET_SLAVE)param, param_copy, sizeof(CDIO_DM_SET_SLAVE));
		kfree(param_copy);
		break;
	case CDIO_IOC_DM_SET_START_TRG:				// Start condition setting
		param_copy	= kmalloc(sizeof(CDIO_DM_SET_START_TRG), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DM_SET_START_TRG)param, sizeof(CDIO_DM_SET_START_TRG));
		lret		= Cdio_dm_set_start_trg(device_ext, (PCDIO_DM_SET_START_TRG)param_copy);
		iret		= copy_to_user((PCDIO_DM_SET_START_TRG)param, param_copy, sizeof(CDIO_DM_SET_START_TRG));
		kfree(param_copy);
		break;
	case CDIO_IOC_DM_SET_START_PTN:				// Pattern matching condition of start condition setting
		param_copy	= kmalloc(sizeof(CDIO_DM_SET_START_PTN), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DM_SET_START_PTN)param, sizeof(CDIO_DM_SET_START_PTN));
		lret		= Cdio_dm_set_start_ptn(device_ext, (PCDIO_DM_SET_START_PTN)param_copy);
		iret		= copy_to_user((PCDIO_DM_SET_START_PTN)param, param_copy, sizeof(CDIO_DM_SET_START_PTN));
		kfree(param_copy);
		break;
	case CDIO_IOC_DM_SET_CLOCK_TRG:				// Clock condition setting
		param_copy	= kmalloc(sizeof(CDIO_DM_SET_CLOCK_TRG), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DM_SET_CLOCK_TRG)param, sizeof(CDIO_DM_SET_CLOCK_TRG));
		lret		= Cdio_dm_set_clock_trg(device_ext, (PCDIO_DM_SET_CLOCK_TRG)param_copy);
		iret		= copy_to_user((PCDIO_DM_SET_CLOCK_TRG)param, param_copy, sizeof(CDIO_DM_SET_CLOCK_TRG));
		kfree(param_copy);
		break;
	case CDIO_IOC_DM_SET_INTERNAL_CLOCK:		// Internal clock setting
		param_copy	= kmalloc(sizeof(CDIO_DM_SET_INTERNAL_CLOCK), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DM_SET_INTERNAL_CLOCK)param, sizeof(CDIO_DM_SET_INTERNAL_CLOCK));
		lret		= Cdio_dm_set_internal_clock(device_ext, (PCDIO_DM_SET_INTERNAL_CLOCK)param_copy);
		iret		= copy_to_user((PCDIO_DM_SET_INTERNAL_CLOCK)param, param_copy, sizeof(CDIO_DM_SET_INTERNAL_CLOCK));
		kfree(param_copy);
		break;
	case CDIO_IOC_DM_SET_STOP_TRG:				// Stop condition setting
		param_copy	= kmalloc(sizeof(CDIO_DM_SET_STOP_TRG), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DM_SET_STOP_TRG)param, sizeof(CDIO_DM_SET_STOP_TRG));
		lret		= Cdio_dm_set_stop_trg(device_ext, (PCDIO_DM_SET_STOP_TRG)param_copy);
		iret		= copy_to_user((PCDIO_DM_SET_STOP_TRG)param, param_copy, sizeof(CDIO_DM_SET_STOP_TRG));
		kfree(param_copy);
		break;
	case CDIO_IOC_DM_SET_STOP_NUM:				// Stop number setting
		param_copy	= kmalloc(sizeof(CDIO_DM_SET_STOP_NUM), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DM_SET_STOP_NUM)param, sizeof(CDIO_DM_SET_STOP_NUM));
		lret		= Cdio_dm_set_stop_num(device_ext, (PCDIO_DM_SET_STOP_NUM)param_copy);
		iret		= copy_to_user((PCDIO_DM_SET_STOP_NUM)param, param_copy, sizeof(CDIO_DM_SET_STOP_NUM));
		kfree(param_copy);
		break;
	case CDIO_IOC_DM_RESET:						// Reset the FIFO
		param_copy	= kmalloc(sizeof(CDIO_DM_RESET), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DM_RESET)param, sizeof(CDIO_DM_RESET));
		lret		= Cdio_dm_reset(device_ext, (PCDIO_DM_RESET)param_copy);
		iret		= copy_to_user((PCDIO_DM_RESET)param, param_copy, sizeof(CDIO_DM_RESET));
		kfree(param_copy);
		break;
	case CDIO_IOC_DM_SET_BUF:					// Buffer setting
		param_copy	= kmalloc(sizeof(CDIO_DM_SET_BUF), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DM_RESET)param, sizeof(CDIO_DM_SET_BUF));
		lret		= Cdio_dm_set_buf(device_ext, (PCDIO_DM_SET_BUF)param_copy);
		iret		= copy_to_user((PCDIO_DM_SET_BUF)param, param_copy, sizeof(CDIO_DM_SET_BUF));
		kfree(param_copy);
		break;
	case CDIO_IOC_DM_START:						// Sampling/Generating start
		param_copy	= kmalloc(sizeof(CDIO_DM_START), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DM_START)param, sizeof(CDIO_DM_START));
		lret		= Cdio_dm_start(device_ext, (PCDIO_DM_START)param_copy);
		iret		= copy_to_user((PCDIO_DM_START)param, param_copy, sizeof(CDIO_DM_START));
		kfree(param_copy);
		break;
	case CDIO_IOC_DM_STOP:						// Sampling/Generating stop
		param_copy	= kmalloc(sizeof(CDIO_DM_STOP), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DM_STOP)param, sizeof(CDIO_DM_STOP));
		lret		= Cdio_dm_stop(device_ext, (PCDIO_DM_STOP)param_copy);
		iret		= copy_to_user((PCDIO_DM_STOP)param, param_copy, sizeof(CDIO_DM_STOP));
		kfree(param_copy);
		break;
	case CDIO_IOC_DM_GET_STATUS:				// Retrieve the transfer status
		param_copy	= kmalloc(sizeof(CDIO_DM_GET_STATUS), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DM_GET_STATUS)param, sizeof(CDIO_DM_GET_STATUS));
		lret		= Cdio_dm_get_status(device_ext, (PCDIO_DM_GET_STATUS)param_copy);
		iret		= copy_to_user((PCDIO_DM_GET_STATUS)param, param_copy, sizeof(CDIO_DM_GET_STATUS));
		kfree(param_copy);
		break;
	case CDIO_IOC_DM_GET_COUNT:					// Retrieve the transfer count
		param_copy	= kmalloc(sizeof(CDIO_DM_GET_COUNT), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DM_GET_COUNT)param, sizeof(CDIO_DM_GET_COUNT));
		lret		= Cdio_dm_get_count(device_ext, (PCDIO_DM_GET_COUNT)param_copy);
		iret		= copy_to_user((PCDIO_DM_GET_COUNT)param, param_copy, sizeof(CDIO_DM_GET_COUNT));
		kfree(param_copy);
		break;
	case CDIO_IOC_DM_GET_WRITEPOINTER_USER_BUF:	// Retrieve the transfer position on user buffer
		param_copy	= kmalloc(sizeof(CDIO_DM_GET_WRITEPOINTER_USER_BUF), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DM_GET_WRITEPOINTER_USER_BUF)param, sizeof(CDIO_DM_GET_WRITEPOINTER_USER_BUF));
		lret		= Cdio_dm_get_write_pointer_user_buf(device_ext, (PCDIO_DM_GET_WRITEPOINTER_USER_BUF)param_copy);
		iret		= copy_to_user((PCDIO_DM_GET_WRITEPOINTER_USER_BUF)param, param_copy, sizeof(CDIO_DM_GET_WRITEPOINTER_USER_BUF));
		kfree(param_copy);
		break;
	case CDIO_IOC_DM_GET_FIFO_COUNT:			// Retrieve FIFO count
		param_copy	= kmalloc(sizeof(CDIO_DM_GET_FIFO_COUNT), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DM_GET_FIFO_COUNT)param, sizeof(CDIO_DM_GET_FIFO_COUNT));
		lret		= Cdio_dm_get_fifo_count(device_ext, (PCDIO_DM_GET_FIFO_COUNT)param_copy);
		iret		= copy_to_user((PCDIO_DM_GET_FIFO_COUNT)param, param_copy, sizeof(CDIO_DM_GET_FIFO_COUNT));
		kfree(param_copy);
		break;
	case CDIO_IOC_DM_GET_INT_COUNT:				// Retrieve BusMaster interrupt count
		param_copy	= kmalloc(sizeof(CDIO_DM_GET_INT_COUNT), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DM_GET_INT_COUNT)param, sizeof(CDIO_DM_GET_INT_COUNT));
		lret		= Cdio_dm_get_int_count(device_ext, (PCDIO_DM_GET_INT_COUNT)param_copy);
		iret		= copy_to_user((PCDIO_DM_GET_INT_COUNT)param, param_copy, sizeof(CDIO_DM_GET_INT_COUNT));
		kfree(param_copy);
		break;
	case CDIO_IOC_DM_TH_WAIT:					// DM thread sleep function
		param_copy	= kmalloc(sizeof(CDIO_DM_TH_WAIT), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DM_TH_WAIT)param, sizeof(CDIO_DM_TH_WAIT));
		lret		= Cdio_dm_wait_event(device_ext, (PCDIO_DM_TH_WAIT)param_copy);
		iret		= copy_to_user((PCDIO_DM_TH_WAIT)param, param_copy, sizeof(CDIO_DM_TH_WAIT));
		kfree(param_copy);
		break;
	case CDIO_IOC_DM_TH_WAKE_UP:				// DM thread wake-up function
		param_copy	= kmalloc(sizeof(CDIO_DM_TH_WAKE_UP), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DM_TH_WAKE_UP)param, sizeof(CDIO_DM_TH_WAKE_UP));
		lret		= Cdio_dm_wake_up_event(device_ext, (PCDIO_DM_TH_WAKE_UP)param_copy);
		iret		= copy_to_user((PCDIO_DM_TH_WAKE_UP)param, param_copy, sizeof(CDIO_DM_TH_WAKE_UP));
		kfree(param_copy);
		break;
	case CDIO_IOC_DM_SET_COUNT_EVENT:			// Notification of the transfer completion with specified number setting
		param_copy	= kmalloc(sizeof(CDIO_DM_SET_COUNT_EVENT), GFP_KERNEL);
		iret		= copy_from_user(param_copy, (PCDIO_DM_SET_COUNT_EVENT)param, sizeof(CDIO_DM_SET_COUNT_EVENT));
		lret		= Cdio_dm_set_count_event(device_ext, (PCDIO_DM_SET_COUNT_EVENT)param_copy);
		iret		= copy_to_user((PCDIO_DM_SET_COUNT_EVENT)param, param_copy, sizeof(CDIO_DM_SET_COUNT_EVENT));
		kfree(param_copy);
		break;
#endif
	default:
		return -EINVAL;
		break;
	}
	return (int)lret;
}

//================================================================
// Each function 
//================================================================
//================================================================
// Initialization function
//================================================================
long Cdio_init(void *device_ext, PCDIO_INIT param)
{
	PCDIO_DATA		dio_data;
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
	unsigned long	flags;
#endif

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data	= (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the global error
	//----------------------------------------
	if (dio_data->g_ret != DIO_ERR_SUCCESS) {
		param->ret	= dio_data->g_ret;
		return 0;
	}
	//----------------------------------------
	// If BoardType is BusMaster and the kernel is not a 2.4 or higher kernel,
	// return an error that it is unsupported kernel
	//----------------------------------------
#if ((!(LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0))) &&\
	    (!(LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0))))
	if(dio_data->bios_d.b_info->board_type == CDIOBI_BT_PCI_DM){
		param->ret	= DIO_ERR_SYS_NOT_SUPPORT_KERNEL;
		return 0;
	}
#else
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//----------------------------------------
	// If the BoardType is BusMaster board
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type == CDIOBI_BT_PCI_DM){
		//----------------------------------------
		// Only one process can be used
		//----------------------------------------
		if(dio_data->ref_count > 1){
			param->ret	= DIO_ERR_SYS_USING_OTHER_PROCESS;
			//----------------------------------------
			// Release the spinlock
			//----------------------------------------
			spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
			return 0;
		}
		dio_data->bm_board.PioMask	= 0xffffffff;
		outl(dio_data->bm_board.PioMask, dio_data->bm_board.dwAddr + 0x10);
		//---------------------------------------------------------
		// Set the initial values of sampling and generating
		//---------------------------------------------------------
		dio_data->bm_board.Sampling.Start			= 0x00000004;		//DIODM_START_SOFT
		dio_data->bm_board.Sampling.Clock			= 0x00040000;		//DIODM_CLK_CLOCK
		dio_data->bm_board.Sampling.Stop			= 0x00000400;		//DIODM_STOP_SOFT
		dio_data->bm_board.Sampling.Pattern			= 0x00000000;
		dio_data->bm_board.Sampling.Mask			= 0x00000000;
		dio_data->bm_board.Sampling.InternalClock	= 40;
		dio_data->bm_board.Sampling.StopNum			= 1000;

		dio_data->bm_board.Generating.Start			= 0x00000004;		//DIODM_START_SOFT
		dio_data->bm_board.Generating.Clock			= 0x00040000;		//DIODM_CLK_CLOCK
		dio_data->bm_board.Generating.Stop			= 0x00000400;		//DIODM_STOP_SOFT
		dio_data->bm_board.Generating.InternalClock	= 40;
		dio_data->bm_board.Generating.StopNum		= 1000;
		//----------------------------------------
		// Reset all
		//----------------------------------------
		ApiBmReset(&dio_data->bm_board.Master, BM_RESET_ALL);
		outl(0xffffffff, dio_data->bm_board.dwAddr + 0x14);				// Clear the interrupt
	}
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
#endif
	//----------------------------------------
	// Set the return value
	//----------------------------------------
	param->ret	= DIO_ERR_SUCCESS;
	return 0;
}

//================================================================
// Exit function
//================================================================
long Cdio_exit(void *device_ext, PCDIO_EXIT param)
{
	PCDIO_DATA			dio_data;
	short				i;
	unsigned short		timer_id;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
	wait_queue_head_t	wq;			// Wait queue (2,4,0)
#else
	struct wait_queue	*wq;		// Wait queue (before (2,4,0))
#endif
	unsigned long		flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data	= (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//----------------------------------------
	// Cheak the process ID
	//----------------------------------------
	for (i=0; i<CDIO_MAX_PROCESS; i++) {
		//----------------------------------------
		// If the process is in use
		//----------------------------------------
		if (dio_data->proc_data[i].process_id == param->process_id) {
			//----------------------------------------
			// Stop the timer
			//----------------------------------------
			Ccom_kill_timer(dio_data->proc_data[i].trg_data.timer_id);
			//----------------------------------------
			// Shelter the timer ID
			//----------------------------------------
			timer_id = dio_data->proc_data[i].trg_data.timer_id;
			//----------------------------------------
			// Shelter the wait structure
			//----------------------------------------
			wq = dio_data->proc_data[i].wait_obj.wq;
			//----------------------------------------
			// Initialize the work
			//----------------------------------------
			memset(&dio_data->proc_data[i], 0, sizeof(dio_data->proc_data[i]));	
			//----------------------------------------
			// Return the wait structure
			//----------------------------------------
			dio_data->proc_data[i].wait_obj.wq = wq;
			//----------------------------------------
			// Return the timer ID
			//----------------------------------------
			dio_data->proc_data[i].trg_data.timer_id = timer_id;
		}
	}
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
	//----------------------------------------
	// Set the return value
	//----------------------------------------
	param->ret	= DIO_ERR_SUCCESS;
	return 0;
}

//================================================================
// Resest function
//================================================================
long Cdio_reset(void *device_ext, PCDIO_RESET param)
{
	PCDIO_DATA		dio_data;
	short			out_port_num;
	short			i;
	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data	= (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//----------------------------------------
	// Set all output ports to 0
	//----------------------------------------
	out_port_num	= 0;
	Cdiobi_get_info(dio_data->board_name, IDIO_NUMBER_OF_DO_PORT, (void *)&out_port_num, NULL, NULL);
	for (i=0; i<out_port_num; i++) {
		Cdiobi_output_port(&dio_data->bios_d, i, 0);
	}
	//----------------------------------------
	// Mask the interrupt
	//----------------------------------------
	Cdiobi_set_all_int_mask(&dio_data->bios_d, CDIOBI_MASK_ALL, NULL);
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
	//----------------------------------------
	// Set the return value
	//----------------------------------------
	param->ret = DIO_ERR_SUCCESS;
	return 0;
}

//================================================================
// Function for setting digital filter
//================================================================
long Cdio_set_digital_filter(void *device_ext, PCDIO_DFILTER param)
{
	PCDIO_DATA		dio_data;
	unsigned long	flags;
	long			lret;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data	= (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//----------------------------------------
	// Call the BIOS
	//----------------------------------------
	lret = Cdiobi_set_digital_filter(&dio_data->bios_d, param->filter_value);
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
	//----------------------------------------
	// Converse the error
	//----------------------------------------
	switch (lret) {
	case 0:		lret = DIO_ERR_SUCCESS;				break;
	case -1:	lret = DIO_ERR_SYS_NOT_SUPPORTED;	break;
	case -2:	lret = DIO_ERR_SYS_FILTER;			break;
	default:	lret = DIO_ERR_SYS_NOT_SUPPORTED;	break;
	}
	param->ret	= lret;
	return 0;
}

//================================================================
// Function for setting 8255 mode
//================================================================
long Cdio_set_8255_mode(void *device_ext, PCDIO_SET_8255MODE param)
{
	PCDIO_DATA			dio_data;
	unsigned long		flags;
	long				lret;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data	= (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//----------------------------------------
	// Call the BIOS
	//----------------------------------------
	lret	= Cdiobi_set_8255_mode(&dio_data->bios_d, param->chip_no, param->ctrl_word);
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
	//----------------------------------------
	// Converse the error
	//----------------------------------------
	switch (lret) {
	case 0:		lret = DIO_ERR_SUCCESS;				break;
	case -1:	lret = DIO_ERR_SYS_NOT_SUPPORTED;	break;
	case -2:	lret = DIO_ERR_SYS_8255;			break;
	default:	lret = DIO_ERR_SYS_NOT_SUPPORTED;	break;
	}
	param->ret	= lret;
	return 0;
}

//================================================================
// Function for retrieving the 8255 mode settting
//================================================================
long Cdio_get_8255_mode(void *device_ext, PCDIO_GET_8255MODE param)
{
	PCDIO_DATA			dio_data;
	unsigned long		flags;
	long				lret;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data	= (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//----------------------------------------
	// Call the BIOS
	//----------------------------------------
	lret	= Cdiobi_get_8255_mode(&dio_data->bios_d, param->chip_no, param->ctrl_word);
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
	//----------------------------------------
	// Converse the error
	//----------------------------------------
	switch (lret) {
	case 0:		lret = DIO_ERR_SUCCESS;				break;
	case -1:	lret = DIO_ERR_SYS_NOT_SUPPORTED;	break;
	case -2:	lret = DIO_ERR_SYS_8255;			break;
	default:	lret = DIO_ERR_SYS_NOT_SUPPORTED;	break;
	}
	param->ret	= lret;
	return 0;
}

//================================================================
// Input/Output function
//================================================================
long Cdio_inout(void *device_ext, PCDIO_INOUT param)
{
	PCDIO_DATA		dio_data;
	short			i;
	long			lret = DIO_ERR_SUCCESS;
	long			can_accs;
	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data	= (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//----------------------------------------
	// Branch process according to the type of command
	//----------------------------------------
	switch (param->cmd) {
	case CDIO_CMD_INP_PORT:
		//----------------------------------------
		// Cheak the number
		//----------------------------------------
		for (i=0; i<param->num; i++) {
			can_accs	= Cdiobi_check_inp_port(&dio_data->bios_d, param->port[i]);
			if (can_accs == 0) {
				lret	= DIO_ERR_SYS_PORT_NO;
			}
		}
		if (lret != DIO_ERR_SUCCESS) {
			break;
		}
		//----------------------------------------
		// Input
		//----------------------------------------
		for (i=0; i<param->num; i++) {
			Cdiobi_input_port(&dio_data->bios_d, param->port[i], &param->data[i]);
		}
		break;
	case CDIO_CMD_INP_BIT:
		//----------------------------------------
		// Cheak the number
		//----------------------------------------
		for (i=0; i<param->num; i++) {
			can_accs = Cdiobi_check_inp_bit(&dio_data->bios_d, param->port[i]);
			if (can_accs == 0) {
				lret	= DIO_ERR_SYS_BIT_NO;
			}
		}
		if (lret != DIO_ERR_SUCCESS) {
			break;
		}
		//----------------------------------------
		// Input
		//----------------------------------------
		for (i=0; i<param->num; i++) {
			Cdiobi_input_bit(&dio_data->bios_d, param->port[i], &param->data[i]);
		}
		break;
	case CDIO_CMD_OUT_PORT:
		//----------------------------------------
		// Cheak the number
		//----------------------------------------
		for (i=0; i<param->num; i++) {
			can_accs	= Cdiobi_check_out_port(&dio_data->bios_d, param->port[i]);
			if (can_accs == 0) {
				lret	= DIO_ERR_SYS_PORT_NO;
			}
		}
		if (lret != DIO_ERR_SUCCESS) {
			break;
		}
		//----------------------------------------
		// Output
		//----------------------------------------
		for (i=0; i<param->num; i++) {
			Cdiobi_output_port(&dio_data->bios_d, param->port[i], param->data[i]);
		}
		break;
	case CDIO_CMD_OUT_BIT:
		//----------------------------------------
		// Cheak the number
		//----------------------------------------
		for (i=0; i<param->num; i++) {
			can_accs	= Cdiobi_check_out_bit(&dio_data->bios_d, param->port[i]);
			if (can_accs == 0) {
				lret	= DIO_ERR_SYS_BIT_NO;
			}
			if (param->data[i] != 0 && param->data[i] != 1) {
				lret	= DIO_ERR_SYS_BIT_DATA;
			}
		}
		if (lret != DIO_ERR_SUCCESS) {
			break;
		}
		//----------------------------------------
		// Output
		//----------------------------------------
		for (i=0; i<param->num; i++) {
			Cdiobi_output_bit(&dio_data->bios_d, param->port[i], param->data[i]);
		}
		break;
	case CDIO_CMD_ECHO_PORT:
		//----------------------------------------
		// Cheak the number
		//----------------------------------------
		for (i=0; i<param->num; i++) {
			can_accs	= Cdiobi_check_out_port(&dio_data->bios_d, param->port[i]);
			if (can_accs == 0) {
				lret	= DIO_ERR_SYS_PORT_NO;
			}
		}
		if (lret != DIO_ERR_SUCCESS) {
			break;
		}
		//----------------------------------------
		// Echo back
		//----------------------------------------
		for (i=0; i<param->num; i++) {
			Cdiobi_echo_port(&dio_data->bios_d, param->port[i], &param->data[i]);
		}
		break;
	case CDIO_CMD_ECHO_BIT:
		//----------------------------------------
		// Cheak the number
		//----------------------------------------
		for (i=0; i<param->num; i++) {
			can_accs	= Cdiobi_check_out_bit(&dio_data->bios_d, param->port[i]);
			if (can_accs == 0) {
				lret	= DIO_ERR_SYS_BIT_NO;
			}
		}
		if (lret != DIO_ERR_SUCCESS) {
			break;
		}
		//----------------------------------------
		// Echo back
		//----------------------------------------
		for (i=0; i<param->num; i++) {
			Cdiobi_echo_bit(&dio_data->bios_d, param->port[i], &param->data[i]);
		}
		break;
	}
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
	//----------------------------------------
	// Set the return value
	//----------------------------------------
	param->ret	= lret;
	return 0;
}

//================================================================
// Subroutine : Mask OK or NOT
//================================================================
long Cdio_is_mask_ok(PCDIO_DATA dio_data, short set_bit_no)
{
	short	i;
	short	bit_no;
	short	port_no;

	port_no	= set_bit_no / 8;
	bit_no	= set_bit_no % 8;
	//----------------------------------------
	// For each process
	//----------------------------------------
	for (i=0; i<CDIO_MAX_PROCESS; i++) {
		//----------------------------------------
		// Ignore it if the process is invalid
		//----------------------------------------
		if (dio_data->proc_data[i].process_id == 0) {
			continue;
		}
		//----------------------------------------
		// NG if the mask is empty in any the process
		//----------------------------------------
		if ((dio_data->proc_data[i].int_data.mask[port_no] & (0x01 << bit_no)) == 0) {
			return 0;
		}
	}
	return 1;
}

//================================================================
// Function for setting interrupt factor
//================================================================
long Cdio_set_interrupt_event(void *device_ext, PCDIO_SET_INT param)
{
	long			lret = DIO_ERR_SUCCESS;
	PCDIO_DATA		dio_data;
	long			bit_chk;
	short			i;
	PCDIO_INT_DATA	int_data;
	unsigned char	sence[CDIOBI_INT_SENCE_SIZE];
	short			bit_no;
	short			port_no;
	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data	= (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the interrupt available bits
	//----------------------------------------
	bit_chk	= Cdiobi_check_int_bit(&dio_data->bios_d, param->bit_no);
	if (bit_chk == 0) {
		param->ret	= DIO_ERR_SYS_INT_BIT;
		return 0;
	}
	//----------------------------------------
	// Check the logic
	//----------------------------------------
	if (param->logic != DIO_INT_NONE &&
		param->logic != DIO_INT_RISE &&
		param->logic != DIO_INT_FALL) {
		param->ret	= DIO_ERR_SYS_INT_LOGIC;
		return 0;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//----------------------------------------
	// If the BusMaster library is supported in the kernel
	//----------------------------------------
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
	//----------------------------------------
	// If it is DM board
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type == CDIOBI_BT_PCI_DM){
		//----------------------------------------
		// Return procedure error of execution if bus master function is in use
		//----------------------------------------
		if ((dio_data->bm_board.PioMask & 0xffff0fff) != 0xffff0fff) {
			param->ret	= DIO_ERR_DM_SEQUENCE;
			//----------------------------------------
			// Release the spinlock
			//----------------------------------------
			spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
			return 0;
		}
	}
#endif
	//----------------------------------------
	// Check the process structure
	//----------------------------------------
	int_data	= NULL;
	for (i=0; i<CDIO_MAX_PROCESS; i++) {
		//----------------------------------------
		// Store the address if the process is in use
		//----------------------------------------
		if (dio_data->proc_data[i].process_id == param->process_id) {
			int_data	= &dio_data->proc_data[i].int_data;
			break;
		}
	}
	//----------------------------------------
	// Determine the process structure if it has not been determined
	//----------------------------------------
	if (int_data == NULL) {
		for (i=0; i<CDIO_MAX_PROCESS; i++) {
			//----------------------------------------
			// Store the address if the process is in use
			//----------------------------------------
			if (dio_data->proc_data[i].process_id == 0) {
				dio_data->proc_data[i].process_id = param->process_id;
				int_data	= &dio_data->proc_data[i].int_data;
				break;
			}
		}
	}
	if (int_data == NULL) {
		param->ret	= DIO_ERR_SYS_USING_OTHER_PROCESS;
		//----------------------------------------
		// Release the spinlock
		//----------------------------------------
		spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
		return 0;
	}
	//----------------------------------------
	// Set the maximum number of bits
	//----------------------------------------
	if (int_data->max_bit == 0 ||
		int_data->max_bit < (param->bit_no + 1)) {
		int_data->max_bit	= param->bit_no + 1;
	}
	//----------------------------------------
	// Set the interrupt logic
	//----------------------------------------
	if (param->logic == DIO_INT_RISE ||
		param->logic == DIO_INT_FALL) {
		lret	= Cdiobi_set_int_logic(&dio_data->bios_d, param->bit_no, param->logic);
		if(lret == -2){
			param->ret = DIO_ERR_SYS_INT_LOGIC;	// Interrupt logic is outside of the available range.
			//----------------------------------------
			// Release the spinlock
			//----------------------------------------
			spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
			return 0;
		}
	}
	//----------------------------------------
	// Input the interrupt status
	//----------------------------------------
	Cdiobi_sence_int_status(&dio_data->bios_d, sence);
	//----------------------------------------
	// Set the interrupt mask
	//----------------------------------------
	if (param->logic == DIO_INT_NONE) {
		port_no	= param->bit_no / 8;
		bit_no	= param->bit_no % 8;
		int_data->mask[port_no] |= (0x01 << bit_no);
		if (Cdio_is_mask_ok(dio_data, param->bit_no)) {
			Cdiobi_set_bit_int_mask(&dio_data->bios_d, param->bit_no, CDIOBI_INT_MASK_CLOSE);
		}
	} else {
		port_no	= param->bit_no / 8;
		bit_no	= param->bit_no % 8;
		int_data->mask[port_no] &= ~(0x01 << bit_no);
		Cdiobi_set_bit_int_mask(&dio_data->bios_d, param->bit_no, CDIOBI_INT_MASK_OPEN);
	}
	//----------------------------------------
	// If the BusMaster library is supported in the kernel
	//----------------------------------------
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
	//----------------------------------------
	// If it is DM board
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type == CDIOBI_BT_PCI_DM){
		// Interrupt mask (Also record in the DM structure)
		dio_data->bm_board.PioMask = 0xffff0fff | (((unsigned short)dio_data->bios_d.int_mask[0] << 12) & 0x0000f000);
	}
#endif
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
	//----------------------------------------
	// Set the return value
	//----------------------------------------
	param->ret	= DIO_ERR_SUCCESS;
	return 0;
}

//================================================================
// Function for setting trigger factor
//================================================================
long Cdio_set_trigger_event(void *device_ext, PCDIO_SET_TRG param)
{
	PCDIO_DATA		dio_data;
	long			lret = DIO_ERR_SUCCESS;
	long			bit_chk;
	short			i;
	PCDIO_TRG_DATA	trg_data;
	unsigned long	tim_backup;
	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data = (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the trigger enable bit
	//----------------------------------------
	bit_chk	= Cdiobi_check_inp_bit(&dio_data->bios_d, param->bit_no);
	if (bit_chk == 0) {
		param->ret	= DIO_ERR_SYS_BIT_NO;
		return 0;
	}
	//----------------------------------------
	// Check the logic
	//----------------------------------------
	if (param->logic != DIO_TRG_NONE &&
		param->logic != DIO_TRG_RISE &&
		param->logic != DIO_TRG_FALL &&
		param->logic != (DIO_TRG_RISE | DIO_TRG_FALL)) {
		param->ret	= DIO_ERR_DLL_TRG_KIND;
		return 0;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//----------------------------------------
	// If the BusMaster library is supported in the kernel
	//----------------------------------------
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
	//----------------------------------------
	// If it is DM board
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type == CDIOBI_BT_PCI_DM){
		//----------------------------------------
		// Return procedure error of execution if bus master function is in use
		//----------------------------------------
		if ((dio_data->bm_board.PioMask & 0xffff0fff) != 0xffff0fff) {
			param->ret	= DIO_ERR_DM_SEQUENCE;
			//----------------------------------------
			// Release the spinlock
			//----------------------------------------
			spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
			return 0;
		}
	}
#endif
	//----------------------------------------
	// Check the process structure
	//----------------------------------------
	trg_data	= NULL;
	for (i=0; i<CDIO_MAX_PROCESS; i++) {
		//----------------------------------------
		// Store the address if the process is in use
		//----------------------------------------
		if (dio_data->proc_data[i].process_id == param->process_id) {
			trg_data	= &dio_data->proc_data[i].trg_data;
			break;
		}
	}
	//----------------------------------------
	// Determine the process structure if it has not been determined
	//----------------------------------------
	if (trg_data == NULL) {
		for (i=0; i<CDIO_MAX_PROCESS; i++) {
			//----------------------------------------
			// Store the address if the process is in use
			//----------------------------------------
			if (dio_data->proc_data[i].process_id == 0) {
				dio_data->proc_data[i].process_id = param->process_id;
				trg_data = &dio_data->proc_data[i].trg_data;
				break;
			}
		}
	}
	if (trg_data == NULL) {
		param->ret	= DIO_ERR_SYS_USING_OTHER_PROCESS;
		//----------------------------------------
		// Release the spinlock
		//----------------------------------------
		spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
		return 0;
	}
	//----------------------------------------
	// Set the previous value
	//----------------------------------------
	for (i=0; i<CDIO_MAX_TRG_BITS/8; i++) {
		// Read if it has the input possible port
		lret	= Cdiobi_check_inp_port(&dio_data->bios_d, i);
		if (lret) {
			Cdiobi_input_port(&dio_data->bios_d, i, &trg_data->back_data[i]);
		} else {
			break;
		}
	}
	//----------------------------------------
	// Set the comparison bit, the logic and the maximum value
	//----------------------------------------
	trg_data->logic[param->bit_no] = param->logic;
	if (trg_data->num == 0 || trg_data->num < (param->bit_no + 1)) {
		trg_data->num	= param->bit_no + 1;
	}
	//----------------------------------------
	// Set the timer and start
	//----------------------------------------
	// Timer to a smaller value
	tim_backup	= trg_data->tim;
	if (trg_data->tim == 0 || trg_data->tim > param->tim) {
		trg_data->tim	= param->tim;
	}
	trg_data->dio_data	= (void *)dio_data;		// Parent DIO_DATA
	lret	= Ccom_set_timer(trg_data->timer_id, trg_data->tim, Cdio_trigger_timer, (void *)trg_data);
	// Check the timer error
	if (lret != CCOM_NO_ERROR) {
		trg_data->tim		= tim_backup;
		param->ret	= DIO_ERR_SYS_TIM;
		//----------------------------------------
		// Release the spinlock
		//----------------------------------------
		spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
		return 0;
	}
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
	//----------------------------------------
	// Set the return value
	//----------------------------------------
	param->ret	= DIO_ERR_SUCCESS;
	return 0;
}

//================================================================
// Function for retrieving device information
//================================================================
long Cdio_get_device_info(void *device_ext, PCDIO_GET_INFO param)
{
	PCDIO_DATA		dio_data;
	long			lret = DIO_ERR_SUCCESS;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data	= (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Call the subroutine
	//----------------------------------------
	lret = Cdiobi_get_info(param->board_name, param->type, param->param1,\
							param->param2, param->param3);
	//----------------------------------------
	// Set the return value
	//----------------------------------------
	param->ret	= 0;
	switch (lret) {
	case 0:				// Normal completed
		param->ret	= 0;
		break;
	case -1:			// Information type error
		param->ret	= DIO_ERR_INFO_INVALID_INFOTYPE;
		break;
	case -2:			// Buffer address error
		param->ret	= DIO_ERR_DLL_BUFF_ADDRESS;
		break;
	}
	return 0;
}

//================================================================
// Thread wait function
//================================================================
long Cdio_wait_event(void *device_ext, PCDIO_TH_WAIT param)
{
	PCDIO_DATA		dio_data;
	short			i;
	PCDIO_PROC_DATA	proc_data;

	//----------------------------------------
	// Cast the structure
	//----------------------------------------
	dio_data	= (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	proc_data	= NULL;
	//----------------------------------------
	// Search the process information
	//----------------------------------------
	for (i=0; i<CDIO_MAX_PROCESS; i++) {
		if (dio_data->proc_data[i].process_id == param->process_id) {
			proc_data	= &dio_data->proc_data[i];
			break;
		}
	}
	if (proc_data == NULL) {
		return 0;
	}
	//----------------------------------------
	// Move to waiting state
	// Return if there is an interrupt or a trigger
	//----------------------------------------
	Ccom_sleep_on(proc_data->wait_obj, (dio_data->proc_data[i].int_data.sence_num > 0 || dio_data->proc_data[i].trg_data.comp_valid != 0));
	param->ret	= DIO_ERR_SUCCESS;
	return 0;
}

//================================================================
// Thread wake-up function
//================================================================
long Cdio_wake_up_event(void *device_ext, PCDIO_TH_WAKE_UP param)
{
	PCDIO_DATA			dio_data;
	short				i;
	PCDIO_PROC_DATA		proc_data;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data	= (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	proc_data	= NULL;
	//----------------------------------------
	// Search the process information
	//----------------------------------------
	for (i=0; i<CDIO_MAX_PROCESS; i++) {
		if (dio_data->proc_data[i].process_id == param->process_id) {
			proc_data	= &dio_data->proc_data[i];
			break;
		}
	}
	if (proc_data == NULL) {
		return 0;
	}
	//----------------------------------------
	// Wake up thread
	//----------------------------------------
	Ccom_wake_up(&proc_data->wait_obj);
	param->ret	= DIO_ERR_SUCCESS;
	return 0;
}

//================================================================
// Function for retrieving interrupt factor (subroutine)
//================================================================
// Interrupt synchronization routine structure
typedef struct {
	PCDIO_DATA			dio_data;
	PCDIO_INT_DATA		int_data;
	PCDIO_TH_GET_INT	param;
} GET_INT_SUB, *PGET_INT_SUB;
// Interrupt synchronization subroutine

static void Cdio_sub_get_interrupt_factor(void *data)
{
	PGET_INT_SUB		param1;
	PCDIO_DATA			dio_data;
	PCDIO_INT_DATA		int_data;
	PCDIO_TH_GET_INT	param;

	//----------------------------------------
	// Transfer structure
	//----------------------------------------
	param1		= (PGET_INT_SUB)data;
	dio_data	= param1->dio_data;
	int_data	= param1->int_data;
	param		= param1->param;
	//----------------------------------------
	// Clear the return data
	//----------------------------------------
	memset(param, 0, sizeof(CDIO_TH_GET_INT));
	//----------------------------------------
	// Copy the return data
	//----------------------------------------
	param->max_bit		= int_data->max_bit;	// The maximum number of bits
	param->sence_num	= int_data->sence_num;	// The number of sence
	memcpy(&param->sence_arry[0][0], &int_data->sence_arry[0][0], sizeof(param->sence_arry));	// Sence queque
	Cdiobi_get_int_logic(&dio_data->bios_d, &param->logic[0]);	// Logic
	// Clear after copying
	memset(&int_data->sence_arry[0][0], 0, sizeof(int_data->sence_arry));
	int_data->sence_num	= 0;	// The number of sence
}

//================================================================
// Function for retrieving interrupt factor
//================================================================
long Cdio_get_interrupt_factor(void *device_ext, PCDIO_TH_GET_INT param)
{
	PCDIO_DATA			dio_data;
	short				i;
	PCDIO_INT_DATA		int_data;
	GET_INT_SUB			sub_data;
	unsigned long		flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data		= (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the process structure
	//----------------------------------------
	int_data	= NULL;
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	for (i=0; i<CDIO_MAX_PROCESS; i++) {
		//----------------------------------------
		// Store the address if the process is in use
		//----------------------------------------
		if (dio_data->proc_data[i].process_id == param->process_id) {
			int_data	= &dio_data->proc_data[i].int_data;
			break;
		}
	}
	if (int_data == NULL) {
		param->ret	= DIO_ERR_SYS_USING_OTHER_PROCESS;
		//----------------------------------------
		// Release the spinlock
		//----------------------------------------
		spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
		return 0;
	}
	//----------------------------------------
	// Execute synchronized with interrupt
	//----------------------------------------
	// Set the parameters
	sub_data.dio_data	= dio_data;
	sub_data.int_data	= int_data;
	sub_data.param		= param;
	// Execute the subroutine synchronized with interrupt
//	Ccom_sync_interrupt(dio_data->irq_no, Cdio_sub_get_interrupt_factor, (void *)&sub_data);
	Cdio_sub_get_interrupt_factor((void *)&sub_data);
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
	param->ret	= DIO_ERR_SUCCESS;
	return 0;
}

//================================================================
// Function for retrieving trigger factor (subroutine)
//================================================================
// Trigger synchronization routine structure
typedef struct {
	PCDIO_DATA			dio_data;
	PCDIO_TRG_DATA		trg_data;
	PCDIO_TH_GET_TRG	param;
} GET_TRG_SUB, *PGET_TRG_SUB;

// Trigger synchronization subroutine
static void Cdio_sub_get_trigger_factor(void *data)
{
	PGET_TRG_SUB		param1;
	PCDIO_DATA			dio_data;
	PCDIO_TRG_DATA		trg_data;
	PCDIO_TH_GET_TRG	param;

	//----------------------------------------
	// Transfer structure
	//----------------------------------------
	param1		= (PGET_TRG_SUB)data;
	dio_data	= param1->dio_data;
	trg_data	= param1->trg_data;
	param		= param1->param;
	//----------------------------------------
	// Clear the return data
	//----------------------------------------
	memset(param, 0, sizeof(CDIO_TH_GET_TRG));
	//----------------------------------------
	// Copy the return data
	//----------------------------------------
	if (trg_data->comp_valid == 1) {
		memcpy(&param->data[0], &trg_data->comp[0], sizeof(param->data));	// Trigger data
		param->num	= trg_data->num;										// The number of trigger
	}
	// Clear after copying
	memset(&trg_data->comp[0], 0, sizeof(trg_data->comp));
	trg_data->comp_valid	= 0;											// Store comparison result Y/N(0:N, 1:Y)
}

//================================================================
// Function for retrieving trigger factor
//================================================================
long Cdio_get_trigger_factor(void *device_ext, PCDIO_TH_GET_TRG param)
{
	PCDIO_DATA			dio_data;
	short				i;
	PCDIO_TRG_DATA		trg_data;
	GET_TRG_SUB			sub_data;
	unsigned long		flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data	= (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the process structure
	//----------------------------------------
	trg_data	= NULL;
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	for (i=0; i<CDIO_MAX_PROCESS; i++) {
		//----------------------------------------
		// Store the address if the process is in use
		//----------------------------------------
		if (dio_data->proc_data[i].process_id == param->process_id) {
			trg_data	= &dio_data->proc_data[i].trg_data;
			break;
		}
	}
	if (trg_data == NULL) {
		param->ret	= DIO_ERR_SYS_USING_OTHER_PROCESS;
		spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
		return 0;
	}
	//----------------------------------------
	// Execute synchronized with interrupt
	//----------------------------------------
	// Set the parameters
	sub_data.dio_data	= dio_data;
	sub_data.trg_data	= trg_data;
	sub_data.param		= param;
	Cdio_sub_get_trigger_factor((void *)&sub_data);
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
	param->ret	= DIO_ERR_SUCCESS;
	return 0;
}

//================================================================
// Trigger timer (comparison subroutine)
//================================================================
short Cdio_trigger_timer_comp(unsigned char *now_data, unsigned char *back_data, short bit_no)
{
	unsigned char	now, back;
	short			comp;

	//----------------------------------------
	// Retrieve the bit
	//----------------------------------------
	now		= (now_data[bit_no/8] >> (bit_no%8)) & 0x01;
	back	= (back_data[bit_no/8] >> (bit_no%8)) & 0x01;
	//----------------------------------------
	// Comparison
	//----------------------------------------
	if (back == 0 && now == 1) {
		comp	= DIO_TRG_RISE;
	} else if (back == 1 && now == 0) {
		comp	= DIO_TRG_FALL;
	} else {
		comp	= DIO_TRG_NONE;
	}
	return comp;
}

//================================================================
// Trigger timer
//================================================================
void Cdio_trigger_timer(void *param)
{
	PCDIO_TRG_DATA	trg_data;
	unsigned char	now_data[CDIO_MAX_TRG_BITS/8];	// Current input value
	short			i;
	PCDIO_DATA		dio_data;
	short			comp;
	long			lret;
	unsigned long	flags;

	//----------------------------------------
	// Set the structure
	//----------------------------------------
	trg_data	= (PCDIO_TRG_DATA)param;
	if (param == NULL) {
		return;
	}
	dio_data	= (PCDIO_DATA)trg_data->dio_data;
	if (dio_data == NULL) {
		return;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//----------------------------------------
	// Input the current value
	//----------------------------------------
	for (i=0; i<CDIO_MAX_TRG_BITS/8; i++) {
		// Read if it has the input available ports
		lret = Cdiobi_check_inp_port(&dio_data->bios_d, i);
		if (lret) {
			Cdiobi_input_port(&dio_data->bios_d, i, &now_data[i]);
		} else {
			now_data[i]	= 0;
		}
	}
	//----------------------------------------
	// Execute comparisons for all bits and store
	//----------------------------------------
	for (i=0; i<trg_data->num; i++) {
		comp	= Cdio_trigger_timer_comp(now_data, trg_data->back_data, i);
		comp	&= trg_data->logic[i];
		trg_data->comp[i] |= comp;
		if (comp) {
			trg_data->comp_valid	= 1;	// Store comparison result Y/N(0:N, 1:Y)
		}
	}
	//----------------------------------------
	// Copy the current work to the previous work
	//----------------------------------------
	memcpy(&trg_data->back_data[0], now_data, sizeof(trg_data->back_data));
	//----------------------------------------
	// To wake up
	//----------------------------------------
	for (i=0; i<CDIO_MAX_PROCESS; i++) {
		trg_data = &dio_data->proc_data[i].trg_data;
		if (trg_data->comp_valid == 1) {
			Ccom_wake_up(&dio_data->proc_data[i].wait_obj);
		}
	}
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
}

//================================================================
// Interrupt service routine
//================================================================
// Kernel version 2.2.X - 2.4.X
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
void Cdio_isr(int irq, void *param)
// Kernel version 2.6.X and 3.10.X
#elif ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
irqreturn_t Cdio_isr(int irq, void *param)
#endif
{
	PCDIO_DATA		dio_data;
	long			lret;
	short			i;
	short			j;
	PCDIO_INT_DATA	int_data;
	unsigned char	sence[CDIOBI_INT_SENCE_SIZE];
	unsigned char	sence_copy[CDIOBI_INT_SENCE_SIZE];
	unsigned long	flags;

	//----------------------------------------
	// Set the structure
	//----------------------------------------
	dio_data	= (PCDIO_DATA)param;
	if (dio_data == NULL) {
		//--------------------------------------
		// Kernel version 2.2.X - 2.4.X
		//--------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
		return;
		//--------------------------------------
		// Kernel version 2.6.X and 3.10.X
		//--------------------------------------
#elif ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
		return IRQ_NONE;
#endif
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//----------------------------------------
	// If the BusMaster library is supported in the kernel
	//----------------------------------------
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
	//----------------------------------------
	// If it is DM board
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type == CDIOBI_BT_PCI_DM){
		//----------------------------------------
		// Execute a BusMaster related process and return interrupt sense of general-purpose input
		//----------------------------------------
		lret	= Cdio_dm_isr(param, sence);
		//----------------------------------------
		// Return if it is not own interrupt
		//----------------------------------------
		if (lret == 0) {
			//--------------------------------------
			// Kernel version 2.2.X - 2.4.X
			//--------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
			//----------------------------------------
			// Release the spinlock
			//----------------------------------------
			spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
			return;
			//--------------------------------------
			// Kernel version 2.6.X and 3.10.X
			//--------------------------------------
#elif ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
			//----------------------------------------
			// Release the spinlock
			//----------------------------------------
			spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
			return IRQ_NONE;
#endif
		//----------------------------------------
		// Return the BusMaster related interrupt (If it is a general-purpose input interrupt, execute the post-processing)
		//----------------------------------------
		}else if(lret == 2){
			//--------------------------------------
			// Kernel version 2.2.X - 2.4.X
			//--------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
			//----------------------------------------
			// Release the spinlock
			//----------------------------------------
			spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
			return;
			//--------------------------------------
			// Kernel version 2.6.X and 3.10.X
			//--------------------------------------
#elif ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
			//----------------------------------------
			// Release the spinlock
			//----------------------------------------
			spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
			return IRQ_HANDLED;
#endif
		}
	}
#endif
	//----------------------------------------
	// If it isn't DM board
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		//----------------------------------------
		// Input the interrupt status
		//----------------------------------------
		lret	= Cdiobi_sence_int_status(&dio_data->bios_d, sence);
		//----------------------------------------
		// Return if it is not own interrupt
		//----------------------------------------
		if (lret == 0) {
			//--------------------------------------
			// Kernel version 2.2.X - 2.4.X
			//--------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
			//----------------------------------------
			// Release the spinlock
			//----------------------------------------
			spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
			return;
			//--------------------------------------
			// Kernel version 2.6.X and 3.10.X
			//--------------------------------------
#elif ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
			//----------------------------------------
			// Release the spinlock
			//----------------------------------------
			spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
			return IRQ_NONE;
#endif
		}
	}
	//----------------------------------------
	// Copy to the interrupt structure of process data
	//----------------------------------------
	for (i=0; i<CDIO_MAX_PROCESS; i++) {
		//----------------------------------------
		// Store the copy if the process is in use
		//----------------------------------------
		int_data = &dio_data->proc_data[i].int_data;
		if (dio_data->proc_data[i].process_id != 0 &&
			int_data->max_bit != 0 &&		// During monitoring the interrupt
			int_data->sence_num < CDIO_MAX_SENCE_NUM) {
			memcpy(sence_copy, sence, sizeof(sence));
			for (j=0; j< CDIOBI_INT_SENCE_SIZE; j++) {
				sence_copy[j]	&= ~(int_data->mask[j]);
			}
			memcpy(&int_data->sence_arry[int_data->sence_num][0], sence_copy, sizeof(sence_copy));
			int_data->sence_num++;
			//----------------------------------------
			// To wake up
			//----------------------------------------
			Ccom_wake_up(&dio_data->proc_data[i].wait_obj);
		}
	}
	//----------------------------------------
	// Call the user interrupt handling
	//----------------------------------------
	Cdio_user_irq(dio_data->board_name, (unsigned short)dio_data->board_id, dio_data->port_base, sence);
	//--------------------------------------
	// Kernel version 2.2.X - 2.4.X
	//--------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
	return;
	//--------------------------------------
	// Kernel version 2.6.X and 3.10.X
	//--------------------------------------
#elif ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
	return IRQ_HANDLED;
#endif
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))
MODULE_DESCRIPTION("API-DIO(LNX) Device Driver");
MODULE_AUTHOR("CONTEC CO., LTD.");
MODULE_VERSION("6.50");
MODULE_LICENSE("Proprietary");
#endif
