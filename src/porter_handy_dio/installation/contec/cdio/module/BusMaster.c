////////////////////////////////////////////////////////////////////////////////
/// @file   BusMaster.c
/// @brief  API-DIO(LNX) PCI Module - BusMaster library source file
/// @author &copy;CONTEC CO.,LTD.
/// @since  2003
////////////////////////////////////////////////////////////////////////////////

#ifndef __BUSMASTER_C__
#define __BUSMASTER_C__

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
#include "BusMaster.h"

//--------------------------------------------------
// Constant
//--------------------------------------------------
#define	BM_MAX_TRANS_SIZE	0x4000000		// 64MByte
#define	BM_MAX_SG_SIZE		0x4000			// Maximum number of S/G
#define	BM_COUNTER_BIT		24				// Features 25 bit counter
#define	BM_COUNTER_MASK		0x00FFFFFF		// 25 bit counter mask
#define	BM_CARRY_MASK		0xFF000000		// 7 Bit carry mask

// Output port
#define	BM_PORT_INP_SGAddr			(pMasAddr->dwPortBmAddr+0x00)		// Set the S/G address (input)
#define	BM_PORT_OUT_SGAddr			(pMasAddr->dwPortBmAddr+0x10)		//   	   ...         (output)
#define	BM_PORT_INP_SGOffset		(pMasAddr->dwPortBmAddr+0x04)		// Set the offset from the S/G address (input)
#define	BM_PORT_OUT_SGOffset		(pMasAddr->dwPortBmAddr+0x14)		//         ...         (output)
#define	BM_PORT_INP_SGWriteStart	(pMasAddr->dwPortBmAddr+0x08)		// Start/Stop rewriting of S/G address	(input)
#define	BM_PORT_OUT_SGWriteStart	(pMasAddr->dwPortBmAddr+0x18)		//         ...                          (output)

#define	BM_PORT_INP_IntDataLen		(pMasAddr->dwPortBmAddr+0x0c)		// Set number of interrupt occurrence data (input)
#define	BM_PORT_OUT_IntDataLen		(pMasAddr->dwPortBmAddr+0x1c)		//         ...                             (output)
#define	BM_PORT_Start_Enable		(pMasAddr->dwPortBmAddr+0x20)		// Set transfer start/stop (I/O port)
#define	BM_PORT_Reset				(pMasAddr->dwPortBmAddr+0x24)		// Reset

#define	BM_PORT_IntMask				(pMasAddr->dwPortBmAddr+0x28)		// Mask the interrupt
#define	BM_PORT_IntClear			(pMasAddr->dwPortBmAddr+0x2c)		// Clear the interrupt status

// Input port
#define	BM_PORT_INP_TotalDataLen	(pMasAddr->dwPortBmAddr+0x00)		// Number of all transferred data (input)
#define	BM_PORT_OUT_TotalDataLen	(pMasAddr->dwPortBmAddr+0x10)		//         ...                    (output)
#define	BM_PORT_INP_FifoConter		(pMasAddr->dwPortBmAddr+0x04)		// FIFO count  (input)
#define	BM_PORT_OUT_FifoConter		(pMasAddr->dwPortBmAddr+0x14)		//         ... (output)
#define	BM_PORT_INP_SGCounter		(pMasAddr->dwPortBmAddr+0x08)		// List number currently being transferred (input)
#define	BM_PORT_OUT_SGCounter		(pMasAddr->dwPortBmAddr+0x18)		//         ...                             (output)
#define	BM_PORT_IntStatus			(pMasAddr->dwPortBmAddr+0x2c)		// Interruput status

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

//--------------------------------------------------
// Prototype declaration (internal interface)
//--------------------------------------------------
static long BmMakeSGList(PSGLIST pSgList, unsigned long dwDir);
static long BmSetSGAddress(PMASTERADDR pMasAddr, unsigned long dwDir);

//========================================================================
// Function name  : ApiBmGetPciResource
// Function	  	  : Retrieve the PCI resource.
//		   			Call it instead of Ccom_get_pci_resource.
//		   			Retrieving the pci_dev structure is different from the Ccom_get_pci_resource().
// I/F			  : External
// In			  : vendor_id			: Vendor ID
//		   			device_id			: Device ID
//		   			board_id			: Board ID
//		   			io_addr				: IO Address
//		   			io_num				: Number of IO ports to use
//		   			mem_addr			: Memory address
//		   			mem_num				: Number of memory addresses to use
//		   			irq					: IRQ to use
//		   			pci_dev				: pci_dev structure
// Return value   : Normal completed	BM_ERROR_SUCCESS	: Normal completed
//		   			Abnormal completed	BM_ERROR_NO_BOARD	: No board found in getting resource.
//										BM_ERROR_ARRY_SIZE 	: The array size is not enough in getting resource.
//										BM_ERROR_BOARDID	: Board ID duplicated in getting resource.
// Addition  	  : Call only once in the DriverEntry or in the corresponding portion.
//		   			The pci_dev structure retrieved in this function is used by ApiBmCreate().
//========================================================================
long ApiBmGetPciResource(
	unsigned short vendor_id,
 	unsigned short device_id,
	unsigned short board_id,
	unsigned long *io_addr,
	unsigned long *io_num,
	unsigned long *mem_addr,
	unsigned long *mem_num,
	unsigned long *irq,
	struct pci_dev *ppci_dev)
{
    struct pci_dev  *pdev = NULL;
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
		return BM_ERROR_NO_BOARD;
	}
#endif
	//----------------------------------------
	// If the area is not sufficient, return Array Size Error
	//----------------------------------------
	if (io_addr == NULL || *io_num < 6) {
		return BM_ERROR_ARRY_SIZE;
	}
	if (mem_addr == NULL || *mem_num < 6) {
		return BM_ERROR_ARRY_SIZE;
	}
	if (irq == NULL) {
		return BM_ERROR_ARRY_SIZE;
	}
	//----------------------------------------
	// Prepare and initialize the return data for the case of no board found
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
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
	while ((pdev = pci_get_device(vendor_id, device_id, pdev))) {
#else
	while ((pdev = pci_find_device(vendor_id, device_id, pdev))) {
#endif
		//----------------------------------------
		// Pass the case that the vendor ID and the device ID do not match with each one you want
		//----------------------------------------
		if (!(pdev->vendor == vendor_id && pdev->device == device_id)) {
			continue;
		}
		//----------------------------------------
		// Pass the case that the revision ID (Board ID) does not match with that you want
		//----------------------------------------
		pci_read_config_byte(pdev,PCI_REVISION_ID,&revision);
		if (revision != board_id) {
			continue;
		}
		//----------------------------------------
		// Enable the PCI device
		//----------------------------------------
		if(pci_enable_device(pdev)){
			return CCOM_ERROR_NO_BOARD;
		}
		//----------------------------------------
		// Board is found!
		//----------------------------------------
		memcpy(ppci_dev, pdev, sizeof(struct pci_dev));
		found_num++;
		io_num_tmp	= 0;
		mem_num_tmp	= 0;
		irq_tmp		= 0xffffffff;
		//----------------------------------------
		// Loop for retrieving I/O and memory
		//----------------------------------------
		for (i=0; i<6 ;i++) {
			//----------------------------------------
			// Kernel version 2.4.X - 2.6.X and 3.10.X
			//----------------------------------------
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) ||\
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
			if (!pci_resource_start(pdev, i)) {
   				break;
			}
			if (pci_resource_flags(pdev, i) & IORESOURCE_IO) {
				io_tmp[io_num_tmp]		= pci_resource_start(pdev, i);
				io_num_tmp++;
			} else {
				mem_tmp[mem_num_tmp]	= pci_resource_start(pdev, i) & PCI_BASE_ADDRESS_MEM_MASK;
				mem_num_tmp++;
			}
#endif
			//----------------------------------------
			// Kernel version 2.2.X
			//----------------------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0))
			if ( pdev->base_address[i] & PCI_BASE_ADDRESS_SPACE_IO) {
				io_tmp[io_num_tmp]		= pdev->base_address[i] & PCI_BASE_ADDRESS_IO_MASK;
				io_num_tmp++;
			} else if ( pdev->base_address[i] & PCI_BASE_ADDRESS_SPACE_MEMORY) {
				mem_tmp[mem_num_tmp]	=  pdev->base_address[i] & PCI_BASE_ADDRESS_MEM_MASK;
				mem_num_tmp++;
			}
#endif
		}
		//----------------------------------------
		// Retrieving IRQ
		//----------------------------------------
		pci_read_config_byte(pdev, PCI_INTERRUPT_PIN, &irq_pin);
		if (irq_pin != 0 && pdev->irq != 0) {
			irq_tmp	= pdev->irq;
		}
	}
	//----------------------------------------
	// Return an error if the found number is equal to 0 or more than 1
	//----------------------------------------
	if (found_num == 0) {
		return BM_ERROR_NO_BOARD;
	}
	if (found_num > 1) {
		return BM_ERROR_NO_BOARD;
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
	return BM_ERROR_SUCCESS;
}

