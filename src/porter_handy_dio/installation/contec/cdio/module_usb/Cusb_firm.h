////////////////////////////////////////////////////////////////////////////////
/// @file   Cusb_firm.h
/// @brief  API-DIO(LNX) USB Module - URB access header file
/// @author &copy;CONTEC CO.,LTD.
/// @since  2017
////////////////////////////////////////////////////////////////////////////////

#ifndef __CUSB_FIRM_H__
#define __CUSB_FIRM_H__

#include "Cusb_module.h"

//================================================================
///@name Definition
//@{
//================================================================
#define CUSB_FIRM_INTERVAL_TIME	100		///< URB Schedule interval time(ms)
//@}

//================================================================
///@name Debug macro
//@{
//================================================================
//#define DEBUG							///< Debug print on/off
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,4,6))

#ifdef DEBUG
#define	PRINT_DBG(fmt, args...)	printk(__FILE__ "(%d)%s:" fmt, __LINE__, __FUNCTION__, ## args);	///< Debug log
#define	PRINT_ERR(fmt, args...)	printk(__FILE__ "(%d)%s:" fmt, __LINE__, __FUNCTION__, ## args);	///< Error log
#define	PRINT_INF(fmt, args...)	printk(__FILE__ "(%d)%s:" fmt, __LINE__, __FUNCTION__, ## args);	///< Information log
#else
#define	PRINT_DBG(fmt, args...)																		///< Debug log
#define	PRINT_ERR(fmt, args...)	printk(__FILE__ "(%d)%s:" fmt, __LINE__, __FUNCTION__, ## args);	///< Error log
#define	PRINT_INF(fmt, args...)	printk(__FILE__ "(%d)%s:" fmt, __LINE__, __FUNCTION__, ## args);	///< Information log
#endif

#else
#define	PRINT_DBG(fmt, args...)
#define	PRINT_ERR(fmt, args...)
#define	PRINT_INF(fmt, args...)
#endif
//@}

//================================================================
/// Function prototype
//================================================================
long Cusb_firm_write_urb(struct cdio_usb *dev, __u8 request, __u16 value, __u16 index, __u16 size, void *data);
long Cusb_firm_read_urb(struct cdio_usb *dev, __u8 request, __u16 value, __u16 index, __u16 size, void *data);
long Cusb_firm_interrupt_urb(struct cdio_usb *dev, __u16 size, void *data);

#endif // __CUSB_FIRM_H__
