////////////////////////////////////////////////////////////////////////////////
/// @file   Cdio_bios.c
/// @brief  API-DIO(LNX) PCI Module - BIOS source file
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
#include "Cdio_bios.h"
//================================================================
// Macro definition
//================================================================
// Input/Output
#define		_inp(port)			(unsigned char)inb(port)
#define		_inpw(port)			(unsigned short)inw(port)
#define		_inpd(port)			(unsigned long)inl(port)
#define		_outp(port, data)	outb(data, port)
#define		_outpw(port, data)	outw(data, port)
#define		_outpd(port, data)	outl(data, port)
//================================================================
// External variables
//================================================================
static CDIOBI_BOARD_INFO	board_info[] = {
// H
	{// PIO-32/32L(PCI)H
	board_name			:		"PIO-32/32L(PCI)H",		// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x9152,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		4,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff, 0xff, 0xff},// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PI-64L(PCI)H
	board_name			:		"PI-64L(PCI)H",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x9162,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		8,						// Number of input ports
	out_port_num		:		0,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff, 0xff, 0xff},// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		0,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PO-64L(PCI)H
	board_name			:		"PO-64L(PCI)H",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x9172,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		0,						// Offset of filter address
	int_mask_off		:		0,						// Offset of interrupt mask address
	int_sence_off		:		0,						// Offset of interrupt sence address
	int_ctrl_off		:		0,						// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		8,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{ 0x00},				// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-16/16L(PCI)H
	board_name			:		"PIO-16/16L(PCI)H",		// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x9182,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff },			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PI-32L(PCI)H
	board_name			:		"PI-32L(PCI)H",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x9192,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		0,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff, 0xff, 0xff},// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		0,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PO-32L(PCI)H
	board_name			:		"PO-32L(PCI)H",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x91A2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		0,						// Offset of filter address
	int_mask_off		:		0,						// Offset of interrupt mask address
	int_sence_off		:		0,						// Offset of interrupt sence address
	int_ctrl_off		:		0,						// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{0x00},					// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-16/16RY(PCI)
	board_name			:		"PIO-16/16RY(PCI)",		// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x91b2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff },			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-16/16B(PCI)H
	board_name			:		"PIO-16/16B(PCI)H",		// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x91C2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff },			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PI-32B(PCI)H
	board_name			:		"PI-32B(PCI)H",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x91D2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		0,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff,0xff, 0xff },	// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		0,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PO-32B(PCI)H
	board_name			:		"PO-32B(PCI)H",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x91E2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{ 0x00 },				// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-16/16L(LPCI)H
	board_name			:		"PIO-16/16L(LPCI)H",	// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xA102,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff },			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-16/16B(LPCI)H
	board_name			:		"PIO-16/16B(LPCI)H",	// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xA112,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff },			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-16/16T(LPCI)H
	board_name			:		"PIO-16/16T(LPCI)H",	// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xA122,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff },			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-32/32H(PCI)H
	board_name			:		"PIO-32/32H(PCI)H",	// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xA142,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		4,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff, 0xff, 0xff},// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-16/16H(PCI)H
	board_name			:		"PIO-16/16H(PCI)H",		// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xA132,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff },			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-32/32RL(PCI)H
	board_name			:		"PIO-32/32RL(PCI)H",	// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xA162,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		4,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff, 0xff, 0xff},// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-16/16RL(PCI)H
	board_name			:		"PIO-16/16RL(PCI)H",	// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xA152,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff },			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-32/32T(PCI)H
	board_name			:		"PIO-32/32T(PCI)H",		// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xA172,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		4,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff, 0xff, 0xff},// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-32/32B(PCI)V
	board_name			:		"PIO-32/32B(PCI)V",		// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xA182,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		4,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff, 0xff, 0xff},// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// RRY-16C(PCI)H
	board_name			:		"RRY-16C(PCI)H",		// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xA192,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0x00},				// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// RRY-32(PCI)H
	board_name			:		"RRY-32(PCI)H",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xA1A2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0x00},				// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-32/32F(PCI)H
	board_name			:		"PIO-32/32F(PCI)H",		// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xA1F2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		4,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff, 0xff, 0xff},// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-16/16T(PCI)H
	board_name			:		"PIO-16/16T(PCI)H",		// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xB102,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff},			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-16/16TB(PCI)H
	board_name			:		"PIO-16/16TB(PCI)H",	// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xB112,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff},			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-1616T-LPE
	board_name			:		"DIO-1616T-LPE",		// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8612,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff},			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-3232L-PE
	board_name			:		"DIO-3232L-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8622,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		4,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff, 0xff, 0xff},// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-1616L-PE
	board_name			:		"DIO-1616L-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8632,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff},			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-1616L-LPE
	board_name			:		"DIO-1616L-LPE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8632,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff},			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-16/16L(CB)H
	board_name			:		"PIO-16/16L(CB)H",		// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8502,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff },			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-3232T-PE
	board_name			:		"DIO-3232T-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8642,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		4,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff, 0xff, 0xff},// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-1616T-PE
	board_name			:		"DIO-1616T-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8652,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff},			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-3232B-PE
	board_name			:		"DIO-3232B-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8662,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		4,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff, 0xff, 0xff},// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-3232F-PE
	board_name			:		"DIO-3232F-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x86F2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		4,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff, 0xff, 0xff},// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-1616B-PE
	board_name			:		"DIO-1616B-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8672,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff },			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-1616B-LPE
	board_name			:		"DIO-1616B-LPE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8672,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff },			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-1616TB-PE
	board_name			:		"DIO-1616TB-PE",		// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x9602,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff },			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DI-64L-PE
	board_name			:		"DI-64L-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x86B2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		8,						// Number of input ports
	out_port_num		:		0,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff, 0xff, 0xff},// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		0,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DI-32L-PE
	board_name			:		"DI-32L-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x86D2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		0,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff, 0xff, 0xff},// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		0,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DO-64L-PE
	board_name			:		"DO-64L-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x86C2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		0,						// Offset of filter address
	int_mask_off		:		0,						// Offset of interrupt mask address
	int_sence_off		:		0,						// Offset of interrupt sence address
	int_ctrl_off		:		0,						// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		8,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{ 0x00},				// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DO-32L-PE
	board_name			:		"DO-32L-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x86E2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		0,						// Offset of filter address
	int_mask_off		:		0,						// Offset of interrupt mask address
	int_sence_off		:		0,						// Offset of interrupt sence address
	int_ctrl_off		:		0,						// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{0x00},					// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DI-32T2-PCI
	board_name			:		"DI-32T2-PCI",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xB162,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		0,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff, 0xff, 0xff},// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		0,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DO-32T2-PCI
	board_name			:		"DO-32T2-PCI",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xB172,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		0,						// Offset of filter address
	int_mask_off		:		0,						// Offset of interrupt mask address
	int_sence_off		:		0,						// Offset of interrupt sence address
	int_ctrl_off		:		0,						// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{0x00},					// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DI-64T2-PCI
	board_name			:		"DI-64T2-PCI",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xB182,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		8,						// Number of input ports
	out_port_num		:		0,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff, 0xff, 0xff},// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		0,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DO-64T2-PCI
	board_name			:		"DO-64T2-PCI",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xB192,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		0,						// Offset of filter address
	int_mask_off		:		0,						// Offset of interrupt mask address
	int_sence_off		:		0,						// Offset of interrupt sence address
	int_ctrl_off		:		0,						// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		8,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{ 0x00},				// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DI-32T-PE
	board_name			:		"DI-32T-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x9642,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		0,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff, 0xff, 0xff},// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		0,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DO-32T-PE
	board_name			:		"DO-32T-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x9652,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		0,						// Offset of filter address
	int_mask_off		:		0,						// Offset of interrupt mask address
	int_sence_off		:		0,						// Offset of interrupt sence address
	int_ctrl_off		:		0,						// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{0x00},					// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DI-64T-PE
	board_name			:		"DI-64T-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x9622,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		8,						// Number of input ports
	out_port_num		:		0,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff, 0xff, 0xff},// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		0,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DO-64T-PE
	board_name			:		"DO-64T-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x9632,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		0,						// Offset of filter address
	int_mask_off		:		0,						// Offset of interrupt mask address
	int_sence_off		:		0,						// Offset of interrupt sence address
	int_ctrl_off		:		0,						// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		8,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{ 0x00},				// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DI-32B-PE
	board_name			:		"DI-32B-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x9662,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		0,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff, 0xff, 0xff},// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		0,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DO-32B-PE
	board_name			:		"DO-32B-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x9672,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		0,						// Offset of filter address
	int_mask_off		:		0,						// Offset of interrupt mask address
	int_sence_off		:		0,						// Offset of interrupt sence address
	int_ctrl_off		:		0,						// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{0x00},					// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-1616H-PE
	board_name			:		"DIO-1616H-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x9682,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff },			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-3232H-PE
	board_name			:		"DIO-3232H-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x9692,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		4,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff, 0xff, 0xff},// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-1616RL-PE
	board_name			:		"DIO-1616RL-PE",		// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x96A2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff },			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-3232RL-PE
	board_name			:		"DIO-3232RL-PE",		// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x96B2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		4,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff, 0xff, 0xff},// Interrupt bit (valid bit is 1)
	max_int_bit			:		32,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