//========================================================================
// Function name  : ApiBmCreate
// Function	  	  : Initialize the hardware of the BusMaster and the MASTERADDR structure.
// I/F	 		  : External
// In	 		  : dwPortBmAddr	 	: Specify the base address (bus master) of the board.
//		   			dwPortPciAddr 	 	: Specify the address for PCI.
// Out	 		  : pMasAddr		 	: Specify the address of the MASTERADDR structure.
//		   			ppci_dev		 	: Address of the pci_dev structure.
// Return value   : Normal completed 	: BM_ERROR_SUCCESS
//		   			Abnormal completed	: BM_ERROR_PARAM
// Addition  	  : This function is called only once in the ccom_entry->add_device 
//					or in the corresponding portion
//========================================================================
long ApiBmCreate(PMASTERADDR pMasAddr, struct pci_dev *ppci_dev, unsigned long dwPortBmAddr, unsigned long dwPortPciAddr)
{
	long	lret;

	//--------------------------------------------
	// Initialize the variables and structures
	//--------------------------------------------
	lret	= BM_ERROR_SUCCESS;
	memset(pMasAddr, 0, sizeof(MASTERADDR));
	pMasAddr->BmOut.dwIntMask	= 0x7f;
	pMasAddr->BmInp.dwIntMask	= 0x7f;
	//--------------------------------------------
	// Check the parameters
	//--------------------------------------------
	if (dwPortBmAddr == 0 || dwPortBmAddr == 0xffffffff ||
		dwPortPciAddr == 0 || dwPortPciAddr == 0xffffffff) {
		return BM_ERROR_PARAM;
	}
	//--------------------------------------------
	// Set the memory base address
	//--------------------------------------------
	pMasAddr->dwPortBmAddr	= dwPortBmAddr;
	pMasAddr->dwPortPciAddr	= dwPortPciAddr;
	//--------------------------------------------
	// Set the pci_dev structure
	//--------------------------------------------
	memcpy(&pMasAddr->PciDev, ppci_dev, sizeof(struct pci_dev));
	//--------------------------------------------
	// Mask the interrupt
	//--------------------------------------------
	BmOutD(BM_PORT_IntMask, 0xffffffff);
	//--------------------------------------------
	// Reset the board
	//--------------------------------------------
	lret	= ApiBmReset(pMasAddr, BM_RESET_ALL);
	if (lret != BM_ERROR_SUCCESS) {
		return	lret;
	}

	return	BM_ERROR_SUCCESS;
}

//========================================================================
// Function name  : ApiBmDestroy
// Function	  	  : Exit processing of the hardware and MASTERADDR structure of the BusMaster,
//					post-processing of the buffer
// I/F	 	  	  : External
// In	 	  	  : pMasAddr			: Point to the MASTERADDR structure
// Out	 	  	  : 
// Return value   : Normal completed	: BM_ERROR_SUCCESS
// Addition  	  : This function is called only once in driver exit processing.
//========================================================================
long ApiBmDestroy(PMASTERADDR pMasAddr)
{
	long	lret = BM_ERROR_SUCCESS;
	
	//--------------------------------------------
	// Stop transfer (Both directions)
	//--------------------------------------------
	lret = ApiBmStop(pMasAddr, BM_DIR_IN | BM_DIR_OUT);
	if (lret != BM_ERROR_SUCCESS) {
		return	lret;
	}
	//---------------------------------------------
	// Unlock the memory
	//---------------------------------------------
	lret = ApiBmUnlockMem(pMasAddr, BM_DIR_IN);
	lret = ApiBmUnlockMem(pMasAddr, BM_DIR_OUT);
	//--------------------------------------------
	// Reset the board
	//--------------------------------------------
//	lret = ApiBmReset(pMasAddr, BM_RESET_ALL);
//	if (lret != BM_ERROR_SUCCESS) {
//		return	lret;
//	}
	//--------------------------------------------
	// Mask the interrupt
	//--------------------------------------------
	BmOutD(BM_PORT_IntMask, 0xffffffff);
	BmOutD(BM_PORT_IntClear, 0xffffffff);
	//--------------------------------------------
	// Initialize the variables and structures
	//--------------------------------------------
	memset(pMasAddr, 0, sizeof(MASTERADDR));
	pMasAddr->BmOut.dwIntMask = 0x7f;
	pMasAddr->BmInp.dwIntMask = 0x7f;

	return BM_ERROR_SUCCESS;
}

//========================================================================
// Function name  : ApiBmGetBufLen
// Function	  	  : Retrieve size (number of data) of the user memory
// I/F	 	  	  : External
// In 		  	  : pMasAddr			: Point to the MASTERADDR structure
//		   			dwDir				: Transfer direction (BM_DIR_IN / BM_DIR_OUT)
// Out	 	  	  : pMasAddr 			: Store the various information
//		 			dwLen				: Size (number of data) of the user memory
// Return value   : Normal completed	: BM_ERROR_SUCCESS
//		   		    Abnormal completed	: BM_ERROR_PARAM
// Addition  	  : 
//		   
//========================================================================
long ApiBmGetBufLen(PMASTERADDR pMasAddr, unsigned long dwDir, unsigned long *dwLen)
{
	long	lret = BM_ERROR_SUCCESS;
	BMEMBER	*pBmTmp;

	//---------------------------------------------
	// Check the parameters (transfer direction)
	//---------------------------------------------
	if (dwDir != BM_DIR_IN && dwDir != BM_DIR_OUT) {
		return BM_ERROR_PARAM;
	}
	//---------------------------------------------
	// Select the I/O data structure
	//---------------------------------------------
	if (dwDir == BM_DIR_IN) {
		pBmTmp	= &pMasAddr->BmInp;
	} else {
		pBmTmp	= &pMasAddr->BmOut;
	}
	//---------------------------------------------
	// Return the number of data
	//---------------------------------------------
	*dwLen	= pBmTmp->SgList.dwBuffLen;

	return lret;
}

