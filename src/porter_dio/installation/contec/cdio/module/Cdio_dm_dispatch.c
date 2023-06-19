////////////////////////////////////////////////////////////////////////////////
/// @file   Cdio_dm_dispatch.c
/// @brief  API-DIO(LNX) PCI Module - Dispatch source file (for DM Board)
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

#include "Ccom_module.h"
#include "Cdio_dispatch.h"
#include "Cdio_dm_dispatch.h"
#include "Cdio_bios.h"
#include "BusMaster.h"

//--------------------------------------------------
// Macro definition
//--------------------------------------------------
// Macro for I/O
/*
#define BmInpB(p, d)		{d = (unsigned char)inb(p);		printk("IN %04X, %02X\n", p, d);}
#define BmInpW(p, d)		{d = (unsigned short)inw(p);	printk("IN %04X, %04X\n", p, d);}
#define BmInpD(p, d)		{d = (unsigned long)inl(p);		printk("IN %04X, %08X\n", p, d);}
#define BmOutB(p, d)		{outb(d, p);					printk("OUT %04X, %02X\n", p, d);}
#define BmOutW(p, d)		{outw(d, p);					printk("OUT %04X, %04X\n", p, d);}
#define BmOutD(p, d)		{outl(d, p);					printk("OUT %04X, %08X\n", p, d);}
*/

#define BmInpB(p, d)		d = (unsigned char)inb(p)
#define BmInpW(p, d)		d = (unsigned short)inw(p)
#define BmInpD(p, d)		d = (unsigned long)inl(p)
#define BmOutB(p, d)		outb(d, p)
#define BmOutW(p, d)		outw(d, p)
#define BmOutD(p, d)		outl(d, p)