// ASIC
	// PIO
	{// PIO-32/32L(PCI)
	board_name			:		"PIO-32/32L(PCI)",		// Board name
	board_type			:		CDIOBI_BT_PCI_ASIC,		// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8122,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		4,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_ASIC_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_ASIC_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_ASIC_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_ASIC_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0x0f, 0},				// Interrupt bit (valid bit is 1)
	max_int_bit			:		4,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-32/32T(PCI)
	board_name			:		"PIO-32/32T(PCI)",		// Board name
	board_type			:		CDIOBI_BT_PCI_ASIC,		// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8152,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		4,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_ASIC_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_ASIC_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_ASIC_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_ASIC_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0x0f, 0},				// Interrupt bit (valid bit is 1)
	max_int_bit			:		4,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-32/32F(PCI)
	board_name			:		"PIO-32/32F(PCI)",		// Board name
	board_type			:		CDIOBI_BT_PCI_ASIC,		// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x81A2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		4,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_ASIC_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_ASIC_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_ASIC_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_ASIC_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0x0f, 0},				// Interrupt bit (valid bit is 1)
	max_int_bit			:		4,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-32/32B(PCI)H
	board_name			:		"PIO-32/32B(PCI)H",		// Board name
	board_type			:		CDIOBI_BT_PCI_ASIC,		// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8112,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		4,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_ASIC_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_ASIC_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_ASIC_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_ASIC_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0x0f, 0},				// Interrupt bit (valid bit is 1)
	max_int_bit			:		4,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-16/16L(PCI)
	board_name			:		"PIO-16/16L(PCI)",		// Board name
	board_type			:		CDIOBI_BT_PCI_ASIC,		// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8172,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_ASIC_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_ASIC_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_ASIC_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_ASIC_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0x0f, 0},				// Interrupt bit (valid bit is 1)
	max_int_bit			:		4,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-16/16T(PCI)
	board_name			:		"PIO-16/16T(PCI)",		// Board name
	board_type			:		CDIOBI_BT_PCI_ASIC,		// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8162,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_ASIC_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_ASIC_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_ASIC_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_ASIC_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0x0f, 0},				// Interrupt bit (valid bit is 1)
	max_int_bit			:		4,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-16/16TB(PCI)
	board_name			:		"PIO-16/16TB(PCI)",		// Board name
	board_type			:		CDIOBI_BT_PCI_ASIC,		// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8192,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_ASIC_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_ASIC_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_ASIC_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_ASIC_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0x0f, 0},				// Interrupt bit (valid bit is 1)
	max_int_bit			:		4,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-16/16B(PCI)
	board_name			:		"PIO-16/16B(PCI)",		// Board name
	board_type			:		CDIOBI_BT_PCI_ASIC,		// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x81D2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_ASIC_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_ASIC_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_ASIC_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_ASIC_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0x0f, 0},				// Interrupt bit (valid bit is 1)
	max_int_bit			:		4,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	// PI
	{// PI-64L(PCI)
	board_name			:		"PI-64L(PCI)",			// Board name
	board_type			:		CDIOBI_BT_PCI_ASIC,		// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8132,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_ASIC_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_ASIC_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_ASIC_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_ASIC_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		8,						// Number of input ports
	out_port_num		:		0,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0x0f, 0},				// Interrupt bit (valid bit is 1)
	max_int_bit			:		4,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		0,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PI-32L(PCI)
	board_name			:		"PI-32L(PCI)",			// Board name
	board_type			:		CDIOBI_BT_PCI_ASIC,		// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x81E2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_ASIC_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_ASIC_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_ASIC_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_ASIC_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		0,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0x0f, 0},				// Interrupt bit (valid bit is 1)
	max_int_bit			:		4,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		0,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PI-32B(PCI)
	board_name			:		"PI-32B(PCI)",			// Board name
	board_type			:		CDIOBI_BT_PCI_ASIC,		// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x81F2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_ASIC_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_ASIC_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_ASIC_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_ASIC_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		0,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0x0f, 0},				// Interrupt bit (valid bit is 1)
	max_int_bit			:		4,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		0,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	// PO
	{// PO-64L(PCI)
	board_name			:		"PO-64L(PCI)",			// Board name
	board_type			:		CDIOBI_BT_PCI_ASIC,		// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8142,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		0,						// Offset of filter address
	int_mask_off		:		0,						// Offset of interrupt mask address
	int_sence_off		:		0,						// Offset of interrupt sence address
	int_ctrl_off		:		0,						// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		8,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{0},					// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PO-32L(PCI)
	board_name			:		"PO-32L(PCI)",			// Board name
	board_type			:		CDIOBI_BT_PCI_ASIC,		// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x9102,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		0,						// Offset of filter address
	int_mask_off		:		0,						// Offset of interrupt mask address
	int_sence_off		:		0,						// Offset of interrupt sence address
	int_ctrl_off		:		0,						// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{0},					// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PO-32B(PCI)
	board_name			:		"PO-32B(PCI)",			// Board name
	board_type			:		CDIOBI_BT_PCI_ASIC,		// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x9112,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		0,						// Offset of filter address
	int_mask_off		:		0,						// Offset of interrupt mask address
	int_sence_off		:		0,						// Offset of interrupt sence address
	int_ctrl_off		:		0,						// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{0},					// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
// Compact PCI
	{// PIO-32/32L(CPCI)
	board_name			:		"PIO-32/32L(CPCI)",		// Board name
	board_type			:		CDIOBI_BT_PCI_ASIC,		// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8202,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		4,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_ASIC_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_ASIC_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_ASIC_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_ASIC_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		4,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0x0f, 0},				// Interrupt bit (valid bit is 1)
	max_int_bit			:		4,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PI-64L(CPCI)
	board_name			:		"PI-64L(CPCI)",			// Board name
	board_type			:		CDIOBI_BT_PCI_ASIC,		// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8212,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_ASIC_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_ASIC_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_ASIC_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_ASIC_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		8,						// Number of input ports
	out_port_num		:		0,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0x0f, 0},				// Interrupt bit (valid bit is 1)
	max_int_bit			:		4,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		0,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PO-64L(CPCI)
	board_name			:		"PO-64L(CPCI)",			// Board name
	board_type			:		CDIOBI_BT_PCI_ASIC,		// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8222,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		0,						// Offset of filter address
	int_mask_off		:		0,						// Offset of interrupt mask address
	int_sence_off		:		0,						// Offset of interrupt sence address
	int_ctrl_off		:		0,						// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		8,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{0},					// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
// Relay
	{// DIO-1616RY-PE
	board_name			:		"DIO-1616RY-PE",		// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x96E2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		2,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_H_FILTER,	// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_H_INTMASK,	// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_H_INTSTS,	// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_H_INTCTRL,	// Offset of interrupt control address
	inp_port_num		:		2,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{ 0xff, 0xff },			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// RRY-16C-PE
	board_name			:		"RRY-16C-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x96D2,					// Device ID
	port_num			:		4,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		0,						// Offset of filter address
	int_mask_off		:		0,						// Offset of interrupt mask address
	int_sence_off		:		0,						// Offset of interrupt sence address
	int_ctrl_off		:		0,						// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{0},					// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// RRY-32-PE
	board_name			:		"RRY-32-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_H_SERIES,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x96C2,					// Device ID
	port_num			:		4,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		0,						// Offset of filter address
	int_mask_off		:		0,						// Offset of interrupt mask address
	int_sence_off		:		0,						// Offset of interrupt sence address
	int_ctrl_off		:		0,						// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{0},					// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// RRY-16C(PCI)
	board_name			:		"RRY-16C(PCI)",			// Board name
	board_type			:		CDIOBI_BT_PCI_RELAY,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x81B2,					// Device ID
	port_num			:		4,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		0,						// Offset of filter address
	int_mask_off		:		0,						// Offset of interrupt mask address
	int_sence_off		:		0,						// Offset of interrupt sence address
	int_ctrl_off		:		0,						// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		2,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{0},					// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// RRY-32(PCI)
	board_name			:		"RRY-32(PCI)",			// Board name
	board_type			:		CDIOBI_BT_PCI_RELAY,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8182,					// Device ID
	port_num			:		4,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		0,						// Offset of filter address
	int_mask_off		:		0,						// Offset of interrupt mask address
	int_sence_off		:		0,						// Offset of interrupt sence address
	int_ctrl_off		:		0,						// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		4,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{0},					// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
// Multipoint
	{// PIO-64/64L(PCI)
	board_name			:		"PIO-64/64L(PCI)",		// Board name
	board_type			:		CDIOBI_BT_PCI_TATEN,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x9122,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		8,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_TATEN_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_TATEN_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_TATEN_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_TATEN_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		8,						// Number of input ports
	out_port_num		:		8,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{0xff,0xff,0},			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PI-128L(PCI)
	board_name			:		"PI-128L(PCI)",			// Board name
	board_type			:		CDIOBI_BT_PCI_TATEN,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x9132,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_TATEN_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_TATEN_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_TATEN_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_TATEN_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		16,						// Number of input ports
	out_port_num		:		0,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{0xff,0xff,0},			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		0,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PO-128L(PCI)
	board_name			:		"PO-128L(PCI)",			// Board name
	board_type			:		CDIOBI_BT_PCI_TATEN,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x9142,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		0,						// Offset of filter address
	int_mask_off		:		0,						// Offset of interrupt mask address
	int_sence_off		:		0,						// Offset of interrupt sence address
	int_ctrl_off		:		0,						// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		16,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{0},					// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-64/64L(PCI)H
	board_name			:		"PIO-64/64L(PCI)H",		// Board name
	board_type			:		CDIOBI_BT_PCI_TATEN,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xA1C2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		8,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_TATEN_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_TATEN_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_TATEN_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_TATEN_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		8,						// Number of input ports
	out_port_num		:		8,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{0xff,0xff,0},			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PI-128L(PCI)H
	board_name			:		"PI-128L(PCI)H",		// Board name
	board_type			:		CDIOBI_BT_PCI_TATEN,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xA1D2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_TATEN_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_TATEN_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_TATEN_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_TATEN_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		16,						// Number of input ports
	out_port_num		:		0,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{0xff,0xff,0},			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		0,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PO-128L(PCI)H
	board_name			:		"PO-128L(PCI)H",		// Board name
	board_type			:		CDIOBI_BT_PCI_TATEN,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xA1E2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		0,						// Offset of filter address
	int_mask_off		:		0,						// Offset of interrupt mask address
	int_sence_off		:		0,						// Offset of interrupt sence address
	int_ctrl_off		:		0,						// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		16,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{0},					// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-6464T-PE
	board_name			:		"DIO-6464T-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_TATEN,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x9612,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		8,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_TATEN_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_TATEN_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_TATEN_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_TATEN_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		8,						// Number of input ports
	out_port_num		:		8,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{0xff,0xff,0},			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-6464L-PE
	board_name			:		"DIO-6464L-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_TATEN,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8682,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		8,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_TATEN_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_TATEN_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_TATEN_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_TATEN_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		8,						// Number of input ports
	out_port_num		:		8,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{0xff,0xff,0},			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DI-128L-PE
	board_name			:		"DI-128L-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_TATEN,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x8692,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_TATEN_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_TATEN_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_TATEN_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_TATEN_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		16,						// Number of input ports
	out_port_num		:		0,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{0xff,0xff,0},			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		0,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DO-128L-PE
	board_name			:		"DO-128L-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_TATEN,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0x86A2,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		0,						// Offset of filter address
	int_mask_off		:		0,						// Offset of interrupt mask address
	int_sence_off		:		0,						// Offset of interrupt sence address
	int_ctrl_off		:		0,						// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		16,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{0},					// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-6464T2-PCI
	board_name			:		"DIO-6464T2-PCI",		// Board name
	board_type			:		CDIOBI_BT_PCI_TATEN,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xB122,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		8,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_TATEN_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_TATEN_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_TATEN_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_TATEN_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		8,						// Number of input ports
	out_port_num		:		8,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{0xff,0xff,0},			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DI-128T2-PCI
	board_name			:		"DI-128T2-PCI",			// Board name
	board_type			:		CDIOBI_BT_PCI_TATEN,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xB132,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_TATEN_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_TATEN_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_TATEN_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_TATEN_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		16,						// Number of input ports
	out_port_num		:		0,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{0xff,0xff,0},			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		0,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DO-128T2-PCI
	board_name			:		"DO-128T2-PCI",			// Board name
	board_type			:		CDIOBI_BT_PCI_TATEN,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xB142,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		0,						// Offset of filter address
	int_mask_off		:		0,						// Offset of interrupt mask address
	int_sence_off		:		0,						// Offset of interrupt sence address
	int_ctrl_off		:		0,						// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		16,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{0},					// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DI-128T-PE
	board_name			:		"DI-128T-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_TATEN,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xA612,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		CDIOBI_PORT_TATEN_FILTER,// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_TATEN_INTMASK,// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_TATEN_INTSTS,// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_TATEN_INTCTRL,// Offset of interrupt control address
	inp_port_num		:		16,						// Number of input ports
	out_port_num		:		0,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		20,						// Maximum value of filter
	int_bit				:		{0xff,0xff,0},			// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		0,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DO-128T-PE
	board_name			:		"DO-128T-PE",			// Board name
	board_type			:		CDIOBI_BT_PCI_TATEN,	// Board type
	device_type			:		DEVICE_TYPE_PCI,		// PCI bus
	vendor_id			:		0x1221,					// Vendor ID
	device_id			:		0xA622,					// Device ID
	port_num			:		32,						// Number of occupied ports
	inp_port_off		:		0,						// Offset of input port address
	out_port_off		:		0,						// Offset of output port address
	filter_off			:		0,						// Offset of filter address
	int_mask_off		:		0,						// Offset of interrupt mask address
	int_sence_off		:		0,						// Offset of interrupt sence address
	int_ctrl_off		:		0,						// Offset of interrupt control address
	inp_port_num		:		0,						// Number of input ports
	out_port_num		:		16,						// Number of output ports
	min_filter_value	:		0,						// Minimum value of filter
	max_filter_value	:		0,						// Maximum value of filter
	int_bit				:		{0},					// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,						// Maximum number of interrupt bits
	num_of_8255			:		0,						// Number of 8255 chips
	can_echo_back		:		1,						// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0						// Positive logic? (0: Negative logic, 1: Positive logic)
	},
// 8255
	{// PIO-48D(PCI)
	board_name			:		"PIO-48D(PCI)",						// Board name
	board_type			:		CDIOBI_BT_PCI_8255,					// Board type
	device_type			:		DEVICE_TYPE_PCI,					// PCI bus
	vendor_id			:		0x1221,								// Vendor ID
	device_id			:		0x81C2,								// Device ID
	port_num			:		32,									// Number of occupied ports
	inp_port_off		:		0,									// Offset of input port address
	out_port_off		:		0,									// Offset of output port address
	filter_off			:		0,									// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_8255_INTMASK,			// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_8255_INTSTS,			// Offset of interrupt sence address
	int_ctrl_off		:		0,									// Offset of interrupt control address
	inp_port_num		:		6,									// Number of input ports
	out_port_num		:		6,									// Number of output ports
	min_filter_value	:		0,									// Minimum value of filter
	max_filter_value	:		0,									// Maximum value of filter
	int_bit				:		{0xff,0xff,0xff,0xff,0xff,0xff},	// Interrupt bit (valid bit is 1)
	max_int_bit			:		48,									// Maximum number of interrupt bits
	num_of_8255			:		2,									// Number of 8255 chips
	can_echo_back		:		1,									// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		1									// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-48D(CB)H
	board_name			:		"PIO-48D(CB)H",						// Board name
	board_type			:		CDIOBI_BT_PCI_8255,					// Board type
	device_type			:		DEVICE_TYPE_PCI,					// PCI bus
	vendor_id			:		0x1221,								// Vendor ID
	device_id			:		0x8512,								// Device ID
	port_num			:		32,									// Number of occupied ports
	inp_port_off		:		0,									// Offset of input port address
	out_port_off		:		0,									// Offset of output port address
	filter_off			:		CDIOBI_PORT_8255_H_FILTER,			// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_8255_INTMASK,			// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_8255_INTSTS,			// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_8255_H_INTCTRL,			// Offset of interrupt control address
	inp_port_num		:		6,									// Number of input ports
	out_port_num		:		6,									// Number of output ports
	min_filter_value	:		0,									// Minimum value of filter
	max_filter_value	:		20,									// Maximum value of filter
	int_bit				:		{0xff,0xff,0xff,0xff,0xff,0xff},	// Interrupt bit (valid bit is 1)
	max_int_bit			:		48,									// Maximum number of interrupt bits
	num_of_8255			:		2,									// Number of 8255 chips
	can_echo_back		:		1,									// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		1									// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-48D(LPCI)H
	board_name			:		"PIO-48D(LPCI)H",					// Board name
	board_type			:		CDIOBI_BT_PCI_8255,					// Board type
	device_type			:		DEVICE_TYPE_PCI,					// PCI bus
	vendor_id			:		0x1221,								// Vendor ID
	device_id			:		0xA1B2,								// Device ID
	port_num			:		32,									// Number of occupied ports
	inp_port_off		:		0,									// Offset of input port address
	out_port_off		:		0,									// Offset of output port address
	filter_off			:		CDIOBI_PORT_8255_H_FILTER,			// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_8255_INTMASK,			// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_8255_INTSTS,			// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_8255_H_INTCTRL,			// Offset of interrupt control address
	inp_port_num		:		6,									// Number of input ports
	out_port_num		:		6,									// Number of output ports
	min_filter_value	:		0,									// Minimum value of filter
	max_filter_value	:		20,									// Maximum value of filter
	int_bit				:		{0xff,0xff,0xff,0xff,0xff,0xff},	// Interrupt bit (valid bit is 1)
	max_int_bit			:		48,									// Maximum number of interrupt bits
	num_of_8255			:		2,									// Number of 8255 chips
	can_echo_back		:		1,									// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		1									// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-48D-LPE
	board_name			:		"DIO-48D-LPE",						// Board name
	board_type			:		CDIOBI_BT_PCI_8255,					// Board type
	device_type			:		DEVICE_TYPE_PCI,					// PCI bus
	vendor_id			:		0x1221,								// Vendor ID
	device_id			:		0x8602,								// Device ID
	port_num			:		32,									// Number of occupied ports
	inp_port_off		:		0,									// Offset of input port address
	out_port_off		:		0,									// Offset of output port address
	filter_off			:		CDIOBI_PORT_8255_H_FILTER,			// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_8255_INTMASK,			// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_8255_INTSTS,			// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_8255_H_INTCTRL,			// Offset of interrupt control address
	inp_port_num		:		6,									// Number of input ports
	out_port_num		:		6,									// Number of output ports
	min_filter_value	:		0,									// Minimum value of filter
	max_filter_value	:		20,									// Maximum value of filter
	int_bit				:		{0xff,0xff,0xff,0xff,0xff,0xff},	// Interrupt bit (valid bit is 1)
	max_int_bit			:		48,									// Maximum number of interrupt bits
	num_of_8255			:		2,									// Number of 8255 chips
	can_echo_back		:		1,									// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		1									// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-48D-PE
	board_name			:		"DIO-48D-PE",						// Board name
	board_type			:		CDIOBI_BT_PCI_8255,					// Board type
	device_type			:		DEVICE_TYPE_PCI,					// PCI bus
	vendor_id			:		0x1221,								// Vendor ID
	device_id			:		0x96F2,								// Device ID
	port_num			:		32,									// Number of occupied ports
	inp_port_off		:		0,									// Offset of input port address
	out_port_off		:		0,									// Offset of output port address
	filter_off			:		CDIOBI_PORT_8255_H_FILTER,			// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_8255_INTMASK,			// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_8255_INTSTS,			// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_8255_H_INTCTRL,			// Offset of interrupt control address
	inp_port_num		:		6,									// Number of input ports
	out_port_num		:		6,									// Number of output ports
	min_filter_value	:		0,									// Minimum value of filter
	max_filter_value	:		20,									// Maximum value of filter
	int_bit				:		{0xff,0xff,0xff,0xff,0xff,0xff},	// Interrupt bit (valid bit is 1)
	max_int_bit			:		48,									// Maximum number of interrupt bits
	num_of_8255			:		2,									// Number of 8255 chips
	can_echo_back		:		1,									// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		1									// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-48D2-PCI
	board_name			:		"DIO-48D2-PCI",						// Board name
	board_type			:		CDIOBI_BT_PCI_8255,					// Board type
	device_type			:		DEVICE_TYPE_PCI,					// PCI bus
	vendor_id			:		0x1221,								// Vendor ID
	device_id			:		0x81C2,								// Device ID
	port_num			:		32,									// Number of occupied ports
	inp_port_off		:		0,									// Offset of input port address
	out_port_off		:		0,									// Offset of output port address
	filter_off			:		CDIOBI_PORT_8255_H_FILTER,			// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_8255_INTMASK,			// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_8255_INTSTS,			// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_8255_H_INTCTRL,			// Offset of interrupt control address
	inp_port_num		:		6,									// Number of input ports
	out_port_num		:		6,									// Number of output ports
	min_filter_value	:		0,									// Minimum value of filter
	max_filter_value	:		20,									// Maximum value of filter
	int_bit				:		{0xff,0xff,0xff,0xff,0xff,0xff},	// Interrupt bit (valid bit is 1)
	max_int_bit			:		48,									// Maximum number of interrupt bits
	num_of_8255			:		2,									// Number of 8255 chips
	can_echo_back		:		1,									// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		1									// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-96D2-LPCI
	board_name			:		"DIO-96D2-LPCI",					// Board name
	board_type			:		CDIOBI_BT_PCI_8255,					// Board type
	device_type			:		DEVICE_TYPE_PCI,					// PCI bus
	vendor_id			:		0x1221,								// Vendor ID
	device_id			:		0xB152,								// Device ID
	port_num			:		128,								// Number of occupied ports
	inp_port_off		:		0,									// Offset of input port address
	out_port_off		:		0,									// Offset of output port address
	filter_off			:		CDIOBI_PORT_8255_T_FILTER,			// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_8255_T_INTMASK,			// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_8255_T_INTSTS,			// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_8255_T_INTCTRL,			// Offset of interrupt control address
	inp_port_num		:		12,									// Number of input ports
	out_port_num		:		12,									// Number of output ports
	min_filter_value	:		0,									// Minimum value of filter
	max_filter_value	:		20,									// Maximum value of filter
	int_bit				:		{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},	// Interrupt bit (valid bit is 1)
	max_int_bit			:		96,									// Maximum number of interrupt bits
	num_of_8255			:		4,									// Number of 8255 chips
	can_echo_back		:		1,									// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		1									// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-96D-LPE
	board_name			:		"DIO-96D-LPE",					// Board name
	board_type			:		CDIOBI_BT_PCI_8255,					// Board type
	device_type			:		DEVICE_TYPE_PCI,					// PCI bus
	vendor_id			:		0x1221,								// Vendor ID
	device_id			:		0xA602,								// Device ID
	port_num			:		128,								// Number of occupied ports
	inp_port_off		:		0,									// Offset of input port address
	out_port_off		:		0,									// Offset of output port address
	filter_off			:		CDIOBI_PORT_8255_T_FILTER,			// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_8255_T_INTMASK,			// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_8255_T_INTSTS,			// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_8255_T_INTCTRL,			// Offset of interrupt control address
	inp_port_num		:		12,									// Number of input ports
	out_port_num		:		12,									// Number of output ports
	min_filter_value	:		0,									// Minimum value of filter
	max_filter_value	:		20,									// Maximum value of filter
	int_bit				:		{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},	// Interrupt bit (valid bit is 1)
	max_int_bit			:		96,									// Maximum number of interrupt bits
	num_of_8255			:		4,									// Number of 8255 chips
	can_echo_back		:		1,									// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		1									// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// PIO-32DM(PCI)
	board_name			:		"PIO-32DM(PCI)",					// Board name
	board_type			:		CDIOBI_BT_PCI_DM,					// Board type
	device_type			:		DEVICE_TYPE_PCI,					// PCI bus
	vendor_id			:		0x1221,								// Vendor ID
	device_id			:		0xC102,								// Device ID
	port_num			:		32,									// Number of occupied ports
	inp_port_off		:		0,									// Offset of input port address
	out_port_off		:		0,									// Offset of output port address
	filter_off			:		0,									// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_DM_INTMASK,				// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_DM_INTSTS,				// Offset of interrupt sence address
	int_ctrl_off		:		0,									// Offset of interrupt control address
	inp_port_num		:		4,									// Number of input ports
	out_port_num		:		4,									// Number of output ports
	min_filter_value	:		0,									// Minimum value of filter
	max_filter_value	:		0,									// Maximum value of filter
	int_bit				:		{0x0f,0},							// Interrupt bit (valid bit is 1)
	max_int_bit			:		4,									// Maximum number of interrupt bits
	num_of_8255			:		0,									// Number of 8255 chips
	can_echo_back		:		1,									// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		1									// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DIO-32DM-PE
	board_name			:		"DIO-32DM-PE",						// Board name
	board_type			:		CDIOBI_BT_PCI_DM,					// Board type
	device_type			:		DEVICE_TYPE_PCI,					// PCI bus
	vendor_id			:		0x1221,								// Vendor ID
	device_id			:		0xA632,								// Device ID
	port_num			:		32,									// Number of occupied ports
	inp_port_off		:		0,									// Offset of input port address
	out_port_off		:		0,									// Offset of output port address
	filter_off			:		0,									// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_DM_INTMASK,				// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_DM_INTSTS,				// Offset of interrupt sence address
	int_ctrl_off		:		0,									// Offset of interrupt control address
	inp_port_num		:		4,									// Number of input ports
	out_port_num		:		4,									// Number of output ports
	min_filter_value	:		0,									// Minimum value of filter
	max_filter_value	:		0,									// Maximum value of filter
	int_bit				:		{0x0f,0},							// Interrupt bit (valid bit is 1)
	max_int_bit			:		4,									// Maximum number of interrupt bits
	num_of_8255			:		0,									// Number of 8255 chips
	can_echo_back		:		1,									// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		1									// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DI-128RL-PCI
	board_name			:		"DI-128RL-PCI",						// Board name
	board_type			:		CDIOBI_BT_PCI_TATEN,				// Board type
	device_type			:		DEVICE_TYPE_PCI,					// PCI bus
	vendor_id			:		0x1221,								// Vendor ID
	device_id			:		0xB1A2,								// Device ID
	port_num			:		32,									// Number of occupied ports
	inp_port_off		:		0,									// Offset of input port address
	out_port_off		:		0,									// Offset of output port address
	filter_off			:		CDIOBI_PORT_TATEN_FILTER,			// Offset of filter address
	int_mask_off		:		CDIOBI_PORT_TATEN_INTMASK,			// Offset of interrupt mask address
	int_sence_off		:		CDIOBI_PORT_TATEN_INTSTS,			// Offset of interrupt sence address
	int_ctrl_off		:		CDIOBI_PORT_TATEN_INTCTRL,			// Offset of interrupt control address
	inp_port_num		:		16,									// Number of input ports
	out_port_num		:		0,									// Number of output ports
	min_filter_value	:		0,									// Minimum value of filter
	max_filter_value	:		20,									// Maximum value of filter
	int_bit				:		{0xff,0xff,0},						// Interrupt bit (valid bit is 1)
	max_int_bit			:		16,									// Maximum number of interrupt bits
	num_of_8255			:		0,									// Number of 8255 chips
	can_echo_back		:		0,									// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		1									// Positive logic? (0: Negative logic, 1: Positive logic)
	},
	{// DO-128RL-PCI
	board_name			:		"DO-128RL-PCI",						// Board name
	board_type			:		CDIOBI_BT_PCI_TATEN,				// Board type
	device_type			:		DEVICE_TYPE_PCI,					// PCI bus
	vendor_id			:		0x1221,								// Vendor ID
	device_id			:		0xB1B2,								// Device ID
	port_num			:		32,									// Number of occupied ports
	inp_port_off		:		0,									// Offset of input port address
	out_port_off		:		0,									// Offset of output port address
	filter_off			:		0,									// Offset of filter address
	int_mask_off		:		0,									// Offset of interrupt mask address
	int_sence_off		:		0,									// Offset of interrupt sence address
	int_ctrl_off		:		0,									// Offset of interrupt control address
	inp_port_num		:		0,									// Number of input ports
	out_port_num		:		16,									// Number of output ports
	min_filter_value	:		0,									// Minimum value of filter
	max_filter_value	:		0,									// Maximum value of filter
	int_bit				:		{0},								// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,									// Maximum number of interrupt bits
	num_of_8255			:		0,									// Number of 8255 chips
	can_echo_back		:		1,									// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		1									// Positive logic? (0: Negative logic, 1: Positive logic)
	},
// End
	{// End
	board_name			:		"End",								// Board name
	board_type			:		0,									// Board type
	device_type			:		DEVICE_TYPE_PCI,					// PCI bus
	vendor_id			:		0,									// Vendor ID
	device_id			:		0,									// Device ID
	port_num			:		0,									// Number of occupied ports
	inp_port_off		:		0,									// Offset of input port address
	out_port_off		:		0,									// Offset of output port address
	filter_off			:		0,									// Offset of filter address
	int_mask_off		:		0,									// Offset of interrupt mask address
	int_sence_off		:		0,									// Offset of interrupt sence address
	int_ctrl_off		:		0,									// Offset of interrupt control address
	inp_port_num		:		0,									// Number of input ports
	out_port_num		:		0,									// Number of output ports
	min_filter_value	:		0,									// Minimum value of filter
	max_filter_value	:		0,									// Maximum value of filter
	int_bit				:		{0},								// Interrupt bit (valid bit is 1)
	max_int_bit			:		0,									// Maximum number of interrupt bits
	num_of_8255			:		0,									// Number of 8255 chips
	can_echo_back		:		0,									// Echo back available? (0: Unavailable, 1: Available)
	is_positive			:		0									// Positive logic? (0: Negative logic, 1: Positive logic)
	}
};

//================================================================
// Function
//================================================================
//================================================================
// BIOS initialization
//================================================================
long Cdiobi_init_bios(PCDIOBI bios_d, char *board_name, unsigned long *io_addr, unsigned long irq_no)
{
	int				i;
	unsigned short	port_base;

	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (bios_d == NULL ||
		board_name == NULL ||
		io_addr == NULL ||
		io_addr[0] == 0x0000) {
		return -1;
	}
	//----------------------------------------
	// Initialize the BIOS data
	//----------------------------------------
	memset(bios_d, 0, sizeof(CDIOBI));
	//----------------------------------------
	// Set the board information
	//----------------------------------------
	for (i=0; strcmp(board_info[i].board_name, "End") != 0; i++) {
		if (strcmp(board_info[i].board_name, board_name) == 0) {
			bios_d->b_info = &board_info[i];
			break;
		}
	}
	if (bios_d->b_info == NULL) {
		return -1;
	}
	//----------------------------------------
	// Store the interrupt number
	//----------------------------------------
	bios_d->irq_no = (unsigned short)(irq_no & 0xffff);
	//----------------------------------------
	// Embed the port
	//----------------------------------------
	port_base = (unsigned short)(io_addr[0] & 0xffff);
	bios_d->port_base = port_base;							// Base address of port
	// Input address
	if (bios_d->b_info->inp_port_num != 0) {
		bios_d->inp_port = port_base + bios_d->b_info->inp_port_off;
	} else {
		bios_d->inp_port = 0;
	}
	// Output address
	if (bios_d->b_info->out_port_num != 0) {
		bios_d->out_port = port_base + bios_d->b_info->out_port_off;
	} else {
		bios_d->out_port = 0;
	}
	// Filter address
	if (bios_d->b_info->filter_off != 0) {
		bios_d->filter_port = port_base + bios_d->b_info->filter_off;
	} else {
		bios_d->filter_port = 0;
	}
	// Interrupt mask address
	if (bios_d->b_info->int_mask_off != 0) {
		bios_d->int_mask_port = port_base + bios_d->b_info->int_mask_off;
	} else {
		bios_d->int_mask_port = 0;
	}
	// Interrupt sence address
	if (bios_d->b_info->int_sence_off != 0) {
		bios_d->int_sence_port = port_base + bios_d->b_info->int_sence_off;
	} else {
		bios_d->int_sence_port = 0;
	}
	// Interrupt control address
	if (bios_d->b_info->int_ctrl_off != 0) {
		bios_d->int_ctrl_port = port_base + bios_d->b_info->int_ctrl_off;
	} else {
		bios_d->int_ctrl_port = 0;
	}
	//----------------------------------------
	// Set the initialized flag
	//----------------------------------------
	bios_d->flag = 1;										// 0: Uninitialized, 1: Initialized
	//----------------------------------------
	// Mask the interrupt
	//----------------------------------------
	Cdiobi_set_all_int_mask(bios_d, CDIOBI_MASK_ALL, NULL);	// Interrupt mask data (0: Open, 1: Close)
	//----------------------------------------
	// Initialize the others work
	//----------------------------------------
	memset(bios_d->out_data, 0, sizeof(bios_d->out_data));	// Output latch data
	for (i=0; i<bios_d->b_info->out_port_num; i++) {		// Initialize by echo back
		Cdiobi_echo_port(bios_d, i, &bios_d->out_data[i]);
	}
	memset(bios_d->int_ctrl, 0, sizeof(bios_d->int_ctrl));	// Interrupt control data (0: Rising, 1: Falling)
	bios_d->filter_value = 0;								// Digital filter value
	Cdiobi_set_digital_filter(bios_d, 0);
	return 0;
}

//================================================================
// BIOS exit
//================================================================
void Cdiobi_clear_bios(PCDIOBI bios_d)
{
	//----------------------------------------
	// Initialize the BIOS data
	//----------------------------------------
	memset(bios_d, 0, sizeof(CDIOBI));
}

//================================================================
// Output latch data initialization
//================================================================
void Cdiobi_init_latch_data(PCDIOBI bios_d)
{
	int i;

	memset(bios_d->out_data, 0, sizeof(bios_d->out_data));	// Output latch data
	for (i=0; i<bios_d->b_info->out_port_num; i++) {		// Initialize according to the echo back
		Cdiobi_echo_port(bios_d, i, &bios_d->out_data[i]);
	}
}

//================================================================
// Port input
//================================================================
void Cdiobi_input_port(PCDIOBI bios_d, short port_no, unsigned char *data)
{
	unsigned long	dwdata;
	
	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (bios_d == NULL ||
		bios_d->flag == 0 ||
		bios_d->inp_port == 0 ||
		port_no < 0 ||
		data == NULL) {
		return;
	}
	//----------------------------------------
	// Check the port number
	//----------------------------------------
	if (Cdiobi_check_inp_port(bios_d, port_no) == 0) {
		return;
	}
	//----------------------------------------
	// Modify the 8255 port number
	//----------------------------------------
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_8255) {			// Board type: 8255
		Cdiobi_8255_port_set(bios_d, &port_no);
	}
	//----------------------------------------
	// Input
	//----------------------------------------
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_DM) {			// Board type: DM
		dwdata	= _inpd(bios_d->inp_port + ((port_no / 4) * 4));	// DWORD access
		*data	= dwdata >> ((port_no % 4) * 8);
	}else{
		*data	= _inp(bios_d->inp_port + port_no);
	}
}