//========================================================================
// Function name  : ApiBmSetBuffer
// Function	      : Set information such as the user memory.
// I/F 		  	  : External
// In 		  	  : pMasAddr			: Point to the MASTERADDR structure
//		   			dwDir				: Transfer direction (BM_DIR_IN / BM_DIR_OUT)
//		   			Buff				: Start point to the user memory
//		   			dwLen				: Size (number of data) of the user memory
//		   			dwIsRing			: Whether it is ring buffer or not (BM_WRITE_ONCE/BM_WRITE_RING)
// Out	 	  	  : pMasAddr 			: Store the various information
// Return value   : Normal completed	: BM_ERROR_SUCCESS
//		   		   	Abnormal completed	: BM_ERROR_PARAM
// Addition	  	  : 
//		   
//========================================================================
long ApiBmSetBuffer(PMASTERADDR pMasAddr, unsigned long dwDir, void *Buff, unsigned long dwLen, unsigned long dwIsRing)
{
	long			lret = BM_ERROR_SUCCESS;
	BMEMBER			*pBmTmp;
	unsigned short	wTmp;
	int				direction;
	
	//---------------------------------------------
	// Check the parameters (transfer direction)
	//---------------------------------------------
	if (dwDir != BM_DIR_IN && dwDir != BM_DIR_OUT) {
		return BM_ERROR_PARAM;
	}
	//---------------------------------------------
	// Error during transfer
	//---------------------------------------------
	BmInpW(BM_PORT_Start_Enable, wTmp);
	if (wTmp & dwDir) {
		return BM_ERROR_SEQUENCE;
	}
	//---------------------------------------------
	// Select the I/O data structure
	//---------------------------------------------
	if (dwDir == BM_DIR_IN) {
		pBmTmp	= &pMasAddr->BmInp;
	} else {
		pBmTmp	= &pMasAddr->BmOut;
	}
	//---------------------------------------------
	// If the previous buffer is not unlocked, unlock it
	//---------------------------------------------
	if (pBmTmp->SgList.dwBuffLen != 0) {
		lret = ApiBmUnlockMem(pMasAddr, dwDir);
		if (lret != BM_ERROR_SUCCESS) {
			return	lret;
		}
	}
	//---------------------------------------------
	// Check the parameters (Check if the buffer is NULL)
	//---------------------------------------------
	if (Buff == NULL) {
		ApiBmUnlockMem(pMasAddr, dwDir);
		return BM_ERROR_PARAM;
	}
	//---------------------------------------------
	// Check the parameters (the number of data in the buffer)
	//---------------------------------------------
	if (dwLen == 0) {
		ApiBmUnlockMem(pMasAddr, dwDir);
		return BM_ERROR_PARAM;
	}
	//---------------------------------------------
	// Save point to the user memory
	//---------------------------------------------
	pBmTmp->SgList.Buff			= (unsigned char*)Buff;
	pBmTmp->SgList.dwBuffLen	= dwLen;
	pBmTmp->SgList.dwIsRing		= dwIsRing;
	//---------------------------------------------
	// Map the buffer of user space to kernel space
	//---------------------------------------------
	lret = BmMakeSGList(&pBmTmp->SgList, dwDir);
	if (lret != BM_ERROR_SUCCESS) {
		ApiBmUnlockMem(pMasAddr, dwDir);
		return	lret;
	}
	//--------------------------------------------
	// Specify the transfer direction
	//--------------------------------------------
	if(dwDir == BM_DIR_OUT){
		direction = PCI_DMA_TODEVICE;
	}else{
		direction = PCI_DMA_FROMDEVICE;
	}
	//--------------------------------------------
	// Map the scatter list
	//--------------------------------------------
	// Kernel version 2.4.X
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
	pci_map_sg(&pMasAddr->PciDev, 
				pBmTmp->SgList.List,
				pBmTmp->SgList.iobuf->nr_pages,
				direction);
	// Kernel version 2.6.X
#else
	pci_map_sg(&pMasAddr->PciDev, 
				pBmTmp->SgList.List,
				pBmTmp->SgList.nr_pages,
				direction);
#endif
	//---------------------------------------------
	// Set S/G list to the board
	//---------------------------------------------
	lret = BmSetSGAddress(pMasAddr, dwDir);
	if (lret != BM_ERROR_SUCCESS) {
		ApiBmUnlockMem(pMasAddr, dwDir);
		return lret;
	}
	return	lret;
}

//========================================================================
// Kernel version 2.4.X
//========================================================================
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
//========================================================================
// Function name  : BmMakeSGList
// Function		  : Map the buffer of user space to kernel space and generate a list 
//					that can be registered in the hard S/G list.
// I/F	 		  : Internal
// In	 		  : SgList			: Point to the scatter list structure
//		   			dwDir			: BM_DIR_IN / BM_DIR_OUT
// Out	 		  : SgList
// Return value   : 
//========================================================================
static long BmMakeSGList(PSGLIST pSgList, unsigned long dwDir)
{
	int		rw;			// Transfer direction
	int		page_no;
	long	lret;
	
	//--------------------------------------------
	// Check the parameters
	//--------------------------------------------
	if (dwDir != BM_DIR_IN && dwDir != BM_DIR_OUT) {
		return	BM_ERROR_PARAM;
	}
	//--------------------------------------------
	// Transfer direction is specified by READ, WRITE which defined in /linux/fs.h
	//--------------------------------------------
	if(dwDir == BM_DIR_OUT){
		rw = WRITE;
	}else{
		rw = READ;
	}
	//--------------------------------------------
	// I/O vector allocation
	//--------------------------------------------
	lret = alloc_kiovec(1, &pSgList->iobuf);
	if (lret){
		return BM_ERROR_MEM;
	}
	//--------------------------------------------
	// Map the user I/O buffer
	//--------------------------------------------
	lret = map_user_kiobuf(rw, pSgList->iobuf, (unsigned long) pSgList->Buff, sizeof(unsigned long) * pSgList->dwBuffLen);
	if (lret) {
		free_kiovec(1, &pSgList->iobuf);
		return BM_ERROR_MEM;
	}
	//--------------------------------------------
	// Lock the user I/O buffer
	//--------------------------------------------
	lret = lock_kiovec(rw, &pSgList->iobuf, 1);
	if (lret) {
		free_kiovec(1, &pSgList->iobuf);
		return BM_ERROR_LOCK_MEM;
	}
	//---------------------------
	// Allocate the sg list memory
	//---------------------------
	pSgList->List = Ccom_alloc_pages(GFP_KERNEL, (pSgList->iobuf->nr_pages * sizeof(struct scatterlist)));
	if(pSgList->List == NULL){
		return BM_ERROR_MEM;
	}
	//---------------------------
	// Initialize sg List memory with 0
	//---------------------------
	memset(pSgList->List, 0, (pSgList->iobuf->nr_pages * sizeof(struct scatterlist)));

//========================================================================
// Kernel version 2,4,0 - 2.4.12
//========================================================================
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,4,13))
	//============================================================
	// Set the length and address for each page to the scatter list
	//============================================================
	//------------------------------------------
	// When use only one page
	//------------------------------------------
	if(pSgList->iobuf->nr_pages == 1){
		pSgList->List[0].length		= pSgList->iobuf->length;
		pSgList->List[0].address	= page_address(pSgList->iobuf->maplist[0]) + pSgList->iobuf->offset;
	//------------------------------------------
	// When use the multiple pages
	//------------------------------------------
	}else{
		//-----------------------------
		// Set the page 0
		//-----------------------------
		pSgList->List[0].length		= PAGE_SIZE - pSgList->iobuf->offset;
		pSgList->List[0].address	= page_address(pSgList->iobuf->maplist[0]) + pSgList->iobuf->offset;
		//-----------------------------
		// Set the intermediate page
		//-----------------------------
		for (page_no = 1; page_no < (pSgList->iobuf->nr_pages - 1); page_no++){
			pSgList->List[page_no].address	= page_address(pSgList->iobuf->maplist[page_no]);
			pSgList->List[page_no].length	= PAGE_SIZE;
		}
		//-----------------------------
		// Set the last page
		//-----------------------------
		page_no = pSgList->iobuf->nr_pages - 1;
		if(pSgList->iobuf->offset){
			pSgList->List[page_no].length = (pSgList->iobuf->offset + pSgList->iobuf->length) % PAGE_SIZE ;
		}else{
			pSgList->List[page_no].length = PAGE_SIZE;
		}
		pSgList->List[page_no].address	= page_address(pSgList->iobuf->maplist[page_no]);
	}
//========================================================================
// Kernel version 2,4,13 -
//========================================================================
#else
	//------------------------------------------
	// When use only one page
	//------------------------------------------
	if(pSgList->iobuf->nr_pages == 1){
		pSgList->List[0].page		= pSgList->iobuf->maplist[0];
		pSgList->List[0].offset		= pSgList->iobuf->offset;
		pSgList->List[0].length		= pSgList->iobuf->length;
	//------------------------------------------
	// When use the multiple pages
	//------------------------------------------
	}else{
		//-----------------------------
		// Set the page 0
		//-----------------------------
		pSgList->List[0].page		= pSgList->iobuf->maplist[0];
		pSgList->List[0].offset		= pSgList->iobuf->offset;
		pSgList->List[0].length		= PAGE_SIZE - pSgList->iobuf->offset;
		//-----------------------------
		// Set the intermediate page
		//-----------------------------
		for (page_no = 1; page_no < (pSgList->iobuf->nr_pages - 1); page_no++){
			pSgList->List[page_no].page		= pSgList->iobuf->maplist[page_no];
			pSgList->List[page_no].offset	= 0;
			pSgList->List[page_no].length	= PAGE_SIZE;
		}
		//-----------------------------
		// Set the last page
		//-----------------------------
		page_no = pSgList->iobuf->nr_pages - 1;
		if(pSgList->iobuf->offset){
			pSgList->List[page_no].length	= (pSgList->iobuf->offset + pSgList->iobuf->length) % PAGE_SIZE ;
		}else{
			pSgList->List[page_no].length	= PAGE_SIZE;
		}
		pSgList->List[page_no].page			= pSgList->iobuf->maplist[page_no];
		pSgList->List[page_no].offset		= 0;
	}
#endif
	return	BM_ERROR_SUCCESS;
}

