////////////////////////////////////////////////////////////////////////////////
/// @file   Cdio_bios.h
/// @brief  API-DIO(LNX) PCI Module - BIOS header file
/// @author &copy;CONTEC CO.,LTD.
/// @since  2002
////////////////////////////////////////////////////////////////////////////////

#ifndef __CDIO_BIOS_H_DEFINED__
#define __CDIO_BIOS_H_DEFINED__

#include "../library/Cdio_ctrl.h"

//================================================================
// Definition values
//================================================================
// Limit
#define	CDIOBI_MAX_PORT				144/8		// The maximum number of I/O ports
#define	CDIOBI_INT_SENCE_SIZE		144/8		// The size of interrupt sense port  BIOS definition value
#define	CDIOBI_DEVICE_NAME_MAX		50			// The maximum length of device name
#define	CDIOBI_BOARD_NAME_MAX		50			// The maximum length of board name
#define	CDIOBI_8255_NUM_MAX			6			// The maximum number of 8255 chips (and set PIO-48D to MAX)
// Interrupt mask
#define	CDIOBI_MASK_ALL				0			// All mask: Mask all of the bits
#define	CDIOBI_MASK_DATA			1			// All mask: Mask with the specified value
#define	CDIOBI_INT_MASK_OPEN		0			// Bit mask: Open the mask
#define	CDIOBI_INT_MASK_CLOSE		1			// Bit mask: Close the mask
// Board type
#define	CDIOBI_BT_PCI_ASIC			0x80000000	// Board type: ASIC
#define	CDIOBI_BT_PCI_TATEN			0x40000000	// Board type: Multipoint
#define	CDIOBI_BT_PCI_RELAY			0x20000000	// Board type: Relay
#define	CDIOBI_BT_PCI_H_SERIES		0x10000000	// Board type: H series
#define	CDIOBI_BT_PCI_8255			0x08000000	// Board type: Programmable board
#define	CDIOBI_BT_PCI_DM			0x04000000	// Board type: BusMaster
// I/O board: ASIC
#define	CDIOBI_PORT_ASIC_INTCTRL	0x0e		// Interrupt control
#define	CDIOBI_PORT_ASIC_FILTER		0x0f		// Digital filter
#define	CDIOBI_PORT_ASIC_INTMASK	0x10		// Interrupt mask
#define	CDIOBI_PORT_ASIC_INTSTS		0x11		// Interrupt status
// I/O board: Multipoint
#define	CDIOBI_PORT_TATEN_FILTER	0x11		// Digital filter
#define	CDIOBI_PORT_TATEN_INTMASK	0x14		// Interrupt mask
#define	CDIOBI_PORT_TATEN_INTSTS	0x14		// Interrupt status
#define	CDIOBI_PORT_TATEN_INTCTRL	0x16		// Interrupt control
// I/O board: H series
#define	CDIOBI_PORT_H_FILTER		0x09		// Digital filter
#define	CDIOBI_PORT_H_INTMASK		0x10		// Interrupt mask
#define	CDIOBI_PORT_H_INTSTS		0x10		// Interrupt status
#define	CDIOBI_PORT_H_INTCTRL		0x14		// Interrupt control
// I/O board: Programmable board PIO-48D(PCI)
#define	CDIOBI_PORT_8255_INTMASK	0x8			// Interrupt mask
#define	CDIOBI_PORT_8255_INTSTS		0x8			// Interrupt control
// I/O board: Programmable board H series
#define	CDIOBI_PORT_8255_H_INTCTRL	0x10		// Interrupt control
#define	CDIOBI_PORT_8255_H_FILTER	0x18		// Digital filter
// I/O board: Programmable board Multipoint type 96 channels 144 channels
#define	CDIOBI_PORT_8255_T_INTMASK	0x18		// Interrupt mask
#define	CDIOBI_PORT_8255_T_INTSTS	0x18		// Interrupt status
#define	CDIOBI_PORT_8255_T_INTCTRL	0x30		// Interrupt control
#define	CDIOBI_PORT_8255_T_FILTER	0x48		// Digital filter
// I/O board: BusMaster PIO-32DM(PCI)
#define	CDIOBI_PORT_DM_INTMASK		0x10		// Interrupt mask
#define	CDIOBI_PORT_DM_INTSTS		0x14		// Interrupt status

// Interrupt/Trigger
#ifndef DIO_INT_NONE
	#define	DIO_INT_NONE		0				// Interrupt: Mask
	#define	DIO_INT_RISE		1				// Interrupt: Rising
	#define	DIO_INT_FALL		2				// Interrupt: Falling
	#define	DIO_TRG_NONE		0				// Trigger: Mask
	#define	DIO_TRG_RISE		1				// Trigger: Rising
	#define	DIO_TRG_FALL		2				// Trigger: Falling
#endif