//================================================================
// Bit input
//================================================================
void Cdiobi_input_bit(PCDIOBI bios_d, short bit_no, unsigned char *data)
{
	unsigned char	tmp;
	unsigned long	dwdata;
	short			port_no;
	
	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (bios_d == NULL ||
		bios_d->flag == 0 ||
		bios_d->inp_port == 0 ||
		bit_no < 0 ||
		data == NULL) {
		return;
	}
	//----------------------------------------
	// Check the bit number
	//----------------------------------------
	if (Cdiobi_check_inp_bit(bios_d, bit_no) == 0) {
		return;
	}
	port_no = bit_no / 8;
	//----------------------------------------
	// Modify the 8255 port number
	//----------------------------------------
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_8255) {			// Board type: 8255
		Cdiobi_8255_port_set(bios_d, &port_no);
	}
	//----------------------------------------
	// Input
	//----------------------------------------
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_DM) {			// Board type: DM
		dwdata	= _inpd(bios_d->inp_port + ((port_no / 4) * 4));	// DWORD access
		tmp		= dwdata >> ((port_no % 4) * 8);
	}else{
		tmp		= _inp(bios_d->inp_port + port_no);
	}
	*data	= (tmp >> (bit_no % 8)) & 0x01;
}

//================================================================
// Port output
//================================================================
void Cdiobi_output_port(PCDIOBI bios_d, short port_no, unsigned char data)
{
	unsigned long	dwdata;
	short			port_no_dw, i;

	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (bios_d == NULL ||
		bios_d->flag == 0 ||
		bios_d->out_port == 0 ||
		port_no < 0) {
		return;
	}
	//----------------------------------------
	// Check the port number
	//----------------------------------------
	if (Cdiobi_check_out_port(bios_d, port_no) == 0) {
		return;
	}
	//----------------------------------------
	// Save the output data
	//----------------------------------------
	bios_d->out_data[port_no] = data;
	//----------------------------------------
	// Modify the 8255 port number
	//----------------------------------------
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_8255) {	// Board type: 8255
		Cdiobi_8255_port_set(bios_d, &port_no);
	}
	//----------------------------------------
	// Output
	//----------------------------------------
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_DM) {	// Board type: DM
		// Create the output data
		dwdata		= 0;
		port_no_dw	= port_no / 4;	// What number of port in the DWORD unit
		for(i = 3; i >= 0; i--){
			dwdata	= (dwdata << 8) | (unsigned long)bios_d->out_data[port_no_dw * 4 + i];
		}
		_outpd(bios_d->out_port + (port_no_dw * 4), dwdata);
	}else{
		_outp(bios_d->out_port + port_no, data);
	}
}