//========================================================================
// Kernel version 2.6.X
//========================================================================
#else
//========================================================================
// Function name  : BmMakeSGList
// Function 	  : Map the buffer of user space to kernel space and generate a list 
//					that can be registered in the hard S/G list.
// I/F	 		  : Internal
// In	 		  : SgList			: Point to the scatter list structure
//		   			dwDir			: BM_DIR_IN / BM_DIR_OUT
// Out	 		  : SgList
// Return value   : 
//========================================================================
static long BmMakeSGList(PSGLIST pSgList, unsigned long dwDir)
{
	int				rw;				// Transfer direction
	int				page_no;
	unsigned long	buf_length;
	unsigned long	buf_offset;
	int				pgcount;
	unsigned long	v_address;		// Start virtual address of the user buffer
	struct page		**maplist;		// Map list

	//--------------------------------------------
	// Check the parameters
	//--------------------------------------------
	if (dwDir != BM_DIR_IN && dwDir != BM_DIR_OUT) {
		return	BM_ERROR_PARAM;
	}
	//--------------------------------------------
	// Transfer direction is specified by READ, WRITE which defined in /linux/fs.h
	//--------------------------------------------
	if(dwDir == BM_DIR_OUT){
		rw = WRITE;
	}else{
		rw = READ;
	}
	//--------------------------------------------
	// Get the size of the user buffer
	//--------------------------------------------
	buf_length	= sizeof(unsigned int) * pSgList->dwBuffLen;
	//--------------------------------------------
	// Get the start virtual address of the user buffer
	//--------------------------------------------
	v_address	= (unsigned long) pSgList->Buff;
	//--------------------------------------------
	// Get the required number of pages from the user buffer size
	// and allocate memory to the page structure for storing the address information
	//--------------------------------------------
	pgcount	= ((v_address + buf_length + PAGE_SIZE - 1) / PAGE_SIZE) - (v_address / PAGE_SIZE);
	maplist	= kmalloc(pgcount * sizeof(struct page **), GFP_KERNEL);
	if (maplist == NULL){
		return BM_ERROR_MEM;
	}
	//--------------------------------------------
	// Get the offset of the buffer
	//--------------------------------------------
	buf_offset	= v_address & (PAGE_SIZE-1);
	//--------------------------------------------
	// Retrieve the page structure of the user buffer.
	//--------------------------------------------
	down_read(&current->mm->mmap_sem);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0))
	pSgList->nr_pages	= get_user_pages(v_address, pgcount, rw == READ, maplist, NULL);
#else
	pSgList->nr_pages	= get_user_pages(current, current->mm, v_address, pgcount, rw == READ, 0, maplist, NULL);
#endif
	up_read(&current->mm->mmap_sem);
	if (pSgList->nr_pages < 0) {
		pSgList->nr_pages = 0;
		return BM_ERROR_LOCK_MEM;
	}
	//---------------------------
	// Allocate the sg list memory
	//---------------------------
	pSgList->List	= Ccom_alloc_pages(GFP_KERNEL, (pSgList->nr_pages * sizeof(struct scatterlist)));
	if(pSgList->List == NULL){
		return BM_ERROR_MEM;
	}
	//---------------------------
	// Initialize sg List memory with 0
	//---------------------------
	memset(pSgList->List, 0, sizeof(struct scatterlist) * pSgList->nr_pages);

	//------------------------------------------
	// When use only one page
	//------------------------------------------
	if(pSgList->nr_pages == 1){
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
		pSgList->List[0].page		= maplist[0];
#elif ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) || \
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
		pSgList->List[0].page_link	= (unsigned long)maplist[0];
#endif
		pSgList->List[0].offset		= buf_offset;
		pSgList->List[0].length		= buf_length;
	//------------------------------------------
	// When use the multiple pages
	//------------------------------------------
	}else{
		//-----------------------------
		// Set the page 0
		//-----------------------------
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
		pSgList->List[0].page		= maplist[0];
#elif ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) || \
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
		pSgList->List[0].page_link	= (unsigned long)maplist[0];
#endif
		pSgList->List[0].offset		= buf_offset;
		pSgList->List[0].length		= PAGE_SIZE - buf_offset;
		//-----------------------------
		// Set the intermediate page
		//-----------------------------
		for (page_no = 1; page_no < (pSgList->nr_pages - 1); page_no++){
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
			pSgList->List[page_no].page			= maplist[page_no];
#elif ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) || \
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
			pSgList->List[page_no].page_link	= (unsigned long)maplist[page_no];
#endif
			pSgList->List[page_no].offset		= 0;
			pSgList->List[page_no].length		= PAGE_SIZE;
		}
		//-----------------------------
		// Set the last page
		//-----------------------------
		page_no = pSgList->nr_pages - 1;
		if(buf_offset){
			pSgList->List[page_no].length	= (buf_offset + buf_length) % PAGE_SIZE ;
		}else{
			pSgList->List[page_no].length	= PAGE_SIZE;
		}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
		pSgList->List[page_no].page			= maplist[page_no];
#elif ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) || \
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0)))
		pSgList->List[page_no].page_link	= (unsigned long)maplist[page_no];
