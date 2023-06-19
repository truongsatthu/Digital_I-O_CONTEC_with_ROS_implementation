////////////////////////////////////////////////////////////////////////////////
/// @file   Cdio_user_irq.c
/// @brief  API-DIO(LNX) PCI Module - User interruption routine
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
// Interrupt service routine 
//================================================================
void Cdio_user_irq(char *board_name, unsigned char board_id, unsigned short io_addr, unsigned char *status)
{
/*
	printk("board_name=%s\n", board_name);
	printk("board_id=%d\n", (int)board_id);
	printk("io_addr=%X\n", (int)io_addr);
	printk("status=%X\n", (int)status[0]);
*/
	return;
}