//================================================================
// Bit output
//================================================================
void Cdiobi_output_bit(PCDIOBI bios_d, short bit_no, unsigned char data)
{
	unsigned char	tmp;
	unsigned long	dwdata;
	short			port_no, port_no_dw, i;
	
	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (bios_d == NULL ||
		bios_d->flag == 0 ||
		bios_d->out_port == 0 ||
		bit_no < 0) {
		return;
	}
	//----------------------------------------
	// Check the bit number
	//----------------------------------------
	if (Cdiobi_check_out_bit(bios_d, bit_no) == 0) {
		return;
	}
	//----------------------------------------
	// Create the output data
	//----------------------------------------
	port_no = bit_no / 8;
	if (data == 0) {
		tmp = bios_d->out_data[port_no] & ~(0x01 << (bit_no % 8));
	} else {
		tmp = bios_d->out_data[port_no] | (0x01 << (bit_no % 8));
	}
	//----------------------------------------
	// Save the output data
	//----------------------------------------
	bios_d->out_data[port_no] = tmp;
	//----------------------------------------
	// Modify the 8255 port number
	//----------------------------------------
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_8255) {	// Board type: 8255
		Cdiobi_8255_port_set(bios_d, &port_no);
	}
	//----------------------------------------
	// Output
	//----------------------------------------
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_DM) {	// Board type: DM
		// Create the output data
		dwdata		= 0;
		port_no_dw	= port_no / 4;	// What number of port in the DWORD unit
		for(i = 3; i >= 0; i--){
			dwdata	= (dwdata << 8) | (unsigned long)bios_d->out_data[port_no_dw * 4 + i];
		}
		_outpd(bios_d->out_port + (port_no_dw * 4), dwdata);
	}else{
		_outp(bios_d->out_port + port_no, tmp);
	}
}