#endif
		pSgList->List[page_no].offset		= 0;
	}
	//--------------------------------------------
	// Free the maplist (page structure) memory
	//--------------------------------------------
	kfree(maplist);

	return	BM_ERROR_SUCCESS;
}
#endif
//========================================================================
// Function name  : BmMakeSGList
// Function		  : Set the S/G list to hardware
// I/F	 	  	  : Internal
// In	 		  : pMasAddr			: Point to the MASTERADDR structure
//		   		 	dwDir				: BM_DIR_IN / BM_DIR_OUT
// Out			  : None
// Return value   : Normal completed	: BM_ERROR_SUCCESS
//		   			Abnormal completed	: 
// Addition  	  : 
//========================================================================
static long BmSetSGAddress(PMASTERADDR pMasAddr, unsigned long dwDir)
{
	unsigned long		i;
	unsigned short		ListEndRetHead;
	PSGLIST 			pSgList;
	
	//--------------------------------------------
	// Check the parameters
	//--------------------------------------------
	if (dwDir != BM_DIR_IN && dwDir != BM_DIR_OUT) {
		return	BM_ERROR_PARAM;
	}
	if (dwDir == BM_DIR_IN) {
		pSgList	= &pMasAddr->BmInp.SgList;
	} else {
		pSgList	= &pMasAddr->BmOut.SgList;
	}
	// Kernel version 2.4.X
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
	if (pSgList->iobuf->nr_pages > BM_MAX_SG_SIZE) {
#else
	if (pSgList->nr_pages > BM_MAX_SG_SIZE) {
#endif
		return BM_ERROR_BUFF;
	}
	//============================================
	// Set the S/G list to the board
	//============================================
	//--------------------------------------------
	// Start writing
	//--------------------------------------------
	if (dwDir == BM_DIR_IN) {
		BmOutW(BM_PORT_INP_SGWriteStart, 1);
	} else {
		BmOutW(BM_PORT_OUT_SGWriteStart, 1);
	}
	//--------------------------------------------
	// Loop the number of times
	//--------------------------------------------
	ListEndRetHead = 0x0000;
	// Kernel version 2.4.X
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
	for (i=0; i<pSgList->iobuf->nr_pages; i++) {
#else
	for (i=0; i<pSgList->nr_pages; i++) {
#endif
		//--------------------------------------------
		// Set the end flag when the last data list is reached
		//--------------------------------------------
		// Kernel version 2.4.X
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
		if (i == pSgList->iobuf->nr_pages -1) {
#else
		if (i == pSgList->nr_pages -1) {
#endif
			if (pSgList->dwIsRing == BM_WRITE_RING) {
				ListEndRetHead = 0xC000;
			} else {
				ListEndRetHead = 0x8000;
			}
		}
		//--------------------------------------------
		// Set to master register
		//--------------------------------------------
		if (dwDir == BM_DIR_IN) {
			BmOutW(BM_PORT_INP_SGOffset, (unsigned short int)(pSgList->List[i].length | ListEndRetHead));
			BmOutD(BM_PORT_INP_SGAddr,   (unsigned long)sg_dma_address(&pSgList->List[i]));
		} else {
			BmOutW(BM_PORT_OUT_SGOffset, (unsigned short int)(pSgList->List[i].length | ListEndRetHead));
			BmOutD(BM_PORT_OUT_SGAddr,   (unsigned long)sg_dma_address(&pSgList->List[i]));
		}
	}
	//--------------------------------------------
	// SG rewrite completed
	//--------------------------------------------
	if (dwDir == BM_DIR_IN) {
		BmOutW(BM_PORT_INP_SGWriteStart, 0);
	} else {
		BmOutW(BM_PORT_OUT_SGWriteStart, 0);
	}
	return	BM_ERROR_SUCCESS;
}

//========================================================================
// Kernel version 2.4.X
//========================================================================
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
//========================================================================
// Function name  : ApiBmUnlockMem
// Function		  : Unlock the memory.
// I/F			  : External
// In			  : pMasAddr			: Point to the MASTERADDR structure
// 				    dwDir				: Transfer direction (BM_DIR_IN / BM_DIR_OUT)
// Out			  : pMasAddr			: Point to the MASTERADDR structure
// Return value	  : 
// Addition		  : Call at the end of transfer.
//========================================================================
long ApiBmUnlockMem(PMASTERADDR pMasAddr, unsigned long dwDir)
{
	PSGLIST		pSgList;
	int			rw, direction;

	//--------------------------------------------
	// Check the parameters
	//--------------------------------------------
	if (dwDir != BM_DIR_IN && dwDir != BM_DIR_OUT) {
		return BM_ERROR_PARAM;
	}
	//--------------------------------------------
	// Get the SgList structure
	//--------------------------------------------
	if (dwDir == BM_DIR_IN) {
		pSgList	= &pMasAddr->BmInp.SgList;
	} else {
		pSgList	= &pMasAddr->BmOut.SgList;
	}
	//--------------------------------------------
	// Check if the SgList structure is in use
	//--------------------------------------------
	if(pSgList->dwBuffLen == 0){
		return	BM_ERROR_SUCCESS;
	}
	//--------------------------------------------
	// Specify the transfer direction
	//--------------------------------------------
	if(dwDir == BM_DIR_OUT){
		rw			= WRITE;
		direction	= PCI_DMA_TODEVICE;
	}else{
		rw			= READ;
		direction	= PCI_DMA_FROMDEVICE;
	}
	//--------------------------------------------
	// Unmap the scatter list
	//--------------------------------------------
	pci_unmap_sg(&pMasAddr->PciDev, pSgList->List, pSgList->iobuf->nr_pages, direction);
	//---------------------------
	// Free the sg list memory area
	//---------------------------
	Ccom_free_pages((unsigned long)&pSgList->List[0], (pSgList->iobuf->nr_pages * sizeof(struct scatterlist)));
	//---------------------------
	// Unlock the user I/O buffer
	//---------------------------
	unlock_kiovec(rw, &pSgList->iobuf);
	//---------------------------
	// Unmap the user I/O buffer
	//---------------------------
	unmap_kiobuf(pSgList->iobuf);
	//---------------------------
	// Free the I/O vector
	//---------------------------
	free_kiovec(1, &pSgList->iobuf);
	//---------------------------
	// Initialize the size information of user memory
	//---------------------------
	pSgList->dwBuffLen	= 0;
	//---------------------------
	// Initialize the point to the user memory with NULL
	//---------------------------
	pSgList->Buff		= NULL;
	//---------------------------
	// Initialize the transfer direction
	//---------------------------
	pSgList->dwIsRing	= BM_WRITE_ONCE;
	return	BM_ERROR_SUCCESS;
}

//========================================================================
// Kernel version 2.6.X
//========================================================================
#else
//========================================================================
// Function name  : ApiBmStart
// Function		  : Unlock the memory.
// I/F			  : External
// In			  : pMasAddr			: Point to the MASTERADDR structure
// 					dwDir				: Transfer direction (BM_DIR_IN / BM_DIR_OUT)
// Out			  : pMasAddr			: Point to the MASTERADDR structure
// Return value	  : 
// Addition		  : Call at the end of transfer.
//========================================================================
long ApiBmUnlockMem(PMASTERADDR pMasAddr, unsigned long dwDir)
{
	PSGLIST			pSgList;
	int				direction;
	unsigned long	i;

	//--------------------------------------------
	// Check the parameters
	//--------------------------------------------
	if (dwDir != BM_DIR_IN && dwDir != BM_DIR_OUT) {
		return BM_ERROR_PARAM;
	}
	//--------------------------------------------
	// Get the SgList structure
	//--------------------------------------------
	if (dwDir == BM_DIR_IN) {
		pSgList	= &pMasAddr->BmInp.SgList;
	} else {
		pSgList	= &pMasAddr->BmOut.SgList;
	}
	//--------------------------------------------
	// Check if the SgList structure is in use
	//--------------------------------------------
	if(pSgList->dwBuffLen == 0){
		return	BM_ERROR_SUCCESS;
	}
	//--------------------------------------------
	// Specify the transfer direction
	//--------------------------------------------
	if(dwDir == BM_DIR_OUT){
		direction	= PCI_DMA_TODEVICE;
	}else{
		direction	= PCI_DMA_FROMDEVICE;
	}
	//--------------------------------------------
	// Unmap the scatter list
	//--------------------------------------------
	pci_unmap_sg(&pMasAddr->PciDev, pSgList->List, pSgList->nr_pages, direction);
	//---------------------------
	// Unmap the user I/O buffer
	//---------------------------
	for (i = 0; i < pSgList->nr_pages; i++) {
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0) && LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
		page_cache_release(pSgList->List[i].page);
#elif ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24) && LINUX_VERSION_CODE < KERNEL_VERSION(2,7,0)) || \
	    (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,0) && LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0)))
		page_cache_release((struct page*)pSgList->List[i].page_link);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0))
		put_hwpoison_page((struct page*)pSgList->List[i].page_link);
#endif
	}
	//---------------------------
	// Free the sg list memory area
	//---------------------------
	Ccom_free_pages((unsigned long)&pSgList->List[0], (pSgList->nr_pages * sizeof(struct scatterlist)));
	//---------------------------
	// Initialize the number of used pages
	//---------------------------
	pSgList->nr_pages	= 0;
	//---------------------------
	// Initialize the size information of user memory
	//---------------------------
	pSgList->dwBuffLen	= 0;
	//---------------------------
	// Initialize the point to the user memory with NULL
	//---------------------------
	pSgList->Buff		= NULL;
	//---------------------------
	// Initialize the transfer direction
	//---------------------------
	pSgList->dwIsRing	= BM_WRITE_ONCE;
	return	BM_ERROR_SUCCESS;
}
#endif
//========================================================================
// Function name  : ApiBmStart
// Function	 	  : Start the BusMaster transfer.
// I/F			  : External
// In			  : pMasAddr			: Point to the MASTERADDR structure
// 					dwDir				: Transfer direction (BM_DIR_IN, BM_DIR_OUT or their logical sum)
// Out			  : Output to the transfer started board
// Return value	  : Normal completed	: BM_ERROR_SUCCESS
// 					Abnormal completed	: BM_ERROR_PARAM
//========================================================================
long ApiBmStart(PMASTERADDR pMasAddr, unsigned long dwDir)
{
	unsigned long		dwIntMask;
	unsigned short		wStart;
	
	//---------------------------------------------
	// Check the parameters
	//---------------------------------------------
	if (dwDir != BM_DIR_IN &&
		dwDir != BM_DIR_OUT &&
		dwDir != (BM_DIR_IN | BM_DIR_OUT)) {
		return	BM_ERROR_PARAM;
	}
	//---------------------------------------------
	// Check the buffer setting
	//---------------------------------------------
	if (dwDir & BM_DIR_IN) {
		if (pMasAddr->BmInp.SgList.Buff == NULL) {
			return BM_ERROR_SEQUENCE;
		}
	}
	if (dwDir & BM_DIR_OUT) {
		if (pMasAddr->BmOut.SgList.Buff == NULL) {
			return BM_ERROR_SEQUENCE;
		}
	}
	//---------------------------------------------
	// Clear and set the variables
	//---------------------------------------------
	if (dwDir & BM_DIR_IN) {
		pMasAddr->BmInp.dwCarryCount	= 0;
		pMasAddr->BmInp.dwCarryCount2	= 0;
		pMasAddr->BmInp.dwIntSence		= 0;
		pMasAddr->BmInp.dwIntMask = ~(	BM_STATUS_COUNT |
										BM_STATUS_TDCARRY |
										BM_STATUS_FIFO_FULL |
										BM_STATUS_SG_OVER_IN |
										BM_STATUS_END);
	}
	if (dwDir & BM_DIR_OUT) {
		pMasAddr->BmOut.dwCarryCount	= 0;
		pMasAddr->BmOut.dwCarryCount2	= 0;
		pMasAddr->BmOut.dwIntSence		= 0;
		// Only in the case of one time transfer, enable the INTL.
		// In the case that the INTL is enabled, disable the last FIFO_EMPTY.
		if (pMasAddr->BmOut.SgList.dwIsRing == BM_WRITE_ONCE) {
			pMasAddr->BmOut.dwIntMask = ~(	BM_STATUS_COUNT |
											BM_STATUS_INTL |
											BM_STATUS_TDCARRY |
											BM_STATUS_FIFO_EMPTY |
											BM_STATUS_END);
		} else {
			pMasAddr->BmOut.dwIntMask = ~(	BM_STATUS_COUNT |
											BM_STATUS_TDCARRY |
											BM_STATUS_FIFO_EMPTY |
											BM_STATUS_END);
		}
	}
	//---------------------------------------------
	// Clear the interrupt and set the interrupt mask
	//---------------------------------------------
	dwIntMask = (pMasAddr->BmInp.dwIntMask & 0xffff) |
				((pMasAddr->BmOut.dwIntMask << 16) & 0xffff0000);
	BmOutD(BM_PORT_IntMask, dwIntMask);
	BmOutD(BM_PORT_IntClear, ~dwIntMask);
	//---------------------------------------------
	// Start transfer
	// (As not stop the current started transfer, OR with the current value)
	//---------------------------------------------
	BmInpW(BM_PORT_Start_Enable, wStart);
	wStart |= dwDir;
	// Clear the status again at the start
	if ((dwDir & BM_DIR_IN) == 0) {
		pMasAddr->BmInp.dwIntSence		= 0;
		//-------------------------------------
		// Initialize the interrupt count value
		//-------------------------------------
		pMasAddr->BmInp.dwCountSenceNum	= 0;
		pMasAddr->BmInp.dwEndSenceNum	= 0;
	}
	if ((dwDir & BM_DIR_OUT) == 0) {
		pMasAddr->BmOut.dwIntSence		= 0;
		//-------------------------------------
		// Initialize the interrupt count value
		//-------------------------------------
		pMasAddr->BmOut.dwCountSenceNum	= 0;
		pMasAddr->BmOut.dwEndSenceNum	= 0;
	}
	BmOutW(BM_PORT_Start_Enable, wStart);
	return	BM_ERROR_SUCCESS;
}

