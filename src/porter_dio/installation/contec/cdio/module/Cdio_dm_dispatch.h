////////////////////////////////////////////////////////////////////////////////
/// @file   Cdio_dm_dispatch.h
/// @brief  API-DIO(LNX) PCI Module - Dispatch header file (for DM Board)
/// @author &copy;CONTEC CO.,LTD.
/// @since  2002
////////////////////////////////////////////////////////////////////////////////
// Source File    Module:
//                Ccom_module.c,Ccom_module.h,Cdio_bios.c,Cdio_bios.h
//                Cdio_dispatch.c,Cdio_dispatch.h,Cdio_user_irq.c
//                Cdio_dm_dispatch.c,Cdio_dm_dispatch.h
//                Library:
//                Ccom_sl.c,Ccom_sl.h,Cdio_sl.c,Cdio_sl.h,Cdio_ctrl.h
//                Cdio_dm_sl.c,Cdio_dm_sl.h,Cdio_dm_ctrl.h
//
// Makefile       Makefile
//
// Output File    Module : cdio.o
//                Library: libcdio.so.1.0
//
// OS             Linux
// Compiler       gcc
//
// Support Board  PIO-32DM(PCI)
//
// First release  
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __CDIO_DM_DISPATCH_H_DEFINED__
#define __CDIO_DM_DISPATCH_H_DEFINED__
#include "../library/Cdio_ctrl.h"
#include "../library/Cdio_dm_ctrl.h"
#include "Ccom_module.h"
#include "Cdio_bios.h"
#include "BusMaster.h"
#include "Cdio_dispatch.h"

//================================================================
// Definition values
//================================================================
//================================================================
// Structure
//================================================================
typedef struct {
	unsigned long	Start;
	unsigned long	Clock;
	unsigned long	Stop;
	unsigned long	Pattern;
	unsigned long	Mask;
	unsigned long	InternalClock;
	unsigned long	StopNum;
} SAMPLING, *PSAMPLING;

typedef struct {
	unsigned long	Start;
	unsigned long	Clock;
	unsigned long	Stop;
	unsigned long	InternalClock;
	unsigned long	StopNum;
} GENERATING, *PGENERATING;

typedef	struct {
	unsigned long	dwAddr;				// Base address (PIO)
	unsigned long	dwAddrBM;			// Base address (BM)
	MASTERADDR		Master;				// BusMaster structure
	unsigned long	PioSence;			// Interrupt sence
	unsigned long	PioMask;			// Interrupt mask
	unsigned short	KeepStart;			// Keep PIO start
	SAMPLING		Sampling;			// Sampling data
	GENERATING		Generating;			// Generating data
	unsigned long	WaitTimeWhenOutput;	// How many milliseconds to wait when output with DMA (more than 10 ms)
} BMBOARD, *PBMBOARD;

//================================================================
// Prototype declaration
//================================================================

long Cdio_dm_set_dir(void *device_ext, PCDIO_DM_SET_DIR param);
long Cdio_dm_set_stand_alone(void *device_ext, PCDIO_DM_SET_STAND_ALONE param);
long Cdio_dm_set_master(void *device_ext, PCDIO_DM_SET_MASTER param);
long Cdio_dm_set_slave(void *device_ext, PCDIO_DM_SET_SLAVE param);
long Cdio_dm_set_start_trg(void *device_ext, PCDIO_DM_SET_START_TRG param);
long Cdio_dm_set_start_ptn(void *device_ext, PCDIO_DM_SET_START_PTN param);
long Cdio_dm_set_clock_trg(void *device_ext, PCDIO_DM_SET_CLOCK_TRG param);
long Cdio_dm_set_internal_clock(void *device_ext, PCDIO_DM_SET_INTERNAL_CLOCK param);
long Cdio_dm_set_stop_trg(void *device_ext, PCDIO_DM_SET_STOP_TRG param);
long Cdio_dm_set_stop_num(void *device_ext, PCDIO_DM_SET_STOP_NUM param);
long Cdio_dm_reset(void *device_ext, PCDIO_DM_RESET param);
long Cdio_dm_set_buf(void *device_ext, PCDIO_DM_SET_BUF param);
long Cdio_dm_start(void *device_ext, PCDIO_DM_START param);
long Cdio_dm_stop(void *device_ext, PCDIO_DM_STOP param);
long Cdio_dm_get_status(void *device_ext, PCDIO_DM_GET_STATUS param);
long Cdio_dm_get_count(void *device_ext, PCDIO_DM_GET_COUNT param);
long Cdio_dm_get_write_pointer_user_buf(void *device_ext, PCDIO_DM_GET_WRITEPOINTER_USER_BUF param);
long Cdio_dm_get_fifo_count(void *device_ext, PCDIO_DM_GET_FIFO_COUNT param);
long Cdio_dm_get_int_count(void *device_ext, PCDIO_DM_GET_INT_COUNT param);
long Cdio_dm_set_count_event(void *device_ext, PCDIO_DM_SET_COUNT_EVENT param);
long Cdio_dm_wait_event(void *device_ext, PCDIO_DM_TH_WAIT param);
long Cdio_dm_wake_up_event(void *device_ext, PCDIO_DM_TH_WAKE_UP param);
long Cdio_dm_isr(void *device_ext, unsigned char *sence);

#endif