//================================================================
// Port echo back
//================================================================
void Cdiobi_echo_port(PCDIOBI bios_d, short port_no, unsigned char *data)
{
	unsigned long	dwdata;

	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (bios_d == NULL ||
		bios_d->flag == 0 ||
		bios_d->out_port == 0 ||
		port_no < 0 ||
		data == NULL) {
		return;
	}
	//----------------------------------------
	// Check the port number
	//----------------------------------------
	if (Cdiobi_check_out_port(bios_d, port_no) == 0) {
		return;
	}
	//----------------------------------------
	// Echo back
	//----------------------------------------
	if (bios_d->b_info->can_echo_back == 0) {	// Echo back available? (0: Unavailable, 1: Available)
		*data = bios_d->out_data[port_no];
	} else {
		//----------------------------------------
		// Modify the 8255 port number
		//----------------------------------------
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_8255) {	// Board type: 8255
			Cdiobi_8255_port_set(bios_d, &port_no);
		}
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_DM) {			// Board type: DM
			dwdata	= _inpd(bios_d->out_port + ((port_no / 4) * 4));	// DWORD access
			*data	= dwdata >> ((port_no % 4) * 8);
		}else{
			*data	= _inp(bios_d->out_port + port_no);
		}
	}
}

//================================================================
// Bit echo back
//================================================================
void Cdiobi_echo_bit(PCDIOBI bios_d, short bit_no, unsigned char *data)
{
	unsigned char	tmp;
	short			port_no;
	unsigned long	dwdata;
	
	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (bios_d == NULL ||
		bios_d->flag == 0 ||
		bios_d->out_port == 0 ||
		bit_no < 0 ||
		data == NULL) {
		return;
	}
	//----------------------------------------
	// Check the bit number
	//----------------------------------------
	if (Cdiobi_check_out_bit(bios_d, bit_no) == 0) {
		return;
	}
	//----------------------------------------
	// Echo back
	//----------------------------------------
	port_no	= bit_no / 8;
	//----------------------------------------
	// Modify the 8255 port number
	//----------------------------------------
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_8255) {	// Board type: 8255
		Cdiobi_8255_port_set(bios_d, &port_no);
	}
	if (bios_d->b_info->can_echo_back == 0) {				// Echo back available? (0: Unavailable, 1: Available)
		tmp = bios_d->out_data[port_no];
	} else {
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_DM) {			// Board type: DM
			dwdata	= _inpd(bios_d->out_port + ((port_no / 4) * 4));	// DWORD access
			tmp		= dwdata >> ((port_no % 4) * 8);
		}else{
			tmp		= _inp(bios_d->out_port + port_no);
		}
	}
	*data = (tmp >> (bit_no % 8)) & 0x01;
}

//================================================================
// Digital filter setting
//================================================================
long Cdiobi_set_digital_filter(PCDIOBI bios_d, unsigned long filter_value)
{
	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (bios_d == NULL ||
		bios_d->flag == 0 ||
		bios_d->filter_port == 0) {
		return -1;		// DIO_ERR_SYS_NOT_SUPPORTED
	}
	if (filter_value < bios_d->b_info->min_filter_value ||
		filter_value > bios_d->b_info->max_filter_value) {
		return -2;		// DIO_ERR_SYS_FILTER
	}
	//----------------------------------------
	// Set the digiter filter
	//----------------------------------------
	bios_d->filter_value = filter_value;
	_outp(bios_d->filter_port, (unsigned char)(filter_value & 0xff));
	return 0;
}

//================================================================
// Retrieve the digital filter
//================================================================
long Cdiobi_get_digital_filter(PCDIOBI bios_d, unsigned long *filter_value)
{
	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (bios_d == NULL ||
		bios_d->flag == 0 ||
		bios_d->filter_port == 0 ||
		filter_value == NULL) {
		return -1;		// DIO_ERR_SYS_NOT_SUPPORTED
	}
	//----------------------------------------
	// Return the digital filter
	//----------------------------------------
	*filter_value = bios_d->filter_value;
	return 0;
}