//========================================================================
// Function name  : ApiBmStop
// Function	      : Force exiting the BusMaster transfer.
// I/F			  : External
// In			  : pMasAddr			: Point to the MASTERADDR structure
// 				    dwDir				: Transfer direction (BM_DIR_IN, BM_DIR_OUT or their logical sum)
// Out			  : Output to the transfer ended board
// Return value	  : Normal completed	: BM_ERROR_SUCCESS
// 					Abnormal completed	: BM_ERROR_PARAM
//========================================================================
long ApiBmStop(PMASTERADDR pMasAddr, unsigned long dwDir)
{
	long			lret;
	unsigned short	wStop;

	//---------------------------------------------
	// Check the parameters
	//---------------------------------------------
	if (dwDir != BM_DIR_IN &&
		dwDir != BM_DIR_OUT &&
		dwDir != (BM_DIR_IN | BM_DIR_OUT)) {
		return	BM_ERROR_PARAM;
	}
	//---------------------------------------------
	// Stop transfer
	//---------------------------------------------
	BmInpW(BM_PORT_Start_Enable, wStop);
	switch(dwDir) {
	case BM_DIR_IN:					wStop &= ~BM_DIR_IN;	break;
	case BM_DIR_OUT:				wStop &= ~BM_DIR_OUT;	break;
	case (BM_DIR_IN | BM_DIR_OUT):	wStop = 0;				break;
	}
	BmOutW(BM_PORT_Start_Enable, wStop);
	lret = BM_ERROR_SUCCESS;
	return	lret;
}

//========================================================================
// Function name  : BmSubGetCount
// Function		  : Retrieve the transfer count after executed ApiBmStart (call synchronized with interrupt)
// I/F			  : Internal
// In			  : 
// Out			  : data				: parameter
// Return value	  : 
//========================================================================
void BmSubGetCount(void *data)
{
	PBM_GET_CNT_SUB	param;
	PMASTERADDR		pMasAddr;
	unsigned long	dwDir, *dwCount, *dwCarry;
	unsigned long	dwDataCount;
	PBMEMBER		pBm;
	unsigned long	dwIntSence;
	unsigned long	dwInSence;
	unsigned long	dwOutSence;
	unsigned long	CarrySave;

	//---------------------------------------------
	// Initialize the variables
	//---------------------------------------------
	param		= (PBM_GET_CNT_SUB)data;
	pMasAddr	= param->pMasAddr;
	dwDir		= param->dwDir;
	dwCount		= param->dwCount;
	dwCarry		= param->dwCarry;
	*dwCarry	= 0;
	//---------------------------------------------
	// Calculate the count value
	//---------------------------------------------
	if (dwDir == BM_DIR_IN) {
		BmInpD(BM_PORT_INP_TotalDataLen, dwDataCount);
		pBm = &pMasAddr->BmInp;
	} else {
		BmInpD(BM_PORT_OUT_TotalDataLen, dwDataCount);
		pBm = &pMasAddr->BmOut;
	}
	//-------------------------------------
	// Calculate the counter carry
	// When the interrupt delay of 100 us occurs
	// at the maximum sampling rate of 50 ns,
	// the counter value of 20000 come up.
	// The carry might be already on as stopping interrupt,
	// check the carry.
	// Since carry is set at 0x00FFFFFF, check also in this case. Ver220
	//-------------------------------------
	if ((dwDataCount < 20000) | (dwDataCount == 0x00FFFFFF)){
		//-------------------------------------
		// Retrieve the interrupt factor
		//-------------------------------------
		BmInpD(BM_PORT_IntStatus, dwIntSence);
		dwInSence	= dwIntSence & 0x7f;
		dwOutSence	= (dwIntSence >> 16) & 0x7f;
		//-------------------------------------
		// Reset the interrupt status
		//-------------------------------------
		if (dwDir == BM_DIR_IN) {
			BmOutD(BM_PORT_IntClear, BM_STATUS_TDCARRY);
		} else {
			BmOutD(BM_PORT_IntClear, BM_STATUS_TDCARRY<<16);
		}
		//-------------------------------------
		// Calculate the counter carry
		//-------------------------------------
		// Input
		if (dwDir == BM_DIR_IN && dwInSence & BM_STATUS_TDCARRY) {
			CarrySave = pMasAddr->BmInp.dwCarryCount;
			pMasAddr->BmInp.dwCarryCount++;
			if (CarrySave > pMasAddr->BmInp.dwCarryCount) {
				pMasAddr->BmInp.dwCarryCount2++;
			}
		}
		// Output
		if (dwDir == BM_DIR_OUT && dwOutSence & BM_STATUS_TDCARRY) {
			CarrySave = pMasAddr->BmOut.dwCarryCount;
			pMasAddr->BmOut.dwCarryCount++;
			if (CarrySave > pMasAddr->BmOut.dwCarryCount) {
				pMasAddr->BmOut.dwCarryCount2++;
			}
		}
	}
	// BM_COUNTER_BIT		24				// Features 25 bit counter
	// BM_COUNTER_MASK		0x00FFFFFF		// 25 bit counter mask
	// BM_CARRY_MASK		0xFF000000		// 7 Bit carry mask
	//-------------------------------------
	// Because the hardware carry is set at 0x00FFFFFF,
	// when the transfer count value is 0x00FFFFFF,
	// the carray count value - 1 is the normal count value
	//-------------------------------------
	//-------------------------------------
	// When the count value is 0x00FFFFFF (24bit)
	//-------------------------------------
	if(dwDataCount == 0x00FFFFFF){
		//-------------------------------------
		// When pBm->dwCarryCount is 0 and the digit of pBm->dwCarryCount2 is counting up
		//-------------------------------------
		if(pBm->dwCarryCount == 0){
			*dwCount = (dwDataCount & BM_COUNTER_MASK) | 
						((pBm->dwCarryCount << BM_COUNTER_BIT) & BM_CARRY_MASK);
			*dwCarry = ((pBm->dwCarryCount >> (32 - BM_COUNTER_BIT)) & BM_COUNTER_MASK) |
						(((pBm->dwCarryCount2 - 1) << BM_COUNTER_BIT) & BM_CARRY_MASK);
		//-------------------------------------
		// When the digit of pBm->dwCarryCount is counting up
		//-------------------------------------
		}else{
			*dwCount = (dwDataCount & BM_COUNTER_MASK) | 
						(((pBm->dwCarryCount - 1) << BM_COUNTER_BIT) & BM_CARRY_MASK);
			*dwCarry = ((pBm->dwCarryCount >> (32 - BM_COUNTER_BIT)) & BM_COUNTER_MASK) |
						((pBm->dwCarryCount2 << BM_COUNTER_BIT) & BM_CARRY_MASK);
		}
	//-------------------------------------
	// Otherwise
	//-------------------------------------
	}else{
		*dwCount = (dwDataCount & BM_COUNTER_MASK) | 
					((pBm->dwCarryCount << BM_COUNTER_BIT) & BM_CARRY_MASK);
					
		*dwCarry = ((pBm->dwCarryCount >> (32 - BM_COUNTER_BIT)) & BM_COUNTER_MASK) |
				   ((pBm->dwCarryCount2 << BM_COUNTER_BIT) & BM_CARRY_MASK);
	}
	return;
}