//================================================================
// Each function 
//================================================================
//================================================================
// I/O direction setting
//================================================================
long Cdio_dm_set_dir(void *device_ext, PCDIO_DM_SET_DIR param)
{
	PCDIO_DATA			dio_data;
	unsigned long		flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data	= (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the supported boards
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		param->ret= DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//---------------------------------------------
	// Set direction to the hardware directly.
	//---------------------------------------------
	switch(param->dir) {
	case PI_32:		BmOutW(dio_data->bm_board.dwAddr + 0x04, 0x00);	break;
	case PO_32:		BmOutW(dio_data->bm_board.dwAddr + 0x04, 0x03);	break;
	case PIO_1616:	BmOutW(dio_data->bm_board.dwAddr + 0x04, 0x02);	break;
	default:		param->ret = DIO_ERR_SYS_DIRECTION;	return 0; break;
	}
	//----------------------------------------
	// Initialize the output latch data
	//----------------------------------------
	Cdiobi_init_latch_data(&dio_data->bios_d);
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
// Stand alone setting
//================================================================
long Cdio_dm_set_stand_alone(void *device_ext, PCDIO_DM_SET_STAND_ALONE param)
{
	PCDIO_DATA		dio_data;
	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data	= (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the supported boards
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		param->ret = DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//---------------------------------------------
	// Set to the hardware directly.
	//---------------------------------------------
	//ExtSig1
	BmOutW(dio_data->bm_board.dwAddr + 0x08, 0x01);
	BmOutD(dio_data->bm_board.dwAddr + 0x0c, 0x00000000);
	//ExtSig2
	BmOutW(dio_data->bm_board.dwAddr + 0x08, 0x02);
	BmOutD(dio_data->bm_board.dwAddr + 0x0c, 0x00000000);
	//ExtSig3
	BmOutW(dio_data->bm_board.dwAddr + 0x08, 0x03);
	BmOutD(dio_data->bm_board.dwAddr + 0x0c, 0x00000000);
	// Sync Control Connector
	BmOutD(dio_data->bm_board.dwAddr + 0x18, 0);
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
// Master setting
//================================================================
long Cdio_dm_set_master(void *device_ext, PCDIO_DM_SET_MASTER param)
{
	PCDIO_DATA		dio_data;
	unsigned long	ExtSig1;
	unsigned long	ExtSig2;
	unsigned long	ExtSig3;
	unsigned long	MasterHalt;
	unsigned long	SlaveHalt;
	unsigned long	Enable;
	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data	= (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the supported boards
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		param->ret	= DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//---------------------------------------------
	// Set master configuration to the hardware directly.
	//---------------------------------------------
	//---------------------------------------------
	// Converse ExtSig1
	//---------------------------------------------
	switch(param->ext_sig1) {
	case 0:									ExtSig1 = 0x00000000;	break;
	case DIODM_EXT_START_SOFT_IN:			ExtSig1 = 0x00000007;	break;
	case DIODM_EXT_STOP_SOFT_IN:			ExtSig1 = 0x00000008;	break;
	case DIODM_EXT_CLOCK_IN:				ExtSig1 = 0x00000001;	break;
	case DIODM_EXT_EXT_TRG_IN:				ExtSig1 = 0x00000002;	break;
	case DIODM_EXT_START_EXT_RISE_IN:		ExtSig1 = 0x00000003;	break;
	case DIODM_EXT_START_EXT_FALL_IN:		ExtSig1 = 0x00000004;	break;
	case DIODM_EXT_START_PATTERN_IN:		ExtSig1 = 0x0000000b;	break;
	case DIODM_EXT_STOP_EXT_RISE_IN:		ExtSig1 = 0x00000005;	break;
	case DIODM_EXT_STOP_EXT_FALL_IN:		ExtSig1 = 0x00000006;	break;
	case DIODM_EXT_CLOCK_ERROR_IN:			ExtSig1 = 0x00000009;	break;
	case DIODM_EXT_HANDSHAKE_IN:			ExtSig1 = 0x0000000a;	break;
	case DIODM_EXT_TRNSNUM_IN:				ExtSig1 = 0x0000000c;	break;
	case DIODM_EXT_START_SOFT_OUT:			ExtSig1 = 0x00001007;	break;
	case DIODM_EXT_STOP_SOFT_OUT:			ExtSig1 = 0x00001008;	break;
	case DIODM_EXT_CLOCK_OUT:				ExtSig1 = 0x00001001;	break;
	case DIODM_EXT_EXT_TRG_OUT:				ExtSig1 = 0x00001002;	break;
	case DIODM_EXT_START_EXT_RISE_OUT:		ExtSig1 = 0x00001003;	break;
	case DIODM_EXT_START_EXT_FALL_OUT:		ExtSig1 = 0x00001004;	break;
	case DIODM_EXT_STOP_EXT_RISE_OUT:		ExtSig1 = 0x00001005;	break;
	case DIODM_EXT_STOP_EXT_FALL_OUT:		ExtSig1 = 0x00001006;	break;
	case DIODM_EXT_CLOCK_ERROR_OUT:			ExtSig1 = 0x00001009;	break;
	case DIODM_EXT_HANDSHAKE_OUT:			ExtSig1 = 0x0000100a;	break;
	case DIODM_EXT_TRNSNUM_OUT:				ExtSig1 = 0x0000100c;	break;
	default:		param->ret = DIO_ERR_SYS_SIGNAL;	return 0;	break;
	}
	//---------------------------------------------
	// Converse ExtSig2
	//---------------------------------------------
	switch(param->ext_sig2) {
	case 0:									ExtSig2 = 0x00000000;	break;
	case DIODM_EXT_START_SOFT_IN:			ExtSig2 = 0x00000007;	break;
	case DIODM_EXT_STOP_SOFT_IN:			ExtSig2 = 0x00000008;	break;
	case DIODM_EXT_CLOCK_IN:				ExtSig2 = 0x00000001;	break;
	case DIODM_EXT_EXT_TRG_IN:				ExtSig2 = 0x00000002;	break;
	case DIODM_EXT_START_EXT_RISE_IN:		ExtSig2 = 0x00000003;	break;
	case DIODM_EXT_START_EXT_FALL_IN:		ExtSig2 = 0x00000004;	break;
	case DIODM_EXT_START_PATTERN_IN:		ExtSig2 = 0x0000000b;	break;
	case DIODM_EXT_STOP_EXT_RISE_IN:		ExtSig2 = 0x00000005;	break;
	case DIODM_EXT_STOP_EXT_FALL_IN:		ExtSig2 = 0x00000006;	break;
	case DIODM_EXT_CLOCK_ERROR_IN:			ExtSig2 = 0x00000009;	break;
	case DIODM_EXT_HANDSHAKE_IN:			ExtSig2 = 0x0000000a;	break;
	case DIODM_EXT_TRNSNUM_IN:				ExtSig2 = 0x0000000c;	break;
	case DIODM_EXT_START_SOFT_OUT:			ExtSig2 = 0x00001007;	break;
	case DIODM_EXT_STOP_SOFT_OUT:			ExtSig2 = 0x00001008;	break;
	case DIODM_EXT_CLOCK_OUT:				ExtSig2 = 0x00001001;	break;
	case DIODM_EXT_EXT_TRG_OUT:				ExtSig2 = 0x00001002;	break;
	case DIODM_EXT_START_EXT_RISE_OUT:		ExtSig2 = 0x00001003;	break;
	case DIODM_EXT_START_EXT_FALL_OUT:		ExtSig2 = 0x00001004;	break;
	case DIODM_EXT_STOP_EXT_RISE_OUT:		ExtSig2 = 0x00001005;	break;
	case DIODM_EXT_STOP_EXT_FALL_OUT:		ExtSig2 = 0x00001006;	break;
	case DIODM_EXT_CLOCK_ERROR_OUT:			ExtSig2 = 0x00001009;	break;
	case DIODM_EXT_HANDSHAKE_OUT:			ExtSig2 = 0x0000100a;	break;
	case DIODM_EXT_TRNSNUM_OUT:				ExtSig2 = 0x0000100c;	break;
	default:		param->ret = DIO_ERR_SYS_SIGNAL;	return 0;	break;
	}
	//---------------------------------------------
	// Converse ExtSig3
	//---------------------------------------------
	switch(param->ext_sig3) {
	case 0:									ExtSig3 = 0x00000000;	break;
	case DIODM_EXT_START_SOFT_IN:			ExtSig3 = 0x00000007;	break;
	case DIODM_EXT_STOP_SOFT_IN:			ExtSig3 = 0x00000008;	break;
	case DIODM_EXT_CLOCK_IN:				ExtSig3 = 0x00000001;	break;
	case DIODM_EXT_EXT_TRG_IN:				ExtSig3 = 0x00000002;	break;
	case DIODM_EXT_START_EXT_RISE_IN:		ExtSig3 = 0x00000003;	break;
	case DIODM_EXT_START_EXT_FALL_IN:		ExtSig3 = 0x00000004;	break;
	case DIODM_EXT_START_PATTERN_IN:		ExtSig3 = 0x0000000b;	break;
	case DIODM_EXT_STOP_EXT_RISE_IN:		ExtSig3 = 0x00000005;	break;
	case DIODM_EXT_STOP_EXT_FALL_IN:		ExtSig3 = 0x00000006;	break;
	case DIODM_EXT_CLOCK_ERROR_IN:			ExtSig3 = 0x00000009;	break;
	case DIODM_EXT_HANDSHAKE_IN:			ExtSig3 = 0x0000000a;	break;
	case DIODM_EXT_TRNSNUM_IN:				ExtSig3 = 0x0000000c;	break;
	case DIODM_EXT_START_SOFT_OUT:			ExtSig3 = 0x00001007;	break;
	case DIODM_EXT_STOP_SOFT_OUT:			ExtSig3 = 0x00001008;	break;
	case DIODM_EXT_CLOCK_OUT:				ExtSig3 = 0x00001001;	break;
	case DIODM_EXT_EXT_TRG_OUT:				ExtSig3 = 0x00001002;	break;
	case DIODM_EXT_START_EXT_RISE_OUT:		ExtSig3 = 0x00001003;	break;
	case DIODM_EXT_START_EXT_FALL_OUT:		ExtSig3 = 0x00001004;	break;
	case DIODM_EXT_STOP_EXT_RISE_OUT:		ExtSig3 = 0x00001005;	break;
	case DIODM_EXT_STOP_EXT_FALL_OUT:		ExtSig3 = 0x00001006;	break;
	case DIODM_EXT_CLOCK_ERROR_OUT:			ExtSig3 = 0x00001009;	break;
	case DIODM_EXT_HANDSHAKE_OUT:			ExtSig3 = 0x0000100a;	break;
	case DIODM_EXT_TRNSNUM_OUT:				ExtSig3 = 0x0000100c;	break;
	default:		param->ret = DIO_ERR_SYS_SIGNAL;	return 0;	break;
	}
	//---------------------------------------------
	// Set to the hardware directly.
	//---------------------------------------------
	//ExtSig1
	BmOutW(dio_data->bm_board.dwAddr + 0x08, 0x01);
	BmOutD(dio_data->bm_board.dwAddr + 0x0c, ExtSig1);
	//ExtSig2
	BmOutW(dio_data->bm_board.dwAddr + 0x08, 0x02);
	BmOutD(dio_data->bm_board.dwAddr + 0x0c, ExtSig2);
	//ExtSig3
	BmOutW(dio_data->bm_board.dwAddr + 0x08, 0x03);
	BmOutD(dio_data->bm_board.dwAddr + 0x0c, ExtSig3);
	// Enable
	MasterHalt	= param->master_halt & 0x01;
	SlaveHalt	= param->slave_halt & 0x01;
	Enable = 0;
	if (ExtSig1 != 0) 		Enable |= 0x10;
	if (ExtSig2 != 0) 		Enable |= 0x08;
	if (ExtSig3 != 0) 		Enable |= 0x04;
	if (MasterHalt != 0)	Enable |= 0x02;
	if (SlaveHalt != 0)		Enable |= 0x01;
	BmOutD(dio_data->bm_board.dwAddr + 0x18, Enable);
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
// Slave setting
//================================================================
long Cdio_dm_set_slave(void *device_ext, PCDIO_DM_SET_SLAVE param)
{
	PCDIO_DATA		dio_data;
	unsigned long	ExtSig1;
	unsigned long	ExtSig2;
	unsigned long	ExtSig3;
	unsigned long	MasterHalt;
	unsigned long	SlaveHalt;
	unsigned long	Enable;
	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data	= (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the supported boards
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		param->ret	= DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	//----------------------------------------
	// Check the arguments
	//----------------------------------------
	if ((param->ext_sig1 != 0 && param->ext_sig1 != 1) ||
		(param->ext_sig2 != 0 && param->ext_sig2 != 1) ||
		(param->ext_sig3 != 0 && param->ext_sig3 != 1) ||
		(param->master_halt != 1 && param->master_halt != 0) ||
		(param->slave_halt != 1 && param->slave_halt != 0)){
		param->ret	= DIO_ERR_SYS_SIGNAL;
		return 0;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//---------------------------------------------
	// Set thread configuration to the hardware directly.
	//---------------------------------------------
	//---------------------------------------------
	// Store in local variables
	//---------------------------------------------
	ExtSig1		= param->ext_sig1 & 0x01;
	ExtSig2		= param->ext_sig2 & 0x01;
	ExtSig3		= param->ext_sig3 & 0x01;
	MasterHalt	= param->master_halt & 0x01;
	SlaveHalt	= param->slave_halt & 0x01;
	//---------------------------------------------
	// Set to the hardware directly.
	//---------------------------------------------
	Enable	= 0x80;		//slave
	if (ExtSig1 != 0) 		Enable |= 0x10;
	if (ExtSig2 != 0) 		Enable |= 0x08;
	if (ExtSig3 != 0) 		Enable |= 0x04;
	if (MasterHalt != 0)	Enable |= 0x02;
	if (SlaveHalt != 0)		Enable |= 0x01;
	BmOutD(dio_data->bm_board.dwAddr + 0x18, Enable);
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
// Start condition setting
//================================================================
long Cdio_dm_set_start_trg(void *device_ext, PCDIO_DM_SET_START_TRG param)
{
	PCDIO_DATA		dio_data;
	unsigned long	Start;
	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data	= (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the supported boards
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		param->ret	= DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	//---------------------------------------------
	// Check the parameters
	//---------------------------------------------
	if ((param->dir != DIODM_DIR_IN) && 
		(param->dir != DIODM_DIR_OUT)){
		param->ret = DIO_ERR_SYS_DIRECTION;
		return 0;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//---------------------------------------------
	// Set start trigger to the work.
	//---------------------------------------------
	//---------------------------------------------
	// Converse the start condition
	//---------------------------------------------
	switch(param->start) {
	case DIODM_START_SOFT:		Start = 0x00000004;	break;
	case DIODM_START_EXT_RISE:	Start = 0x00000005;	break;
	case DIODM_START_EXT_FALL:	Start = 0x00000006;	break;
	case DIODM_START_PATTERN:	Start = 0x00000007;	break;
	case DIODM_START_EXTSIG_1:	Start = 0x00000001;	break;
	case DIODM_START_EXTSIG_2:	Start = 0x00000002;	break;
	case DIODM_START_EXTSIG_3:	Start = 0x00000003;	break;
	default:	param->ret = DIO_ERR_SYS_START;	return 0;	break;
	}
	//---------------------------------------------
	// Store the start trigger
	//---------------------------------------------
	if (param->dir == DIODM_DIR_IN) {
		dio_data->bm_board.Sampling.Start	= Start;
	} else {
		dio_data->bm_board.Generating.Start	= Start;
	}
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
// Pattern matching condition of start condition setting
//================================================================
long Cdio_dm_set_start_ptn(void *device_ext, PCDIO_DM_SET_START_PTN param)
{
	PCDIO_DATA		dio_data;
	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data = (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the supported boards
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		param->ret = DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//---------------------------------------------
	// Set patten to the work.
	//---------------------------------------------
	//---------------------------------------------
	// Store the patten, the mask
	//---------------------------------------------
	dio_data->bm_board.Sampling.Pattern	= param->ptn;
	dio_data->bm_board.Sampling.Mask	= param->mask;
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
// Clock condition setting
//================================================================
long Cdio_dm_set_clock_trg(void *device_ext, PCDIO_DM_SET_CLOCK_TRG param)
{
	PCDIO_DATA		dio_data;
	unsigned long	Clock;
	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data = (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the supported boards
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		param->ret = DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	//---------------------------------------------
	// Check the parameters
	//---------------------------------------------
	if ((param->dir != DIODM_DIR_IN) && 
		(param->dir != DIODM_DIR_OUT)){
		param->ret = DIO_ERR_SYS_DIRECTION;
		return 0;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//---------------------------------------------
	// Set clock trigger to the work.
	//---------------------------------------------
	//---------------------------------------------
	// Converse the clock condition
	//---------------------------------------------
	switch(param->clock) {
	case DIODM_CLK_CLOCK:		Clock = 0x00040000;	break;
	case DIODM_CLK_EXT_TRG:		Clock = 0x00050000;	break;
	case DIODM_CLK_HANDSHAKE:	Clock = 0x00060000;	break;
	case DIODM_CLK_EXTSIG_1:	Clock = 0x00010000;	break;
	case DIODM_CLK_EXTSIG_2:	Clock = 0x00020000;	break;
	case DIODM_CLK_EXTSIG_3:	Clock = 0x00030000;	break;
	default:	param->ret = DIO_ERR_SYS_CLOCK;	return 0;	break;
	}
	//---------------------------------------------
	// Store the clock condition
	//---------------------------------------------
	if (param->dir == DIODM_DIR_IN) {
		dio_data->bm_board.Sampling.Clock	= Clock;
	} else {
		dio_data->bm_board.Generating.Clock	= Clock;
	}
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
// Internal clock setting
//================================================================
long Cdio_dm_set_internal_clock(void *device_ext, PCDIO_DM_SET_INTERNAL_CLOCK param)
{
	PCDIO_DATA		dio_data;
	unsigned long	SetClk;
	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data = (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the supported boards
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		param->ret = DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	//---------------------------------------------
	// Check the parameters
	//---------------------------------------------
	if ((param->dir != DIODM_DIR_IN) && 
		(param->dir != DIODM_DIR_OUT)){
		param->ret = DIO_ERR_SYS_DIRECTION;
		return 0;
	}
	switch(param->unit) {
	case DIODM_TIM_UNIT_S:
		SetClk = param->clock * 40000000;
		if(param->clock > 107){
			param->ret = DIO_ERR_SYS_CLOCK_VAL;
			return 0;
		}
		break;
	case DIODM_TIM_UNIT_MS:	
		SetClk = param->clock * 40000;
		if(param->clock > 107 * 1000){
			param->ret = DIO_ERR_SYS_CLOCK_VAL;
			return 0;
		}
		break;
	case DIODM_TIM_UNIT_US:
		SetClk = param->clock * 40;
		if(param->clock > 107 * 1000 * 1000){
			param->ret = DIO_ERR_SYS_CLOCK_VAL;
			return 0;
		}
		break;
	case DIODM_TIM_UNIT_NS:
		SetClk = param->clock / 25;
		if(param->clock < 50){			// Don't execute because the upper limit check exceeds the maximum value of the type
			param->ret = DIO_ERR_SYS_CLOCK_VAL;
			return 0;
		}
		break;
	default:
		param->ret = DIO_ERR_SYS_CLOCK_UNIT;
		return 0;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//---------------------------------------------
	// Set internal clock to the work.
	//---------------------------------------------
	//---------------------------------------------
	// Store the clock
	//---------------------------------------------
	if (param->dir == DIODM_DIR_IN) {
		dio_data->bm_board.Sampling.InternalClock	= SetClk;
	} else {
		dio_data->bm_board.Generating.InternalClock	= SetClk;
	}
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
// Stop condition setting
//================================================================
long Cdio_dm_set_stop_trg(void *device_ext, PCDIO_DM_SET_STOP_TRG param)
{
	PCDIO_DATA		dio_data;
	unsigned long	Stop;
	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data = (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the supported boards
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		param->ret = DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	//---------------------------------------------
	// Check the parameters
	//---------------------------------------------
	if ((param->dir != DIODM_DIR_IN) && 
		(param->dir != DIODM_DIR_OUT)){
		param->ret = DIO_ERR_SYS_DIRECTION;
		return 0;
	}
	//---------------------------------------------
	// Set stop trigger to the work.
	//---------------------------------------------
	//---------------------------------------------
	// Converse the stop condition
	//---------------------------------------------
	switch(param->stop) {
	case DIODM_STOP_SOFT:		Stop = 0x00000400;	break;
	case DIODM_STOP_EXT_RISE:	Stop = 0x00000500;	break;
	case DIODM_STOP_EXT_FALL:	Stop = 0x00000600;	break;
	case DIODM_STOP_NUM:		Stop = 0x00000700;	break;
	case DIODM_STOP_EXTSIG_1:	Stop = 0x00000100;	break;
	case DIODM_STOP_EXTSIG_2:	Stop = 0x00000200;	break;
	case DIODM_STOP_EXTSIG_3:	Stop = 0x00000300;	break;
	default:					
		param->ret	= DIO_ERR_SYS_STOP;
		return 0;
		break;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//---------------------------------------------
	// Store the stop condition
	//---------------------------------------------
	if (param->dir == DIODM_DIR_IN) {
		dio_data->bm_board.Sampling.Stop	= Stop;
	} else {
		dio_data->bm_board.Generating.Stop	= Stop;
	}
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
// Stop number setting
//================================================================
long Cdio_dm_set_stop_num(void *device_ext, PCDIO_DM_SET_STOP_NUM param)
{
	PCDIO_DATA		dio_data;
	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data = (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the supported boards
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		param->ret = DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	//---------------------------------------------
	// Check the parameters
	//---------------------------------------------
	if ((param->dir != DIODM_DIR_IN) && 
		(param->dir != DIODM_DIR_OUT)){
		param->ret = DIO_ERR_SYS_DIRECTION;
		return 0;
	}
	if (param->stop_num > 0xffffff || param->stop_num == 0 ){
		param->ret = DIO_ERR_SYS_STOP_NUM;
		return 0;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//---------------------------------------------
	// Set stop number to the work
	//---------------------------------------------
	//---------------------------------------------
	// Store the clock
	//---------------------------------------------
	if (param->dir == DIODM_DIR_IN) {
		dio_data->bm_board.Sampling.StopNum		= param->stop_num;
	}else{
		dio_data->bm_board.Generating.StopNum	= param->stop_num;
	}
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
//  FIFO reset
//================================================================
long Cdio_dm_reset(void *device_ext, PCDIO_DM_RESET param)
{
	PCDIO_DATA		dio_data;
	long			lret;
	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data = (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the supported boards
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		param->ret = DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	//---------------------------------------------
	// Check the parameters
	//---------------------------------------------
	if ((param->reset & ~(DIODM_RESET_FIFO_IN | DIODM_RESET_FIFO_OUT))){
		param->ret = DIO_ERR_SYS_RESET;
		return 0;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//---------------------------------------------
	// To reset.
	//---------------------------------------------
	lret = ApiBmReset(&dio_data->bm_board.Master, param->reset);
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
	if (lret == BM_ERROR_PARAM) {
		param->ret = DIO_ERR_DM_PARAM;
		return 0;
	}
	//----------------------------------------
	// Set the return value
	//----------------------------------------
	param->ret = DIO_ERR_SUCCESS;
	return 0;
}

//================================================================
// Buffer setting
//================================================================
long Cdio_dm_set_buf(void *device_ext, PCDIO_DM_SET_BUF param)
{
	PCDIO_DATA		dio_data;
	long			lret;
//	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data = (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the supported boards
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		param->ret = DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	//-------------------------------------------
	// Check the parameters
	//-------------------------------------------
	if ((param->dir != DIODM_DIR_IN) && 
		(param->dir != DIODM_DIR_OUT)){
		param->ret = DIO_ERR_SYS_DIRECTION;
		return 0;
	}
	if (param->len > (0x4000000 / 4)) {
		param->ret = DIO_ERR_SYS_LEN;
		return 0;
	}
	if (param->is_ring != DIODM_WRITE_ONCE && param->is_ring != DIODM_WRITE_RING) {
		param->ret = DIO_ERR_SYS_RING;
		return 0;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
//	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//---------------------------------------------
	// Set buffer to the S/G list.
	//---------------------------------------------
	//---------------------------------------------
	// Set the buffer.
	//---------------------------------------------
	lret = ApiBmSetBuffer(&dio_data->bm_board.Master,
						 param->dir,
						 param->buf,
						 param->len,
						 param->is_ring);
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
//	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
	switch(lret) {
	case BM_ERROR_SUCCESS:
		param->ret = DIO_ERR_SUCCESS;
		break;
	case BM_ERROR_PARAM:
		param->ret = DIO_ERR_DM_PARAM;
		return 0;
		break;
	case BM_ERROR_MEM:
		param->ret = DIO_ERR_SYS_MEMORY;
		return 0;
		break;
	case BM_ERROR_BUFF:
		param->ret = DIO_ERR_DM_BUFFER;
		return 0;
		break;
	case BM_ERROR_LOCK_MEM:
		param->ret = DIO_ERR_DM_LOCK_MEMORY;
		return 0;
		break;
	case BM_ERROR_SEQUENCE:
		param->ret = DIO_ERR_DM_SEQUENCE;
		return 0;
		break;
	default:
		param->ret = lret;
		return 0;
		break;
	}
	return 0;
}
//================================================================
// Sampling/Generating start
//================================================================
long Cdio_dm_start(void *device_ext, PCDIO_DM_START param)
{
	PCDIO_DATA		dio_data;
	unsigned long	Condition;
	long			lret;
	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data = (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the supported boards
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		param->ret = DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//---------------------------------------------
	// Set the sampling condition, 
	// start BusMaster and PIO in the order.
	//---------------------------------------------
	//---------------------------------------------
	// Set the sampling condition
	//---------------------------------------------
	if (param->dir & DIODM_DIR_IN) {
		// Condition
		Condition = dio_data->bm_board.Sampling.Start | dio_data->bm_board.Sampling.Clock | dio_data->bm_board.Sampling.Stop;
		BmOutW(dio_data->bm_board.dwAddr + 0x08, 0x04);
		BmOutD(dio_data->bm_board.dwAddr + 0x0c, Condition);
		// Paramter
		BmOutW(dio_data->bm_board.dwAddr + 0x08, 0x06);
		BmOutD(dio_data->bm_board.dwAddr + 0x0c, dio_data->bm_board.Sampling.Mask);
		BmOutW(dio_data->bm_board.dwAddr + 0x08, 0x07);
		BmOutD(dio_data->bm_board.dwAddr + 0x0c, dio_data->bm_board.Sampling.Pattern);
		// Internal clock
		BmOutW(dio_data->bm_board.dwAddr + 0x08, 0x05);
		BmOutD(dio_data->bm_board.dwAddr + 0x0c, dio_data->bm_board.Sampling.InternalClock);
		// Stop number
		BmOutW(dio_data->bm_board.dwAddr + 0x08, 0x08);
		BmOutD(dio_data->bm_board.dwAddr + 0x0c, dio_data->bm_board.Sampling.StopNum);
		// Open the mask (input) of sampling stop
		dio_data->bm_board.PioMask &= 0xfffffffd;
	}
	//---------------------------------------------
	// Set the generating condition
	//---------------------------------------------
	if (param->dir & DIODM_DIR_OUT) {
		// Condition
		Condition = dio_data->bm_board.Generating.Start | dio_data->bm_board.Generating.Clock | dio_data->bm_board.Generating.Stop;
		BmOutW(dio_data->bm_board.dwAddr + 0x08, 0x09);
		BmOutD(dio_data->bm_board.dwAddr + 0x0c, Condition);
		// Internal clock
		BmOutW(dio_data->bm_board.dwAddr + 0x08, 0x0a);
		BmOutD(dio_data->bm_board.dwAddr + 0x0c, dio_data->bm_board.Generating.InternalClock);
		// Stop number
		BmOutW(dio_data->bm_board.dwAddr + 0x08, 0x0b);
		BmOutD(dio_data->bm_board.dwAddr + 0x0c, dio_data->bm_board.Generating.StopNum);
		// Open the mask (input) of generating stop
		dio_data->bm_board.PioMask &= 0xfffdffff;
	}
	BmOutD(dio_data->bm_board.dwAddr + 0x10, dio_data->bm_board.PioMask);
	//---------------------------------------------
	// Start the BusMaster
	//---------------------------------------------
	dio_data->bm_board.PioSence = 0x00;
	lret = ApiBmStart(&dio_data->bm_board.Master, param->dir);
	switch(lret) {
	case BM_ERROR_SUCCESS:
		param->ret = DIO_ERR_SUCCESS;
		break;
	case BM_ERROR_PARAM:
		param->ret = DIO_ERR_DM_PARAM;
		//----------------------------------------
		// Release the spinlock
		//----------------------------------------
		spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
		return 0;
		break;
	case BM_ERROR_SEQUENCE:
		param->ret = DIO_ERR_DM_SEQUENCE;
		//----------------------------------------
		// Release the spinlock
		//----------------------------------------
		spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
		return 0;
		break;
	default:
		param->ret = lret;
		//----------------------------------------
		// Release the spinlock
		//----------------------------------------
		spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
		return 0;
		break;
	}
	//---------------------------------------------
	// Output
	//---------------------------------------------
	if (param->dir & DIODM_DIR_OUT) {
		//---------------------------------------------
		// If the WaitTimeWhenOutput is not 0 in the setting file
		//---------------------------------------------
		if (dio_data->bm_board.WaitTimeWhenOutput != 0) {
			//----------------------------------------
			// Release the spinlock
			//----------------------------------------
			spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
			//---------------------------------------------
			// Wait for data to accumulate in the FIFO
			//---------------------------------------------
			Ccom_sleep_on_timeout(dio_data->bm_board.WaitTimeWhenOutput);
			//----------------------------------------
			// Get the spinlock
			//----------------------------------------
			spin_lock_irqsave(&dio_data->spinlock_flag, flags);
		}
	}
	//---------------------------------------------
	// Start the PIO
	//---------------------------------------------
	dio_data->bm_board.KeepStart |= param->dir;
	BmOutW(dio_data->bm_board.dwAddr + 0x06, dio_data->bm_board.KeepStart);
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
	param->ret = DIO_ERR_SUCCESS;
	return 0;
}

//================================================================
// Sampling/Generating stop
//================================================================
long Cdio_dm_stop(void *device_ext, PCDIO_DM_STOP param)
{
	PCDIO_DATA		dio_data;
	long			lret;
	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data = (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the supported boards
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		param->ret = DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//---------------------------------------------
	// Stop the PIO
	//---------------------------------------------
	dio_data->bm_board.KeepStart &= ~param->dir;
	BmOutW(dio_data->bm_board.dwAddr + 0x06, dio_data->bm_board.KeepStart);
	//---------------------------------------------
	// Stop the BusMaster
	//---------------------------------------------
	lret = ApiBmStop(&dio_data->bm_board.Master, param->dir);
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
	if (lret == BM_ERROR_PARAM) {
		param->ret = DIO_ERR_DM_PARAM;
		return 0;
	}
	param->ret = DIO_ERR_SUCCESS;
	return 0;
}

//================================================================
// Retrieve the transfer status
//================================================================
long Cdio_dm_get_status(void *device_ext, PCDIO_DM_GET_STATUS param)
{
	PCDIO_DATA		dio_data;
	long			lret;
	long			Start;
	unsigned long	PioStatus;
	unsigned long	BmStatus;
	unsigned long	dwHalt;
	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data = (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the supported boards
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		param->ret = DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	//-------------------------------------------
	// Check the parameters
	//-------------------------------------------
	if ((param->dir != DIODM_DIR_IN) && 
		(param->dir != DIODM_DIR_OUT)){
		param->ret = DIO_ERR_SYS_DIRECTION;
		return 0;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//---------------------------------------------
	// Get the status, the error.
	//---------------------------------------------
	//---------------------------------------------
	// Get the status.
	//---------------------------------------------
	lret = ApiBmGetStatus(&dio_data->bm_board.Master, param->dir, &BmStatus);
	BmInpD(dio_data->bm_board.dwAddr + 0x14, PioStatus);
	Start = ApiBmGetStartEnabled(&dio_data->bm_board.Master, param->dir);
	PioStatus |= dio_data->bm_board.PioSence;
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
	dwHalt = (PioStatus >> 30) & 0x03;
	if (param->dir == DIODM_DIR_IN) {
		PioStatus = PioStatus & 0x0000ffff;
	} else {
		PioStatus = (PioStatus >> 16) & 0x0000ffff;
	}
	//---------------------------------------------
	// Split the status to error and information.
	//---------------------------------------------
	param->status	= 0x00;
	param->err		= 0x00;
	// Status
	if (BmStatus & BM_STATUS_END)			param->status |= DIODM_STATUS_BMSTOP;
	if (PioStatus & 0x0001)					param->status |= DIODM_STATUS_PIOSTART;
	if (PioStatus & 0x0002)					param->status |= DIODM_STATUS_PIOSTOP;
	if (PioStatus & 0x0004)					param->status |= DIODM_STATUS_TRGIN;
	if (PioStatus & 0x0008)					param->status |= DIODM_STATUS_OVERRUN;
	if ((PioStatus != 0 || BmStatus != 0) && Start == BM_TRANS_STOP) param->status |= DIODM_STATUS_BMSTOP;
	// Error
	if (BmStatus & BM_STATUS_FIFO_EMPTY)	param->err |= DIODM_STATUS_FIFOEMPTY;
	if (BmStatus & BM_STATUS_FIFO_FULL)		param->err |= DIODM_STATUS_FIFOFULL;
	if (BmStatus & BM_STATUS_SG_OVER_IN)	param->err |= DIODM_STATUS_SGOVERIN;
	if (PioStatus & 0x0010)					param->err |= DIODM_STATUS_TRGERR;
	if (PioStatus & 0x0020)					param->err |= DIODM_STATUS_CLKERR;
	if (dwHalt    & 0x0001)					param->err |= DIODM_STATUS_SLAVEHALT;
	if (dwHalt    & 0x0002)					param->err |= DIODM_STATUS_MASTERHALT;
	//----------------------------------------
	// Set the return value
	//----------------------------------------
	param->ret = DIO_ERR_SUCCESS;
	return 0;
}

//================================================================
// Retrieve the transfer count
//================================================================
long Cdio_dm_get_count(void *device_ext, PCDIO_DM_GET_COUNT param)
{
	PCDIO_DATA		dio_data;
	long			lret;
	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data = (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the supported boards
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		param->ret = DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	//-------------------------------------------
	// Check the parameters
	//-------------------------------------------
	if ((param->dir != DIODM_DIR_IN) && 
		(param->dir != DIODM_DIR_OUT)){
		param->ret = DIO_ERR_SYS_DIRECTION;
		return 0;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//---------------------------------------------
	// Get the count number.
	//---------------------------------------------
	lret = ApiBmGetCount(&dio_data->bm_board.Master, param->dir, &param->count, &param->carry);
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
	if (lret == BM_ERROR_PARAM) {
		param->ret = DIO_ERR_DM_PARAM;
		return 0;
	}
	//----------------------------------------
	// Set the return value
	//----------------------------------------
	param->ret = DIO_ERR_SUCCESS;
	return 0;
}

//================================================================
// Retrieve the transfer count on user buffer (write point)
//================================================================
long Cdio_dm_get_write_pointer_user_buf(void *device_ext, PCDIO_DM_GET_WRITEPOINTER_USER_BUF param)
{
	PCDIO_DATA		dio_data;
	long			lret;
	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data = (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the supported boards
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		param->ret = DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	//-------------------------------------------
	// Check the parameters
	//-------------------------------------------
	if ((param->dir != DIODM_DIR_IN) && 
		(param->dir != DIODM_DIR_OUT)){
		param->ret = DIO_ERR_SYS_DIRECTION;
		return 0;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//---------------------------------------------
	// Get the count number.
	//---------------------------------------------
	lret = ApiBmGetCount(&dio_data->bm_board.Master, param->dir, &param->count, &param->carry);
	if (lret == BM_ERROR_PARAM) {
		param->ret = DIO_ERR_DM_PARAM;
		//----------------------------------------
		// Release the spinlock
		//----------------------------------------
		spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
		return 0;
	}
	//---------------------------------------------
	// Get the user buffer size.
	//---------------------------------------------
	lret = ApiBmGetBufLen(&dio_data->bm_board.Master, param->dir, &param->buf_len);
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
	if (lret == BM_ERROR_PARAM) {
		param->ret = DIO_ERR_DM_PARAM;
		return 0;
	}
	//----------------------------------------
	// Set the return value
	//----------------------------------------
	param->ret = DIO_ERR_SUCCESS;
	return 0;
}

//================================================================
// Retrieve the FIFO count
//================================================================
long Cdio_dm_get_fifo_count(void *device_ext, PCDIO_DM_GET_FIFO_COUNT param)
{
	PCDIO_DATA		dio_data;
	long			lret;
	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data = (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the supported boards
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		param->ret = DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	//-------------------------------------------
	// Check the parameters
	//-------------------------------------------
	if ((param->dir != DIODM_DIR_IN) && 
		(param->dir != DIODM_DIR_OUT)){
		param->ret = DIO_ERR_SYS_DIRECTION;
		return 0;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//---------------------------------------------
	// Get the FIFO count number.
	//---------------------------------------------
	lret = ApiBmCheckFifo(&dio_data->bm_board.Master, param->dir, &param->count);
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
	if (lret == BM_ERROR_PARAM) {
		param->ret = DIO_ERR_DM_PARAM;
		return 0;
	}
	//----------------------------------------
	// Set the return value
	//----------------------------------------
	param->ret = DIO_ERR_SUCCESS;
	return 0;
}

//================================================================
// Retrieve the BusMaster interrupt count
//================================================================
long Cdio_dm_get_int_count(void *device_ext, PCDIO_DM_GET_INT_COUNT param)
{
	PCDIO_DATA		dio_data;
	BM_GET_INT_CNT	int_count;
	long			lret;
	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data = (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the supported boards
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		param->ret = DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//---------------------------------------------
	// Get the number of interrupt counts.
	//---------------------------------------------
	lret = ApiBmGetIntCount(&dio_data->bm_board.Master, &int_count);
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
	if (lret == BM_ERROR_PARAM) {
		param->ret = DIO_ERR_DM_PARAM;
		return 0;
	}
	//---------------------------------------------
	// Set number of counts to the return value
	//---------------------------------------------
	param->in_count_sence_num	= int_count.dwInCountSenceNum;
	param->in_end_sence_num		= int_count.dwInEndSenceNum;
	param->out_count_sence_num	= int_count.dwOutCountSenceNum;
	param->out_end_sence_num	= int_count.dwOutEndSenceNum;
	//----------------------------------------
	// Set the return value
	//----------------------------------------
	param->ret = DIO_ERR_SUCCESS;
	return 0;
}

//================================================================
// DM thread sleep function
//================================================================
long Cdio_dm_wait_event(void *device_ext, PCDIO_DM_TH_WAIT param)
{
	PCDIO_DATA		dio_data;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data = (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the supported boards
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		param->ret = DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	//---------------------------------------------
	// To sleep the thread for DM
	//---------------------------------------------
	ApiBmWaitEvent(&dio_data->bm_board.Master);
	//----------------------------------------
	// Set the return value
	//----------------------------------------
	param->ret = DIO_ERR_SUCCESS;
	return 0;
}
//================================================================
// DM thread wake-up function
//================================================================
long Cdio_dm_wake_up_event(void *device_ext, PCDIO_DM_TH_WAKE_UP param)
{
	PCDIO_DATA		dio_data;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data = (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the supported boards
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		param->ret = DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	//---------------------------------------------
	// Wake up the thread for DM
	//---------------------------------------------
	ApiBmWakeUpEvent(&dio_data->bm_board.Master);
	//----------------------------------------
	// Set the return value
	//----------------------------------------
	param->ret = DIO_ERR_SUCCESS;
	return 0;
}
//================================================================
// Notification of the transfer completion with specified number setting
//================================================================
long Cdio_dm_set_count_event(void *device_ext, PCDIO_DM_SET_COUNT_EVENT param)
{
	PCDIO_DATA		dio_data;
	long			lret;
	unsigned long	flags;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data = (PCDIO_DATA)device_ext;
	if (dio_data == NULL || param == NULL) {
		return -1;
	}
	//----------------------------------------
	// Check the supported boards
	//----------------------------------------
	if(dio_data->bios_d.b_info->board_type != CDIOBI_BT_PCI_DM){
		param->ret = DIO_ERR_SYS_NOT_SUPPORTED;
		return 0;
	}
	//-------------------------------------------
	// Check the parameters
	//-------------------------------------------
	if ((param->dir != DIODM_DIR_IN) && 
		(param->dir != DIODM_DIR_OUT)){
		param->ret = DIO_ERR_SYS_DIRECTION;
		return 0;
	}
	if ((param->count > 0xffffff) | (param->count == 0)) {
		param->ret = DIO_ERR_SYS_COUNT;
		return 0;
	}
	//----------------------------------------
	// Get the spinlock
	//----------------------------------------
	spin_lock_irqsave(&dio_data->spinlock_flag, flags);
	//---------------------------------------------
	// Set the transfer number interrupt.
	//---------------------------------------------
	lret = ApiBmSetNotifyCount(&dio_data->bm_board.Master, param->dir, param->count);
	//----------------------------------------
	// Release the spinlock
	//----------------------------------------
	spin_unlock_irqrestore(&dio_data->spinlock_flag, flags);
	if (lret == BM_ERROR_PARAM) {
		param->ret = DIO_ERR_DM_PARAM;
		return 0;
	}
	//----------------------------------------
	// Set the return value
	//----------------------------------------
	param->ret = DIO_ERR_SUCCESS;
	return 0;
}

//================================================================
// Post-process at the transfer completion interrupt (local)
//================================================================
long Cdio_dm_trans_end_process(PCDIO_DATA dio_data, unsigned short PioInSence, unsigned short PioOutSence)
{
	unsigned long	dwFifoCount, dwStatus, dwCnt;
	long			Enabled;

	//-----------------------------------------------
	// Completion process of input
	//-----------------------------------------------
	if (PioInSence & 0x02) {
		//-----------------------------------------
		// Stop the process if it was stopped by the master error
		//-----------------------------------------
		ApiBmGetStatus(&dio_data->bm_board.Master, BM_DIR_IN, &dwStatus);
		if (dwStatus & BM_STATUS_END) {
			return 0;
		}
		//-----------------------------------------
		// Wait until FIFO is empty or S/GOverIn comes up
		//-----------------------------------------
		dwCnt = 0;
		for ( ; ; ) {
			dwFifoCount = 0;
			dwStatus = 0;
			ApiBmCheckFifo(&dio_data->bm_board.Master, BM_DIR_IN, &dwFifoCount);
			ApiBmGetStatus(&dio_data->bm_board.Master, BM_DIR_IN, &dwStatus);
			Enabled = ApiBmGetStartEnabled(&dio_data->bm_board.Master, BM_DIR_IN);
			//-----------------------------------------
			// Stop the master transfer
			//-----------------------------------------
			if ((dwFifoCount == 0) || (dwStatus & BM_STATUS_SG_OVER_IN) ||
				(dwStatus & BM_STATUS_END) || (Enabled == BM_TRANS_STOP) || dwCnt == 100) {
				ApiBmStop(&dio_data->bm_board.Master, BM_DIR_IN);
				break;
			}
			dwCnt++;
		}
		// Close the mask and open the Event interrupt mask
		dio_data->bm_board.PioMask |= 0x0000ffff;
		dio_data->bm_board.PioMask &= 0xffff0fff | (((unsigned short)dio_data->bios_d.int_mask[0] << 12) & 0x0000f000);
		BmOutD(dio_data->bm_board.dwAddr + 0x10, dio_data->bm_board.PioMask);
	}
	//-----------------------------------------------
	// Completion process of output
	//-----------------------------------------------
	if (PioOutSence & 0x02) {
		//-----------------------------------------
		// Stop the master transfer
		//-----------------------------------------
		ApiBmStop(&dio_data->bm_board.Master, BM_DIR_OUT);
		//-----------------------------------------
		// Notify the transfer completion
		//-----------------------------------------
		ApiBmSetStopEvent(&dio_data->bm_board.Master, BM_DIR_OUT);
		// Close the mask and open the Event interrupt mask
		dio_data->bm_board.PioMask |= 0xffff0000;
		dio_data->bm_board.PioMask &= 0xffff0fff | (((unsigned short)dio_data->bios_d.int_mask[0] << 12) & 0x0000f000);
		BmOutD(dio_data->bm_board.dwAddr + 0x10, dio_data->bm_board.PioMask);
	}
	//----------------------------------------
	// Set the return value
	//----------------------------------------
	return 0;
}

//================================================================
// Subroutine of the interrupt service routine 
//================================================================
long Cdio_dm_isr(void *device_ext, unsigned char *sence)
{
	PCDIO_DATA		dio_data;
	long			MyInt =  0;		// No own interrupt
	unsigned short	InStatus, OutStatus;
	unsigned short	PioInSence, PioOutSence;
	unsigned long	IntSence;
	unsigned short	lret;

	//----------------------------------------
	// Cast to structure
	//----------------------------------------
	dio_data = (PCDIO_DATA)device_ext;
	if (dio_data == NULL) {
		return 0;
	}
	//-----------------------------------------------
	// Local side
	// In the interrupt handling routine,
	// first process the local side and then
	// process the master side.
	//-----------------------------------------------
	//-----------------------------------------------
	// Interrupt status input and clear
	//-----------------------------------------------
	BmInpD((dio_data->bm_board.dwAddr+0x14), IntSence);
	dio_data->bm_board.PioSence |= IntSence;
	BmOutD((dio_data->bm_board.dwAddr+0x14), IntSence);
	if (((~dio_data->bm_board.PioMask) & IntSence) != 0x0000) {
		MyInt = 2;	// BusMaster interrupt
		//-----------------------------------------------
		// Split interrupt factor
		//-----------------------------------------------
		PioInSence	= (unsigned short)(IntSence & 0xffff);
		PioOutSence	= (unsigned short)((IntSence >> 16) & 0xffff);
		//-----------------------------------------------
		// Completion process of input
		//-----------------------------------------------
		if (PioInSence & 0x02) {
			dio_data->bm_board.KeepStart &= ~BM_DIR_IN;
		}
		//-----------------------------------------------
		// Completion process of output
		//-----------------------------------------------
		if (PioOutSence & 0x02) {
			dio_data->bm_board.KeepStart &= ~BM_DIR_OUT;
		}
		//-----------------------------------------------
		// Post-process of the transfer completion interrupt (local)
		//-----------------------------------------------
		Cdio_dm_trans_end_process(dio_data, PioInSence, PioOutSence);
	}
	//-----------------------------------------------
	// Return the interrupt sence of general-purpose input
	//-----------------------------------------------
	sence[0] = (unsigned char)((IntSence >> 12) & 0x0000000f);
	// OFF the masked bits
	sence[0] = sence[0] & ~(dio_data->bios_d.int_mask[0]);
	if (sence[0] != 0) {
		MyInt = 1;	// General-purpose input interrupt
		return MyInt;
	}
	//-----------------------------------------------
	// Master side
	//-----------------------------------------------
	lret = ApiBmInterrupt(&dio_data->bm_board.Master, &InStatus, &OutStatus);
	if (lret == BM_INT_EXIST) {
		MyInt = 2;	// BusMaster interrupt
	}
	return MyInt;
}