//================================================================
// 8255 mode setting
//================================================================
long Cdiobi_set_8255_mode(PCDIOBI bios_d, unsigned short chip_no, unsigned short ctrl_word)
{
	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (bios_d == NULL ||
		bios_d->flag == 0 ||
		bios_d->b_info->num_of_8255 == 0) {
		return -1;		// DIO_ERR_SYS_NOT_SUPPORTED
	}
	if (chip_no > (bios_d->b_info->num_of_8255 - 1)){
		return -2;		// DIO_ERR_SYS_8255
	}
	//----------------------------------------
	// Set the control word
	//----------------------------------------
	bios_d->ctrl_word_8255[chip_no] = ctrl_word;
	_outp(bios_d->port_base + (chip_no * 4 + 3), ctrl_word);
	return 0;
}

//================================================================
// Retrieve the 8255 mode settting
//================================================================
long Cdiobi_get_8255_mode(PCDIOBI bios_d, unsigned short chip_no, unsigned short *ctrl_word)
{
	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (bios_d == NULL ||
		bios_d->flag == 0 ||
		bios_d->b_info->num_of_8255 == 0) {
		return -1;		// DIO_ERR_SYS_NOT_SUPPORTED
	}
	if (chip_no > (bios_d->b_info->num_of_8255 -1)) {
		return -2;		// DIO_ERR_SYS_8255
	}
	//----------------------------------------
	// Retrieve the control word setting
	//----------------------------------------
	*ctrl_word = bios_d->ctrl_word_8255[chip_no];

	return 0;
}

//================================================================
//	Calculate I/O ports of the board using 8255 chip
//	Because I/O ports on the port map are not arranged consecutively,
//	allow using the existing I/O functions by shifting the port number.
//================================================================
void Cdiobi_8255_port_set(PCDIOBI bios_d, unsigned short *pport_no)
{
	//------------------------------
	// Set the port number PIO-48D(PCI)
	//------------------------------
	*pport_no = *pport_no*4/3;
	return;
}

//================================================================
// Mask all interrupts
//================================================================
void Cdiobi_set_all_int_mask(PCDIOBI bios_d, int flag, unsigned char *mask)
{
	unsigned char	b_data;
	unsigned short	w_data;
	unsigned long	d_data;
	short			port_no, real_port_no;

	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (bios_d == NULL ||
		bios_d->flag == 0 ||
		bios_d->int_mask_port == 0) {
		return;
	}
	if (flag != CDIOBI_MASK_ALL &&
		flag != CDIOBI_MASK_DATA) {
		return;
	}
	if (flag == CDIOBI_MASK_DATA && mask == NULL) {
		return;
	}
	//----------------------------------------
	// In the case of masking all the interrupts
	//----------------------------------------
	if (flag == CDIOBI_MASK_ALL) {
		// 4 channels of the input signals as interrupt for ASIC
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_ASIC) {		// Board type: ASIC
			b_data = 0x0f;						// Mask 4 bits
			bios_d->int_mask[0] = b_data;
			_outp(bios_d->int_mask_port, b_data);
		}
		// 16 channels of the input signals as interrupt for Multipoint
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_TATEN) {		// Board type: Multipoint
			w_data = 0xffff;
			bios_d->int_mask[0] = 0xff;			// Mask 16 bits
			bios_d->int_mask[1] = 0xff;
			_outpw(bios_d->int_mask_port, w_data);
		}
		// 32 and 16 channels of the input signals as interrupt for H series
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_H_SERIES) {	// Board type: H series
			w_data = 0xffff;
			d_data = 0xffffffff;
			if (bios_d->b_info->inp_port_num == 2) {
				bios_d->int_mask[0] = 0xff;		// Mask 16 bits
				bios_d->int_mask[1] = 0xff;
				_outpw(bios_d->int_mask_port, w_data);
			} else {
				bios_d->int_mask[0] = 0xff;		// Mask 32 bits
				bios_d->int_mask[1] = 0xff;
				bios_d->int_mask[2] = 0xff;
				bios_d->int_mask[3] = 0xff;
				_outpd(bios_d->int_mask_port, d_data);
			}
		}
		// All channels of the input signals as interrupt for 8255
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_8255) {	// Board type: 8255
			b_data = 0xff;						// Mask 8 bits
			for(port_no = 0; port_no < bios_d->b_info->inp_port_num; port_no++){
				// Mask data
				bios_d->int_mask[port_no]	= b_data;
				// Mask
				real_port_no = port_no;
				Cdiobi_8255_port_set(bios_d, &real_port_no);
				_outp(bios_d->int_mask_port + real_port_no, b_data);
			}
		}
		// Mask all the interrupts of the local side for the DM board
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_DM) {	// Board type: DM
			d_data = 0xffffffff;
			bios_d->int_mask[0] = 0x0f;			// Mask 4 bits
			_outpd(bios_d->int_mask_port, d_data);
		}
	}
	//----------------------------------------
	// In the case of specified mask
	//----------------------------------------
	if (flag == CDIOBI_MASK_DATA) {
		// 4 channels of the input signals as interrupt for ASIC
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_ASIC) {	// Board type: ASIC
			b_data = mask[0];
			bios_d->int_mask[0] = b_data;
			_outp(bios_d->int_mask_port, b_data);
		}
		// 16 channels of the input signals as interrupt for Multipoint
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_TATEN) {	// Board type: Multipoint
			w_data = (mask[1] << 8) | (mask[0]);
			bios_d->int_mask[0] = mask[0];
			bios_d->int_mask[1] = mask[1];
			_outpw(bios_d->int_mask_port, w_data);
		}
		// 32 and 16 channels of the input signals as interrupt for H series
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_H_SERIES) {	// Board type: H series
			w_data = (mask[1] << 8) | (mask[0]);
			d_data = (mask[3] << 24) | (mask[2] << 16) | (mask[1] << 8) | (mask[0]);
			if (bios_d->b_info->inp_port_num == 2) {
				bios_d->int_mask[0] = mask[0];		// 1Mask 16 bits
				bios_d->int_mask[1] = mask[1];
				_outpw(bios_d->int_mask_port, w_data);
			} else {
				bios_d->int_mask[0] = mask[0];		// Mask 32 bits
				bios_d->int_mask[1] = mask[1];
				bios_d->int_mask[2] = mask[2];
				bios_d->int_mask[3] = mask[3];
				_outpd(bios_d->int_mask_port, d_data);
			}
		}
		// All channels of the input signals as interrupt for 8255
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_8255) {	// Board type: 8255
			for(port_no = 0; port_no < bios_d->b_info->inp_port_num; port_no++){
				// Mask data
				bios_d->int_mask[port_no]	= mask[port_no];
				// Mask
				real_port_no = port_no;
				Cdiobi_8255_port_set(bios_d, &real_port_no);
				_outp(bios_d->int_mask_port + real_port_no, mask[port_no]);
			}
		}
		// 4 channels of the input signals as interrupt for DM board.  Mask the interrupts expcept general-purpose input
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_DM) {	// Board type: DM
			d_data = 0xffff0fff | (mask[0] << 12);
			bios_d->int_mask[0] = mask[0];			// Mask 4 bits
			_outpd(bios_d->int_mask_port, d_data);
		}
	}
	return;
}

//================================================================
// Retrieve the interrupt mask
//================================================================
void Cdiobi_get_all_int_mask(PCDIOBI bios_d, unsigned char *mask)
{
	short	port_no;

	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (bios_d == NULL ||
		bios_d->flag == 0 ||
		bios_d->int_mask_port == 0) {
		return;
	}
	if (mask == NULL) {
		return;
	}
	//----------------------------------------
	// Copy the mask
	//----------------------------------------
	memset(mask, 0, sizeof(bios_d->int_mask));
	// 4 channels of the input signals as interrupt for ASIC
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_ASIC) {		// Board type: ASIC
		mask[0] = bios_d->int_mask[0];
	}
	// 16 channels of the input signals as interrupt for Multipoint
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_TATEN) {		// Board type: Multipoint
		mask[0] = bios_d->int_mask[0];
		mask[1] = bios_d->int_mask[1];
	}
	// 32 and 16 channels of the input signals as interrupt for H series
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_H_SERIES) {	// Board type: H series
		if (bios_d->b_info->inp_port_num == 2) {
			mask[0] = bios_d->int_mask[0];
			mask[1] = bios_d->int_mask[1];
		} else {
			mask[0] = bios_d->int_mask[0];
			mask[1] = bios_d->int_mask[1];
			mask[2] = bios_d->int_mask[2];
			mask[3] = bios_d->int_mask[3];
		}
	}
	// All channels of the input signals as interrupt for 8255
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_8255) {		// Board type: 8255
		for(port_no = 0; port_no < bios_d->b_info->inp_port_num; port_no++){
			mask[port_no]	= bios_d->int_mask[port_no];
		}
	}
	// 4 channels of the input signals as interrupt for DM board
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_DM) {		// Board type: DM
		mask[0] = bios_d->int_mask[0];
	}
	return;
}