//========================================================================
// Function name  : ApiBmGetCount
// Function		  : Retrieve the transfer count after executed ApiBmStart
// I/F			  : External
// In			  : pMasAddr			: Point to the MASTERADDR structure
// 				    dwDir				: Transfer direction (1: Mem->FIFO / 2: FIFO->Mem)
// Out			  : *dwCount			: Number of transferred data
// :			    *dwCarry			: Carry count
// Return value	  : Normal completed	: BM_ERROR_SUCCESS
// 					Abnormal completed	: BM_ERROR_PARAM
//========================================================================
long ApiBmGetCount(PMASTERADDR pMasAddr, unsigned long dwDir, unsigned long *dwCount, unsigned long *dwCarry)
{
	BM_GET_CNT_SUB	sub_data;

	//---------------------------------------------
	// Check the parameters
	//---------------------------------------------
	if (dwDir != BM_DIR_IN && dwDir != BM_DIR_OUT) {
		return	BM_ERROR_PARAM;
	}
	//---------------------------------------------
	// Set the parameters
	//---------------------------------------------
	sub_data.pMasAddr	= pMasAddr;
	sub_data.dwDir		= dwDir;
	sub_data.dwCount	= dwCount;
	sub_data.dwCarry	= dwCarry;
	//-------------------------------------
	// Call directly because the interrupt spinlock is on in the upper layer.
	//-------------------------------------
	BmSubGetCount((void *)&sub_data);

	return BM_ERROR_SUCCESS;
}

//========================================================================
// Function name  : ApiBmGetStatus
// Function		  : 
// I/F			  : External
// In			  : pMasAddr			: Point to the MASTERADDR structure
// 					dwDir   			: Transfer direction (BM_DIR_IN or BM_DIR_OUT)
// Out			  : dwStatus 			: Status information
// Return value	  : 
//========================================================================
long ApiBmGetStatus(PMASTERADDR pMasAddr, unsigned long dwDir, unsigned long *dwStatus)
{
	unsigned long	dwSts;
	
	//---------------------------------------------
	// For status, return only the status used by the driver
	// except for those used in the library.
	//---------------------------------------------
	if (dwDir == BM_DIR_IN) {
		dwSts = pMasAddr->BmInp.dwIntSence &(	BM_STATUS_FIFO_EMPTY |
											BM_STATUS_FIFO_FULL |
											BM_STATUS_SG_OVER_IN |
											BM_STATUS_END);
	} else if (dwDir == BM_DIR_OUT) {
		dwSts = pMasAddr->BmOut.dwIntSence &(	BM_STATUS_FIFO_EMPTY |
											BM_STATUS_FIFO_FULL |
											BM_STATUS_END);
	} else {
		return BM_ERROR_PARAM;
	}
	*dwStatus = dwSts;
	return BM_ERROR_SUCCESS;
}

//========================================================================
// Function name  : ApiBmReset
// Function		  : 
// I/F			  : External
// In			  : pMasAddr			: Point to the MASTERADDR structure
// 					dwResetType			: Reset type
// Out			  : 
// Return value	  : 
//========================================================================
long ApiBmReset(PMASTERADDR pMasAddr, unsigned long dwResetType)
{
	//---------------------------------------------
	// Check the parameters
	//---------------------------------------------
	if (dwResetType & ~0x1f) {
		return	BM_ERROR_PARAM;
	}
	//---------------------------------------------
	// Reset
	//---------------------------------------------
	BmOutW(BM_PORT_Reset, (unsigned short)dwResetType);
	if (dwResetType & BM_RESET_ALL) {
		// PIO dependent code
		BmOutD(pMasAddr->dwPortPciAddr + 0x14, 0xffffffff);	// Clear the interrupt
		// PIO dependent code end
		BmOutD(BM_PORT_IntClear, 0xffffffff);				// Clear the interrupt
		//----------------------------------------
		// Initialize the work
		//----------------------------------------
		memset(&pMasAddr->BmInp, 0, sizeof(pMasAddr->BmInp));
		memset(&pMasAddr->BmOut, 0, sizeof(pMasAddr->BmOut));
	}
	return BM_ERROR_SUCCESS;
}

//========================================================================
// Function name  : ApiBmCheckFifo
// Function		  : Return FIFO counter value
// I/F			  : External
// In			  : pMasAddr			: Point to the MASTERADDR structure
// 					dwDir   			: Transfer direction (BM_DIR_IN or BM_DIR_OUT)
// Out			  : 
// Return value	  : Counter value
//========================================================================
long ApiBmCheckFifo(PMASTERADDR pMasAddr, unsigned long dwDir, unsigned long *lpdwFifoCnt)
{
	unsigned long	dwFifoCnt;
	
	//---------------------------------------------
	// Check the parameters
	//---------------------------------------------
	if (dwDir != BM_DIR_IN &&
		dwDir != BM_DIR_OUT) {
		return	BM_ERROR_PARAM;
	}
	//---------------------------------------------
	// Read the counter value
	//---------------------------------------------
	if (dwDir == BM_DIR_IN) {
		BmInpW(BM_PORT_INP_FifoConter, dwFifoCnt);
	} else {
		BmInpW(BM_PORT_OUT_FifoConter, dwFifoCnt);
	}
	*lpdwFifoCnt = dwFifoCnt;
	return BM_ERROR_SUCCESS;
}

//========================================================================
// Function name  : ApiBmGetStartEnabled
// Function		  : Check if the BusMaster transfer is possible
// I/F			  : External
// In			  : pMasAddr			: Point to the MASTERADDR structure
// 					dwDir   			: Transfer direction (BM_DIR_IN or BM_DIR_OUT)
// Out			  : 
// Return value	  : BM_TRANS_ACTIVE: running, BM_TRANS_STOP: stopping
//========================================================================
long ApiBmGetStartEnabled(PMASTERADDR pMasAddr, unsigned long dwDir)
{
	unsigned short	wStart;
	
	//---------------------------------------------
	// Check the parameters
	//---------------------------------------------
	if (dwDir != BM_DIR_IN &&
		dwDir != BM_DIR_OUT) {
		return	BM_ERROR_PARAM;
	}
	//---------------------------------------------
	// Read the start
	//---------------------------------------------
	BmInpW(BM_PORT_Start_Enable, wStart);
	if (wStart & dwDir) return BM_TRANS_ACTIVE;
	return BM_TRANS_STOP;
}

//========================================================================
// Function name  : ApiBmSetNotifyCount
// Function		  : Set the data to notify the busmaster transfer completion with the specified number
// I/F			  : External
// In			  : pMasAddr			: Point to the MASTERADDR structure
// Out			  : 
// Return value	  : Normal completed	: BM_ERROR_SUCCESS 
// 					Abnormal completed	: BM_ERROR_PARAM
//========================================================================
long ApiBmSetNotifyCount(PMASTERADDR pMasAddr, unsigned long dwDir, unsigned long dwCount)
{
	//---------------------------------------------
	// Check the parameters
	//---------------------------------------------
	if (dwDir != BM_DIR_IN &&
		dwDir != BM_DIR_OUT) {
		return	BM_ERROR_PARAM;
	}
	if (dwCount == 0 || dwCount > 0x00ffffff) {
		return	BM_ERROR_PARAM;
	}
	//---------------------------------------------
	// Set the data
	//---------------------------------------------
	if (dwDir == BM_DIR_IN) {
		BmOutD(BM_PORT_INP_IntDataLen, dwCount);
	} else {
		BmOutD(BM_PORT_OUT_IntDataLen, dwCount);
	}
	return BM_ERROR_SUCCESS;
}

