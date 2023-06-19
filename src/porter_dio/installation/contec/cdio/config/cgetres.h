////////////////////////////////////////////////////////////////////////////////
/// @file   cgetres.h
/// @brief  API-DIO(LNX) config(PCI) : Common driver resource acquisition header file
/// @author &copy;CONTEC CO.,LTD.
/// @since  2002
////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------
// Structure definition
//---------------------------------------------------------------
typedef struct {
	unsigned long		vendor_id;		// Vendor ID
	unsigned long		device_id;		// Device ID
	unsigned long		board_id;		// Board ID
	unsigned long 		io_addr[6];		// I/O address
	unsigned long		io_num;			// The number of I/O Addresses
	unsigned long 		mem_addr[6];	// Memory address
	unsigned long		mem_num;		// The number of memory Addresses
	unsigned long		irq;			// IRQ (If not use IRQ, set it to 0xffffffff)
	long				ret;			// Return value (It is the same as the return code)
} GETPCIRES, *PGETPCIRES;

typedef struct {
	unsigned long		vendor_id;		// Vendor ID
	unsigned long		device_id;		// Device ID
	unsigned long		board_id;		// Board ID
	unsigned long 		revision_id;	// Revision
	long				ret;			// Return value (It is the same as the return code)
} GETPCIREV, *PGETPCIREV;

//---------------------------------------------------------------
// Function definition
//---------------------------------------------------------------
#ifndef MODULE
void res_drv_init(void);
void res_drv_exit(void);
long get_pci_resource(PGETPCIRES param);
long get_pci_contec_revision(PGETPCIREV param);
#endif
//---------------------------------------------------------------
// Control code definition
//---------------------------------------------------------------
#define	CGETRES_IOC_MAGIC			'C'										// Magic No.
#define	CGETRES_IOC_GET_PCI			_IOWR(CGETRES_IOC_MAGIC, 1, PGETPCIRES)	// Get the PCI resource
#define	CGETREV_IOC_GET_PCI			_IOWR(CGETRES_IOC_MAGIC, 2, PGETPCIREV)	// Get the PCI revision (Contec definition)