//================================================================
// Mask one bit of interrupt
//================================================================
long Cdiobi_set_bit_int_mask(PCDIOBI bios_d, short bit_no, short open)
{
	unsigned char	b_data;
	unsigned short	w_data;
	unsigned long	d_data;
	short			port_no;

	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (bios_d == NULL ||
		bios_d->flag == 0 ||
		bios_d->int_mask_port == 0) {
		return 0;
	}
	if (open != CDIOBI_INT_MASK_CLOSE &&
		open != CDIOBI_INT_MASK_OPEN) {
		return -1;
	}
	//----------------------------------------
	// Check the interrupt bits
	//----------------------------------------
	if (Cdiobi_check_int_bit(bios_d, bit_no) == 0) {
		return -1;
	}
	//----------------------------------------
	// In the case of open mask
	//----------------------------------------
	if (open == CDIOBI_INT_MASK_OPEN) {
		// 4 channels of the input signals as interrupt for ASIC
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_ASIC) {		// Board type: ASIC
			b_data = bios_d->int_mask[0] & ~(0x01 << bit_no);
			bios_d->int_mask[0] = b_data;
			_outp(bios_d->int_mask_port, b_data);
		}
		// 16 channels of the input signals as interrupt for Multipoint
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_TATEN) {		// Board type: Multipoint
			w_data = (bios_d->int_mask[1] << 8) | (bios_d->int_mask[0]);
			w_data &= ~(0x01 << bit_no);
			bios_d->int_mask[0] = (unsigned char)(w_data & 0xff);
			bios_d->int_mask[1] = (unsigned char)((w_data >> 8) & 0xff);
			_outpw(bios_d->int_mask_port, w_data);
		}
		// 32 and 16 channels of the input signals as interrupt for H series
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_H_SERIES) {	// Board type: H series
			w_data = (bios_d->int_mask[1] << 8) | (bios_d->int_mask[0]);
			d_data = (bios_d->int_mask[3] << 24) | (bios_d->int_mask[2] << 16) |
					 (bios_d->int_mask[1] << 8) | (bios_d->int_mask[0]);
			if (bios_d->b_info->inp_port_num == 2) {
				w_data &= ~(0x01 << bit_no);
				bios_d->int_mask[0] = (unsigned char)(w_data & 0xff);
				bios_d->int_mask[1] = (unsigned char)((w_data >> 8) & 0xff);
				_outpw(bios_d->int_mask_port, w_data);
			} else {
				d_data &= ~(0x01 << bit_no);
				bios_d->int_mask[0] = (unsigned char)(d_data & 0xff);
				bios_d->int_mask[1] = (unsigned char)((d_data >> 8)  & 0xff);
				bios_d->int_mask[2] = (unsigned char)((d_data >> 16) & 0xff);
				bios_d->int_mask[3] = (unsigned char)((d_data >> 24) & 0xff);
				_outpd(bios_d->int_mask_port, d_data);
			}
		}
		// All channels of the input signals as interrupt for 8255
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_8255) {		// Board type: 8255
			port_no = bit_no / 8;
			b_data = bios_d->int_mask[port_no] & ~(0x01 << (bit_no % 8));
			bios_d->int_mask[port_no] = b_data;
			Cdiobi_8255_port_set(bios_d, &port_no);
			_outp(bios_d->int_mask_port + port_no, b_data);
		}
		// 4 channels of the input signals as interrupt for DM board
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_DM) {		// Board type: DM
			b_data = bios_d->int_mask[0] & ~(0x01 << bit_no);
			bios_d->int_mask[0] = b_data;
			d_data = 0xffff0fff | (b_data << 12);
			_outpd(bios_d->int_mask_port, d_data);
		}
	}
	//----------------------------------------
	// In the case of closing the mask
	//----------------------------------------
	if (open == CDIOBI_INT_MASK_CLOSE) {
		// 4 channels of the input signals as interrupt for ASIC
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_ASIC) {		// Board type: ASIC
			b_data = bios_d->int_mask[0] | (0x01 << bit_no);
			bios_d->int_mask[0] = b_data;
			_outp(bios_d->int_mask_port, b_data);
		}
		// 16 channels of the input signals as interrupt for Multipoint
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_TATEN) {		// Board type: Multipoint
			w_data = (bios_d->int_mask[1] << 8) | (bios_d->int_mask[0]);
			w_data |= (0x01 << bit_no);
			bios_d->int_mask[0] = (unsigned char)(w_data & 0xff);
			bios_d->int_mask[1] = (unsigned char)((w_data >> 8) & 0xff);
			_outpw(bios_d->int_mask_port, w_data);
		}
		// 32 and 16 channels of the input signals as interrupt for H series
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_H_SERIES) {	// Board type: H series
			w_data = (bios_d->int_mask[1] << 8) | (bios_d->int_mask[0]);
			d_data = (bios_d->int_mask[3] << 24) | (bios_d->int_mask[2] << 16) |
					 (bios_d->int_mask[1] << 8) | (bios_d->int_mask[0]);
			if (bios_d->b_info->inp_port_num == 2) {
				w_data |= (0x01 << bit_no);
				bios_d->int_mask[0] = (unsigned char)(w_data & 0xff);
				bios_d->int_mask[1] = (unsigned char)((w_data >> 8) & 0xff);
				_outpw(bios_d->int_mask_port, w_data);
			} else {
				d_data |= (0x01 << bit_no);
				bios_d->int_mask[0] = (unsigned char)(d_data & 0xff);
				bios_d->int_mask[1] = (unsigned char)((d_data >> 8)  & 0xff);
				bios_d->int_mask[2] = (unsigned char)((d_data >> 16) & 0xff);
				bios_d->int_mask[3] = (unsigned char)((d_data >> 24) & 0xff);
				_outpd(bios_d->int_mask_port, d_data);
			}
		}
		// All channels of the input signals as interrupt for 8255
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_8255) {		// Board type: 8255
			port_no	= bit_no / 8;
			b_data	= bios_d->int_mask[port_no] | (0x01 << (bit_no % 8));
			bios_d->int_mask[port_no] = b_data;
			Cdiobi_8255_port_set(bios_d, &port_no);
			_outp(bios_d->int_mask_port + port_no, b_data);
		}
		// 4 channels of the input signals as interrupt for DM board
		if (bios_d->b_info->board_type & CDIOBI_BT_PCI_DM) {		// Board type: DM
			b_data = bios_d->int_mask[0] | (0x01 << bit_no);
			bios_d->int_mask[0] = b_data;
			d_data = 0xffff0fff | (b_data << 12);
			_outpd(bios_d->int_mask_port, d_data);
		}
	}
	return 0;
}

//================================================================
// Interrupt logic setting
//================================================================
long Cdiobi_set_int_logic(PCDIOBI bios_d, short bit_no, short logic)
{
	unsigned char	int_mask[CDIOBI_INT_SENCE_SIZE];
	short			port;
	short			bit;
	unsigned char	b_data;
	unsigned short	w_data;
	unsigned long	d_data;
	unsigned short	no_of_8255;
	
	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	// Return an error if no control port and no DIO_INT_RISE
	if ((bios_d->int_ctrl_port == 0) && (logic != DIO_INT_RISE)){
		return -2;
	}
	if (bios_d == NULL ||
		bios_d->flag == 0 ||
		bios_d->int_ctrl_port == 0) {
		return -1;
	}
	if (logic != DIO_INT_RISE &&
		logic != DIO_INT_FALL) {
		return 0;
	}
	//----------------------------------------
	// Check the interrupt bits
	//----------------------------------------
	if (Cdiobi_check_int_bit(bios_d, bit_no) == 0) {
		return -1;
	}
	//----------------------------------------
	// Make the interrupt logic
	//----------------------------------------
	port	= bit_no / 8;
	bit		= bit_no % 8;
	if (logic == DIO_INT_RISE) {
		bios_d->int_ctrl[port] &= ~(0x01 << bit);
	}
	if (logic == DIO_INT_FALL) {
		bios_d->int_ctrl[port] |= (0x01 << bit);
	}
	//----------------------------------------
	// Save the interrupt mask and full mask
	//----------------------------------------
	Cdiobi_get_all_int_mask(bios_d, int_mask);
	Cdiobi_set_all_int_mask(bios_d, CDIOBI_MASK_ALL, NULL);
	//----------------------------------------
	// Set the logic according to the board
	//----------------------------------------
	// In the case of ASIC
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_ASIC) {	// Board type: ASIC
		b_data = 0;
		if (logic == DIO_INT_RISE) {
			b_data	= (bit_no << 4) | 0x00;
		}
		if (logic == DIO_INT_FALL) {
			b_data	= (bit_no << 4) | 0x04;
		}
		_outp(bios_d->int_ctrl_port, b_data);
		Ccom_udelay(10);		// ASIC: Countermeasure against interrupt logic switching bug
	}
	// In the case of Mlutipoint
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_TATEN) {		// Board type: Multipoint
		w_data = (bios_d->int_ctrl[1] << 8) | bios_d->int_ctrl[0];
		_outpw(bios_d->int_ctrl_port, w_data);
	}
	// 32 and 16 channels of the input signals as interrupt for H series
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_H_SERIES) {	// Board type: H series
		w_data = (bios_d->int_ctrl[1] << 8) | bios_d->int_ctrl[0];
		d_data = (bios_d->int_ctrl[3] << 24) | (bios_d->int_ctrl[2] << 16) | 
				 (bios_d->int_ctrl[1] << 8) | bios_d->int_ctrl[0];
		if (bios_d->b_info->inp_port_num == 2) {
			_outpw(bios_d->int_ctrl_port, w_data);
		} else {
			_outpd(bios_d->int_ctrl_port, d_data);
		}
	}
	// For the devices that doesn't support the 8255 interrupt control, flick with the parameter check of this function
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_8255) {	// Board type: 8255
		for(no_of_8255 = 0; no_of_8255 < bios_d->b_info->num_of_8255; no_of_8255++){
			_outp(bios_d->int_ctrl_port + (no_of_8255 * 4) + 0, bios_d->int_ctrl[(no_of_8255 * 3) + 0]);
			_outp(bios_d->int_ctrl_port + (no_of_8255 * 4) + 1, bios_d->int_ctrl[(no_of_8255 * 3) + 1]);
			_outp(bios_d->int_ctrl_port + (no_of_8255 * 4) + 2, bios_d->int_ctrl[(no_of_8255 * 3) + 2]);
		}
	}
	//----------------------------------------
	// Restore the mask
	//----------------------------------------
	Cdiobi_set_all_int_mask(bios_d, CDIOBI_MASK_DATA, int_mask);
	return 0;
}

//================================================================
// Retrieve the interrupt status
//================================================================
long Cdiobi_sence_int_status(PCDIOBI bios_d, unsigned char *sence)
{
	long			lret = 0;		// No own interrupt
	unsigned char	b_data;
	unsigned short	w_data;
	unsigned long	d_data;
	short			port_no, real_port_no;
	
	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (bios_d == NULL ||
		bios_d->flag == 0 ||
		sence == NULL) {
		return 0;
	}
	//----------------------------------------
	// In the case of no sence port
	// Keep that it is not own interrupt tentatively.
	// This way of thinking will change if the ISA is supported.
	//----------------------------------------
	if (bios_d->int_sence_port == 0) {
		return 0;
	}
	//----------------------------------------
	// Read and reset the interrupt sense port
	//----------------------------------------
	memset(sence, 0, CDIOBI_INT_SENCE_SIZE);
	// In the case of ASIC
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_ASIC) {	// Board type: ASIC
		// Sense and reset
		b_data = _inp(bios_d->int_sence_port);
		_outp(bios_d->int_sence_port, b_data);
		// OFF the masked bits
		b_data &= ~(bios_d->int_mask[0]);
		sence[0] = b_data;
		// Own interrupt?
		if (b_data) {
			lret = 1;
		}
	}
	// In the case of Mlutipoint
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_TATEN) {	// Board type: Multipoint
		// Sense and reset
		w_data = _inpw(bios_d->int_sence_port);
		// OFF the masked bits
		w_data &= ~(((bios_d->int_mask[1]<<8) & 0xff00) | bios_d->int_mask[0]);
		sence[0] = (w_data & 0xff);
		sence[1] = ((w_data >> 8) & 0xff);
		// Own interrupt?
		if (w_data) {
			lret = 1;
		}
	}
	// 32 and 16 channels of the input signals as interrupt for H series
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_H_SERIES) {	// Board type: H series
		if (bios_d->b_info->inp_port_num == 2) {
			// Sence and reset
			w_data = _inpw(bios_d->int_sence_port);
			sence[0] = (w_data & 0xff);
			sence[1] = ((w_data >> 8) & 0xff);
			if (w_data) {
				lret = 1;
			}
		} else {
			// Sence and reset
			d_data = _inpd(bios_d->int_sence_port);
			sence[0] = (d_data & 0xff);
			sence[1] = ((d_data >> 8) & 0xff);
			sence[2] = ((d_data >> 16) & 0xff);
			sence[3] = ((d_data >> 24) & 0xff);
			if (d_data) {
				lret = 1;
			}
		}
	}
	// All channels of the input signals as interrupt for 8255
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_8255) {	// Board type: 8255
		for(port_no = 0; port_no < bios_d->b_info->inp_port_num; port_no++){
			// Sence and reset
			real_port_no = port_no;
			Cdiobi_8255_port_set(bios_d, &real_port_no);
			sence[port_no] = _inp(bios_d->int_sence_port + real_port_no);
			// OFF the masked bits
			sence[port_no] = sence[port_no] & ~(bios_d->int_mask[port_no]);
			// Own interrupt?
			if (sence[port_no]){
				lret = 1;
			}
		}
	}
	// 4 channels of the input signals as interrupt for DM board
	if (bios_d->b_info->board_type & CDIOBI_BT_PCI_DM) {	// Board type: DM
		// Sense and reset	
		d_data = _inpd(bios_d->int_sence_port);
		_outpd(bios_d->int_sence_port, d_data);
		sence[0] = (unsigned char)((d_data >> 12) & 0x0000000f);
		// OFF the masked bits
		sence[0] = sence[0] & ~(bios_d->int_mask[0]);
		// Own interrupt?
		if (sence[0]){
			lret = 1;
		}
	}
	return lret;
}