//================================================================
// Structure
//================================================================
//----------------------------------------------------------------
// Board information structure
// : Board database
//----------------------------------------------------------------
typedef struct {
	char			board_name[CDIOBI_BOARD_NAME_MAX];		// Board name
	unsigned long	board_type;								// Board type
	unsigned short	device_type;							// Device type (Bus)
	unsigned short	vendor_id;								// Vendor ID
	unsigned short	device_id;								// Device ID
	unsigned short	port_num;								// Number of occupied ports
	unsigned short	inp_port_off;							// Offset of input port address
	unsigned short	out_port_off;							// Offset of output port address
	unsigned short	filter_off;								// Offset of filter address
	unsigned short	int_mask_off;							// Offset of interrupt mask address
	unsigned short	int_sence_off;							// Offset of interrupt sence address
	unsigned short	int_ctrl_off;							// Offset of interrupt control address
	unsigned short	inp_port_num;							// Number of input ports
	unsigned short	out_port_num;							// Number of output ports
	unsigned short	min_filter_value;						// Minimum value of filter
	unsigned short	max_filter_value;						// Maximum value of filter
	unsigned char	int_bit[CDIOBI_INT_SENCE_SIZE];			// Interrupt bit (valid bit is 1)
	unsigned short	max_int_bit;							// Maximum number of interrupt bits
	unsigned short	num_of_8255;							// Number of 8255 chips
	unsigned short	can_echo_back;							// Echo back available? (0: Unavailable, 1: Available)
	unsigned short	is_positive;							// Positive logic? (0: Negative logic, 1: Positive logic)
} CDIOBI_BOARD_INFO, *PCDIOBI_BOARD_INFO;
//----------------------------------------------------------------
// BIOS data structure
// : Information used by the BIOS
//----------------------------------------------------------------
typedef struct {
	unsigned short		flag;								// 0: Uninitialized, 1: Initialized
	PCDIOBI_BOARD_INFO	b_info;								// Address of board information structure
	unsigned short		port_base;							// Base address of port
	unsigned short		irq_no;								// IRQ
	unsigned short		inp_port;							// Input port address
	unsigned short		out_port;							// Output port address
	unsigned short		filter_port;						// Filter address
	unsigned short		int_mask_port;						// Interrupt mask address
	unsigned short		int_sence_port;						// Interrupt sence address
	unsigned short		int_ctrl_port;						// Interrupt control address
	unsigned char		out_data[CDIOBI_MAX_PORT];			// Output latch data
	unsigned char		int_mask[CDIOBI_INT_SENCE_SIZE];	// Interrupt mask data (0: Open, 1: Close)
	unsigned char		int_ctrl[CDIOBI_INT_SENCE_SIZE];	// Interrupt control data (0: Rising, 1: Falling)
	unsigned long		filter_value;						// Digital filter value
	unsigned short		ctrl_word_8255[CDIOBI_8255_NUM_MAX];// Control word of 8255
} CDIOBI, *PCDIOBI;
//================================================================
// Function Prototype
//================================================================
long Cdiobi_init_bios(PCDIOBI bios_d, char *board_name, unsigned long *io_addr, unsigned long irq_no);
void Cdiobi_clear_bios(PCDIOBI bios_d);
void Cdiobi_init_latch_data(PCDIOBI bios_d);
void Cdiobi_input_port(PCDIOBI bios_d, short port_no, unsigned char *data);
void Cdiobi_input_bit(PCDIOBI bios_d, short bit_no, unsigned char *data);
void Cdiobi_output_port(PCDIOBI bios_d, short port_no, unsigned char data);
void Cdiobi_output_bit(PCDIOBI bios_d, short bit_no, unsigned char data);
void Cdiobi_echo_port(PCDIOBI bios_d, short port_no, unsigned char *data);
void Cdiobi_echo_bit(PCDIOBI bios_d, short bit_no, unsigned char *data);
long Cdiobi_set_digital_filter(PCDIOBI bios_d, unsigned long filter_value);
long Cdiobi_get_digital_filter(PCDIOBI bios_d, unsigned long *filter_value);
void Cdiobi_set_all_int_mask(PCDIOBI bios_d, int flag, unsigned char *mask);
void Cdiobi_get_all_int_mask(PCDIOBI bios_d, unsigned char *mask);
long Cdiobi_set_bit_int_mask(PCDIOBI bios_d, short bit_no, short open);
long Cdiobi_set_int_logic(PCDIOBI bios_d, short bit_no, short logic);
long Cdiobi_sence_int_status(PCDIOBI bios_d, unsigned char *sence);
long Cdiobi_get_int_logic(PCDIOBI bios_d, unsigned short *logic);
long Cdiobi_set_8255_mode(PCDIOBI bios_d, unsigned short chip_no, unsigned short ctrl_word);
long Cdiobi_get_8255_mode(PCDIOBI bios_d, unsigned short chip_no, unsigned short *ctrl_word);
void Cdiobi_8255_port_set(PCDIOBI bios_d, unsigned short *pport_no);
long Cdiobi_get_info(char *device, short info_type, void *param1, void *param2, void *param3);
long Cdiobi_check_int_bit(PCDIOBI bios_d, short bit_no);
long Cdiobi_check_inp_port(PCDIOBI bios_d, short port_no);
long Cdiobi_check_inp_bit(PCDIOBI bios_d, short bit_no);
long Cdiobi_check_out_port(PCDIOBI bios_d, short port_no);
long Cdiobi_check_out_bit(PCDIOBI bios_d, short bit_no);
long Cdiobi_check_board_exist(PCDIOBI bios_d);
#endif