//========================================================================
// Function name  : ApiBmInterrupt
// Function		  : Interrupt handling routine
// I/F			  : External
// In			  : pMasAddr			: Point to the MASTERADDR structure
// Out			  : 
// Return value	  : BM_INT_EXIST: own interrupt, BM_INT_NONE: other board interrupt
//========================================================================
long ApiBmInterrupt(PMASTERADDR pMasAddr, unsigned short *InStatus, unsigned short *OutStatus)
{
	long			lret;
	unsigned long	dwIntSence;
	unsigned long	CarrySave;

	//-------------------------------------
	// Retrieve the interrupt factor
	//-------------------------------------
	BmInpD(BM_PORT_IntStatus, dwIntSence);
	//-------------------------------------
	// Reset the interrupt status
	//-------------------------------------
	BmOutD(BM_PORT_IntClear, dwIntSence);
	//-------------------------------------
	// Reset the interrupt status
	//-------------------------------------
	dwIntSence &=  (~pMasAddr->BmInp.dwIntMask & 0x007f) |
				  ((~pMasAddr->BmOut.dwIntMask << 16) & 0x007f0000);
	if (dwIntSence) {
		lret = BM_INT_EXIST;
	} else {
		lret = BM_INT_NONE;
	}
	//-------------------------------------
	// Set the returned interrupt status
	//-------------------------------------
	*InStatus	= (unsigned short)dwIntSence & 0x7f;
	*OutStatus	= (unsigned short)(dwIntSence >> 16) & 0x7f;
	pMasAddr->BmInp.dwIntSence	|= *InStatus;
	pMasAddr->BmOut.dwIntSence	|= *OutStatus;
	//-------------------------------------
	// Calculate the counter carry
	//-------------------------------------
	// Input
	if (pMasAddr->BmInp.dwIntSence & BM_STATUS_TDCARRY) {
		pMasAddr->BmInp.dwIntSence &= ~BM_STATUS_TDCARRY;
		CarrySave = pMasAddr->BmInp.dwCarryCount;
		pMasAddr->BmInp.dwCarryCount++;
		if (CarrySave > pMasAddr->BmInp.dwCarryCount) {
			pMasAddr->BmInp.dwCarryCount2++;
		}
	}
	// Output
	if (pMasAddr->BmOut.dwIntSence & BM_STATUS_TDCARRY) {
		pMasAddr->BmOut.dwIntSence &= ~BM_STATUS_TDCARRY;
		CarrySave = pMasAddr->BmOut.dwCarryCount;
		pMasAddr->BmOut.dwCarryCount++;
		if (CarrySave > pMasAddr->BmOut.dwCarryCount) {
			pMasAddr->BmOut.dwCarryCount2++;
		}
	}
	if(lret == BM_INT_EXIST){
		//-------------------------------------
		// when I/O transfer count notification or input transfer completed
		//-------------------------------------
		//-------------------------------------
		// Increase each count
		//-------------------------------------
		if(*InStatus & BM_STATUS_COUNT){
			pMasAddr->BmInp.dwCountSenceNum++;
		}
		if(*OutStatus & BM_STATUS_COUNT){
			pMasAddr->BmOut.dwCountSenceNum++;
		}
		if(*InStatus & BM_STATUS_END){
			pMasAddr->BmInp.dwEndSenceNum++;
		}
	}
	//-------------------------------------
	// Wake up the interrupt thread
	//-------------------------------------
	if((lret == BM_INT_EXIST) && (
		(pMasAddr->BmInp.dwCountSenceNum != 0) ||
		(pMasAddr->BmOut.dwCountSenceNum != 0) ||
		(pMasAddr->BmInp.dwEndSenceNum != 0))){
		Ccom_wake_up(&pMasAddr->wait_obj);
	}
	return lret;
}

//========================================================================
// Function name  : ApiBmSetStopEvent
// Function		  : Notify the bus master transfer completion of output.
// 					(Count up and wake up the thread)
// I/F			  : External
// In			  : pMasAddr			: Point to the MASTERADDR structure
// 				    dwDir   			: Transfer direction (BM_DIR_IN or BM_DIR_OUT)
// Out		      :
// Return value	  : 
//========================================================================
void ApiBmSetStopEvent(PMASTERADDR pMasAddr, unsigned long dwDir)
{
	if (dwDir == BM_DIR_OUT) {
		//-------------------------------------
		// Count up the transfer completion of output
		//-------------------------------------
		pMasAddr->BmOut.dwEndSenceNum++;
		//-------------------------------------
		// Wake up the thread
		//-------------------------------------
		Ccom_wake_up(&pMasAddr->wait_obj);
	}
}

//========================================================================
// Function name  : BmSubGetIntCount
// Function		  : Copy the count of BusMaster interrupt (call synchronized with interrupt)
// I/F			  : Internal
// In			  : 
// Out			  : data				: parameter
// Return value	  : 
//========================================================================
void BmSubGetIntCount(void *data)
{
	PBM_GET_INT_CNT_SUB	param;
	PBM_GET_INT_CNT		pIntCount;
	PMASTERADDR			pMasAddr;
	
	//-------------------------------------
	// Prepare the variables
	//-------------------------------------
	param		= (PBM_GET_INT_CNT_SUB)data;
	pIntCount	= param->pIntCount;
	pMasAddr	= param->pMasAddr;
	//-------------------------------------
	//	Copy the count value for the return 
	//-------------------------------------
	pIntCount->dwInCountSenceNum	= pMasAddr->BmInp.dwCountSenceNum;
	pIntCount->dwInEndSenceNum		= pMasAddr->BmInp.dwEndSenceNum;
	pIntCount->dwOutCountSenceNum	= pMasAddr->BmOut.dwCountSenceNum;
	pIntCount->dwOutEndSenceNum		= pMasAddr->BmOut.dwEndSenceNum;
	//-------------------------------------
	// Initialize the count value of original data
	//-------------------------------------
	pMasAddr->BmInp.dwCountSenceNum	= 0;
	pMasAddr->BmInp.dwEndSenceNum	= 0;
	pMasAddr->BmOut.dwCountSenceNum	= 0;
	pMasAddr->BmOut.dwEndSenceNum	= 0;

	return;
}

//========================================================================
// Function name  : ApiBmGetIntCount
// Function		  : Retrieve the count of the BusMaster interrupt
// I/F			  : External
// In			  : pMasAddr			: Point to the MASTERADDR structure
// Out			  : IntCount			: Transfer direction (BM_DIR_IN or BM_DIR_OUT)
// Return value	  : Normal completed	: BM_ERROR_SUCCESS
// 					Abnormal completed	: BM_ERROR_PARAM
//========================================================================
long ApiBmGetIntCount(PMASTERADDR pMasAddr, PBM_GET_INT_CNT pIntCount)
{
	BM_GET_INT_CNT_SUB	sub_data;
	
	//---------------------------------------------
	// Check the parameters (Check if it is NULL)
	//---------------------------------------------
	if (pIntCount == NULL) {
		return BM_ERROR_PARAM;
	}
	//---------------------------------------------
	// Set the parameters
	//---------------------------------------------
	sub_data.pMasAddr	= pMasAddr;
	sub_data.pIntCount	= pIntCount;
	//-------------------------------------
	// Call directly because the interrupt spinlock is on in the upper layer.
	//-------------------------------------
	BmSubGetIntCount((void *)&sub_data);
	
	return BM_ERROR_SUCCESS;
}

//========================================================================
// Function name  : ApiBmWaitEvent
// Function		  : Sleep the thread
// I/F			  : External
// In			  : pMasAddr			: Point to the MASTERADDR structure
// Out			  : 
// Return value	  : 
//========================================================================
void ApiBmWaitEvent(PMASTERADDR pMasAddr)
{
	//-------------------------------------
	// Sleep the thread
	// Check whether there is an interrupt
	// Return if there is an interrupt
	//-------------------------------------
	Ccom_sleep_on(pMasAddr->wait_obj, (pMasAddr->BmInp.dwCountSenceNum	|
										pMasAddr->BmInp.dwEndSenceNum	|
										pMasAddr->BmOut.dwCountSenceNum	|
										pMasAddr->BmOut.dwEndSenceNum	));
	return;
}

//========================================================================
// Function name  : ApiBmWakeUpEvent
// Function		  : Wake up the thread
// I/F			  : External
// In			  : pMasAddr			: Point to the MASTERADDR structure
// Out			  : 
// Return value	  : 
//========================================================================
void ApiBmWakeUpEvent(PMASTERADDR pMasAddr)
{
	//-------------------------------------
	// Wake up the thread
	//-------------------------------------
	Ccom_wake_up(&pMasAddr->wait_obj);

	return;
}
#endif	// __BUSMASTER_C__