//================================================================
// Retrieve the interrupt logic
//================================================================
long Cdiobi_get_int_logic(PCDIOBI bios_d, unsigned short *logic)
{
	short		bit_no;
	short		port;
	short		bit;

	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (bios_d == NULL ||
		bios_d->flag == 0 ||
		logic == NULL) {
		return 0;
	}
	//----------------------------------------
	// In the case of no control port,
	// set all valid bits to rise
	//----------------------------------------
	memset(logic, 0, sizeof(short) * CDIOBI_INT_SENCE_SIZE * 8);
	if (bios_d->int_ctrl_port == 0) {
		for (bit_no=0; bit_no<bios_d->b_info->max_int_bit; bit_no++) {
			port	= bit_no / 8;
			bit		= bit_no % 8;
			if (bios_d->b_info->int_bit[port] & (0x01<<bit)) {
				logic[bit_no] = DIO_INT_RISE;
			}
		}
		return bios_d->b_info->max_int_bit;
	}
	//----------------------------------------
	// In the case that there are control ports
	// Control 0 -> return the rising,
	// Control 1 -> return the falling 
	//----------------------------------------
	for (bit_no=0; bit_no<bios_d->b_info->max_int_bit; bit_no++) {
		port	= bit_no / 8;
		bit		= bit_no % 8;
		if (bios_d->int_ctrl[port] & (0x01<<bit)) {
			logic[bit_no] = DIO_INT_FALL;
		} else {
			logic[bit_no] = DIO_INT_RISE;
		}
	}
	return bios_d->b_info->max_int_bit;
}

//================================================================
// Function for retrieving device information
//================================================================
long Cdiobi_get_info(char *device, short info_type, void *param1, void *param2, void *param3)
{
	PCDIOBI_BOARD_INFO	b_info;
	short				i;
	int					copy_ret;
	char				d_name[CDIOBI_BOARD_NAME_MAX];
	void				*c_param1;

	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (param1 == NULL ||
		device == NULL) {
		return -2;		// Buffer address error
	}

	if (access_ok(VERIFY_READ, device, sizeof(d_name)))
	{
		copy_ret = copy_from_user(d_name, device, sizeof(d_name));
	} else {
		strcpy(d_name, device);
	}

	c_param1 = kmalloc(sizeof(void *), GFP_KERNEL);
	if (access_ok(VERIFY_READ, param1, sizeof(void *)))
	{
		copy_ret = copy_from_user(c_param1, param1, sizeof(void *));
	} else {
		memcpy(c_param1, param1, sizeof(void *));
	}
	
	//----------------------------------------
	// Search the board information
	//----------------------------------------
	b_info = NULL;
	for (i=0; strcmp(board_info[i].board_name, "End") != 0; i++) {
		if (strcmp(board_info[i].board_name, d_name) == 0) {
			b_info = &board_info[i];
			break;
		}
	}
	if (b_info == NULL) {
		if (access_ok(VERIFY_WRITE, param1, sizeof(void *)))
		{
			copy_ret = copy_to_user(param1, c_param1, sizeof(void *));
		} else {
			memcpy(param1, c_param1, sizeof(void *));
		}
		kfree(c_param1);
		return -1;
	}
	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	switch (info_type) {
	case IDIO_DEVICE_TYPE:				// Device type (short)
		*(short *)c_param1 = (short)b_info->device_type;
		break;
	case IDIO_NUMBER_OF_8255:			// Number of 8255 (int)
		*(int *)c_param1 = (int)b_info->num_of_8255;
		break;
	case IDIO_IS_8255_BOARD:			// 8255 Type (int)
		if (b_info->num_of_8255 == 0) {
			*(int *)c_param1 = 0;
		} else {
			*(int *)c_param1 = 1;
		}
		break;
	case IDIO_NUMBER_OF_DI_BIT:			// DI BIT(short)
		*(short *)c_param1 = (short)b_info->inp_port_num * 8;
		break;
	case IDIO_NUMBER_OF_DO_BIT:			// DO BIT(short)
		*(short *)c_param1 = (short)b_info->out_port_num * 8;
		break;
	case IDIO_NUMBER_OF_DI_PORT:		// DI PORT(short)
		*(short *)c_param1 = (short)b_info->inp_port_num;
		break;
	case IDIO_NUMBER_OF_DO_PORT:		// DO PORT(short)
		*(short *)c_param1 = (short)b_info->out_port_num;
		break;
	case IDIO_IS_POSITIVE_LOGIC:		// Positive logic? (int)
		*(int *)c_param1 = (int)b_info->is_positive;
		break;
	case IDIO_IS_ECHO_BACK:				// Echo back available? (int)
		*(int *)c_param1 = (int)b_info->can_echo_back;
		break;
	case IDIO_NUM_OF_PORT:				// Number of occupied ports (short)
		*(short *)c_param1 = (short)b_info->port_num;
		break;
	default:
		if (access_ok(VERIFY_WRITE, param1, sizeof(void *)))
		{
			copy_ret = copy_to_user(param1, c_param1, sizeof(void *));
		} else {
			memcpy(param1, c_param1, sizeof(void *));
		}
		kfree(c_param1);
		return -1;		// Information type error
	}

	if (access_ok(VERIFY_WRITE, param1, sizeof(void *)))
	{
		copy_ret = copy_to_user(param1, c_param1, sizeof(void *));
	} else {
		memcpy(param1, c_param1, sizeof(void *));
	}
	kfree(c_param1);
	return 0;
}

//================================================================
// Interrupt available bits check
//================================================================
long Cdiobi_check_int_bit(PCDIOBI bios_d, short bit_no)
{
	short	port;
	short	bit;

	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (bios_d == NULL ||
		bios_d->flag == 0) {
		return 0;		// Buffer address error
	}
	port	= bit_no / 8;
	bit		= bit_no % 8;
	if (bios_d->b_info->int_bit[port] & (0x01<<bit)) {
		return 1;
	}
	return 0;
}

//================================================================
// Input ports check
//================================================================
long Cdiobi_check_inp_port(PCDIOBI bios_d, short port_no)
{
	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (bios_d == NULL ||
		bios_d->flag == 0 ||
		port_no < 0) {
		return 0;
	}
	//----------------------------------------
	// Check
	//----------------------------------------
	if (bios_d->b_info->inp_port_num == 0) {
		return 0;
	}
	if (port_no < 0) {
		return 0;
	}
	if (port_no < bios_d->b_info->inp_port_num) {
		return 1;
	}
	return 0;
}

//================================================================
// Input bits check
//================================================================
long Cdiobi_check_inp_bit(PCDIOBI bios_d, short bit_no)
{
	short	comp_bit;
	
	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (bios_d == NULL ||
		bios_d->flag == 0 ||
		bit_no < 0) {
		return 0;
	}
	//----------------------------------------
	// Check
	//----------------------------------------
	if (bios_d->b_info->inp_port_num == 0) {
		return 0;
	}
	comp_bit = bios_d->b_info->inp_port_num * 8;
	if (bit_no < 0) {
		return 0;
	}
	if (bit_no < comp_bit) {
		return 1;
	}
	return 0;
}
//================================================================
// Output ports check
//================================================================
long Cdiobi_check_out_port(PCDIOBI bios_d, short port_no)
{
	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (bios_d == NULL ||
		bios_d->flag == 0 ||
		port_no < 0) {
		return 0;
	}
	//----------------------------------------
	// Check
	//----------------------------------------
	if (bios_d->b_info->out_port_num == 0) {
		return 0;
	}
	if (port_no < 0) {
		return 0;
	}
	if (port_no < bios_d->b_info->out_port_num) {
		return 1;
	}
	return 0;
}

//================================================================
// Output bits check
//================================================================
long Cdiobi_check_out_bit(PCDIOBI bios_d, short bit_no)
{
	short	comp_bit;
	
	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (bios_d == NULL ||
		bios_d->flag == 0 ||
		bit_no < 0) {
		return 0;
	}
	//----------------------------------------
	// Check
	//----------------------------------------
	if (bios_d->b_info->out_port_num == 0) {
		return 0;
	}
	comp_bit = bios_d->b_info->out_port_num * 8;
	if (bit_no < 0) {
		return 0;
	}
	if (bit_no < comp_bit) {
		return 1;
	}
	return 0;
}

//================================================================
// Check whether the board exists
//================================================================
long Cdiobi_check_board_exist(PCDIOBI bios_d)
{
	unsigned long	filter_value;
	unsigned char	test_value;
	unsigned char	read_value;
	
	//----------------------------------------
	// Check the parameters
	//----------------------------------------
	if (bios_d == NULL ||
		bios_d->flag == 0) {
		return -1;		// Does not exist
	}
	//----------------------------------------
	// Check (use the filter board)
	//----------------------------------------
	// Backup
	filter_value = bios_d->filter_value;
	// Can not check for the case of no filter
	if (bios_d->filter_port == 0) {
		return 0;
	}
	// Test 3
	test_value = 3;
	_outp(bios_d->filter_port, test_value);
	read_value = _inp(bios_d->filter_port);
	if (read_value != test_value) {
		return -1;
	}
	// Test 5
	test_value = 5;
	_outp(bios_d->filter_port, test_value);
	read_value = _inp(bios_d->filter_port);
	if (read_value != test_value) {
		return -1;
	}
	// Restore
	Cdiobi_set_digital_filter(bios_d, filter_value);
	return 0;		// Exist
}

