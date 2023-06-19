////////////////////////////////////////////////////////////////////////////////
/// @file   Cdio_dm_ctrl.h
/// @brief  API-DIO(LNX) Driver/Shard library common header file (for DM Board)
/// @author &copy;CONTEC CO.,LTD.
/// @since  2002
////////////////////////////////////////////////////////////////////////////////

#ifndef __CDIO_DM_CTRL_H_DEFINED__
#define __CDIO_DM_CTRL_H_DEFINED__

#include <linux/ioctl.h>
//================================================================
// Macro definition
//================================================================


//================================================================
// Error code
//================================================================
#define DIO_ERR_DLL_DIRECTION	10500					// I/O direction is outside of the setting range.
#define DIO_ERR_SYS_DIRECTION	50000					// I/O direction is outside of the setting range.
#define DIO_ERR_SYS_SIGNAL		50001					// Outside the range of the signal.
#define DIO_ERR_SYS_START		50002					// Outside the range of the start conditions.
#define DIO_ERR_SYS_CLOCK		50003					// Clock condition is outside of the setting range.
#define DIO_ERR_SYS_CLOCK_VAL	50004					// Clock value is outside of the setting range.
#define DIO_ERR_SYS_CLOCK_UNIT	50005					// Unit of clock value is outside of the setting range.
#define DIO_ERR_SYS_STOP		50006					// Stop condition is outside of the setting range.
#define DIO_ERR_SYS_STOP_NUM	50007					// Stop number is outside of the setting range.
#define DIO_ERR_SYS_RESET		50008					// Reset content is outside of the setting range.
#define DIO_ERR_SYS_LEN			50009					// Data number is outside of the setting range.
#define DIO_ERR_SYS_RING		50010					// Buffer repetition use setting is outside of the setting range.
#define DIO_ERR_SYS_COUNT		50011					// Data transfer number is outside of the setting range.
#define DIO_ERR_DM_BUFFER		50100					// Buffer was too large to allocate.
#define DIO_ERR_DM_LOCK_MEMORY	50101					// Memory could not be locked.
#define DIO_ERR_DM_PARAM		50102					// Parameter error
#define DIO_ERR_DM_SEQUENCE		50103					// Execution sequence error


//================================================================
// Control structure for DM
//================================================================
// I/O direction setting
typedef struct {
	unsigned long	dir;								// I/O direction
	long			ret;								// Return value
} CDIO_DM_SET_DIR, *PCDIO_DM_SET_DIR;

// Stand alone setting
typedef struct {
	long			ret;								// Return value
} CDIO_DM_SET_STAND_ALONE, *PCDIO_DM_SET_STAND_ALONE;

// Master setting
typedef struct {
	unsigned long	ext_sig1;							// ExtSig1 Output signal
	unsigned long	ext_sig2;							// ExtSig2 Output signal
	unsigned long	ext_sig3;							// ExtSig3 Output signal
	unsigned long	master_halt;						// Master halt
	unsigned long	slave_halt;							// Slave halt
	long			ret;								// Return value
} CDIO_DM_SET_MASTER, *PCDIO_DM_SET_MASTER;

// Slave setting
typedef struct {
	unsigned long	ext_sig1;							// ExtSig1 Enable/Disable setting
	unsigned long	ext_sig2;							// ExtSig2 Enable/Disable setting
	unsigned long	ext_sig3;							// ExtSig3 Enable/Disable setting
	unsigned long	master_halt;						// Master halt
	unsigned long	slave_halt;							// Slave halt
	long			ret;								// Return value
} CDIO_DM_SET_SLAVE, *PCDIO_DM_SET_SLAVE;

// Start condition setting
typedef struct {
	unsigned long	dir;								// Transfer direction
	unsigned long	start;								// Start condition
	long			ret;								// Return value
} CDIO_DM_SET_START_TRG, *PCDIO_DM_SET_START_TRG;

// Pattern matching condition of start condition setting
typedef struct {
	unsigned long	ptn;								// Bit pattern to be compared
	unsigned long	mask;								// Specify mask of the valid bits
	long			ret;								// Return value
} CDIO_DM_SET_START_PTN, *PCDIO_DM_SET_START_PTN;

// Clock condition setting
typedef struct {
	unsigned long	dir;								// Transfer direction
	unsigned long	clock;								// Clock condition
	long			ret;								// Return value
} CDIO_DM_SET_CLOCK_TRG, *PCDIO_DM_SET_CLOCK_TRG;

// Internal clock value setting
typedef struct {
	unsigned long	dir;								// Transfer direction
	unsigned long	clock;								// Clock value
	unsigned long	unit;								// Clock unit
	long			ret;								// Return value
} CDIO_DM_SET_INTERNAL_CLOCK, *PCDIO_DM_SET_INTERNAL_CLOCK;

// Stop condition setting
typedef struct {
	unsigned long	dir;								// Transfer direction
	unsigned long	stop;								// Stop condition
	long			ret;								// Return value
} CDIO_DM_SET_STOP_TRG, *PCDIO_DM_SET_STOP_TRG;

// Stop number setting
typedef struct {
	unsigned long	dir;								// Transfer direction
	unsigned long	stop_num;							// Stop number
	long			ret;								// Return value
} CDIO_DM_SET_STOP_NUM, *PCDIO_DM_SET_STOP_NUM;

// FIFO reset
typedef struct {
	unsigned long	reset;								// Reset content
	long			ret;								// Return value
} CDIO_DM_RESET, *PCDIO_DM_RESET;

// Buffer setting
typedef struct {
	unsigned long	dir;								// Transfer direction
	unsigned long	*buf;								// Buffer
	unsigned long	len;								// Data number
	unsigned long	is_ring;							// Once transfer or Unlimited transfer
	long			ret;								// Return value
} CDIO_DM_SET_BUF, *PCDIO_DM_SET_BUF;

// Sampling/generating start
typedef struct {
	unsigned long	dir;								// Transfer direction
	unsigned long	wait_time_when_output;				// How many milliseconds to wait for data to accumulate in fifo at the start of transfer (for DM)
	long			ret;								// Return value
} CDIO_DM_START, *PCDIO_DM_START;

// Sampling/Generating stop
typedef struct {
	unsigned long	dir;								// Transfer direction
	long			ret;								// Return value
} CDIO_DM_STOP, *PCDIO_DM_STOP;

// Retrieve the transfer status
typedef struct {
	unsigned long	dir;								// Transfer direction
	unsigned long	status;								// Status information
	unsigned long	err;								// Error information
	long			ret;								// Return value
} CDIO_DM_GET_STATUS, *PCDIO_DM_GET_STATUS;

// Retrieve the transfer count
typedef struct {
	unsigned long	dir;								// Transfer direction
	unsigned long	count;								// Count number
	unsigned long	carry;								// Carry count
	long			ret;								// Return value
} CDIO_DM_GET_COUNT, *PCDIO_DM_GET_COUNT;

// Retrieve the transfer count on user buffer
typedef struct {
	unsigned long	dir;								// Transfer direction
	unsigned long	buf_len;							// User buffer size
	unsigned long	count;								// Count number
	unsigned long	carry;								// Carry count
	long			ret;								// Return value
} CDIO_DM_GET_WRITEPOINTER_USER_BUF, *PCDIO_DM_GET_WRITEPOINTER_USER_BUF;

// Retrieve the FIFO count
typedef struct {
	unsigned long	dir;								// Transfer direction
	unsigned long	count;								// Count number
	long			ret;								// Return value
} CDIO_DM_GET_FIFO_COUNT, *PCDIO_DM_GET_FIFO_COUNT;

// Retrieve the BusMaster interrupt count
typedef struct {
	unsigned long	in_count_sence_num;					// Number of event notification count for input
	unsigned long	in_end_sence_num;					// Completion event notification count for input
	unsigned long	out_count_sence_num;				// Number of event notification count for output
	unsigned long	out_end_sence_num;					// Completion event notification count for output
	long			ret;								// Return value
} CDIO_DM_GET_INT_COUNT, *PCDIO_DM_GET_INT_COUNT;

// Specified number transfer completion notification setting
typedef struct {
	unsigned long	dir;								// Transfer direction
	unsigned long	count;								// Count number
	long			ret;								// Return value
} CDIO_DM_SET_COUNT_EVENT, *PCDIO_DM_SET_COUNT_EVENT;

// Post-process at the transfer completion interrupt (local)
typedef struct {
	long			ret;								// Return value
} CDIO_DM_TRANS_END_PROCESS, *PCDIO_DM_TRANS_END_PROCESS;

// Wait in thread for DM
typedef struct {
	long			ret;								// Return value
} CDIO_DM_TH_WAIT, *PCDIO_DM_TH_WAIT;

// Thread wake-up for DM
typedef struct {
	long			ret;								// Return value
} CDIO_DM_TH_WAKE_UP, *PCDIO_DM_TH_WAKE_UP;

//================================================================
// Control code
//================================================================
// Control code for DM
#define	CDIO_IOC_DM_SET_DIR						_IOWR(CDIO_IOC_MAGIC, 50, PCDIO_DM_SET_DIR)						// I/O direction setting
#define	CDIO_IOC_DM_SET_STAND_ALONE				_IOWR(CDIO_IOC_MAGIC, 51, PCDIO_DM_SET_STAND_ALONE)				// Stand alone setting
#define	CDIO_IOC_DM_SET_MASTER					_IOWR(CDIO_IOC_MAGIC, 52, PCDIO_DM_SET_MASTER)					// Master setting
#define	CDIO_IOC_DM_SET_SLAVE					_IOWR(CDIO_IOC_MAGIC, 53, PCDIO_DM_SET_SLAVE)					// Slave setting
#define	CDIO_IOC_DM_SET_START_TRG				_IOWR(CDIO_IOC_MAGIC, 54, PCDIO_DM_SET_START_TRG)				// Start condition setting
#define	CDIO_IOC_DM_SET_START_PTN				_IOWR(CDIO_IOC_MAGIC, 55, PCDIO_DM_SET_START_PTN)				// Pattern matching condition of start condition setting
#define	CDIO_IOC_DM_SET_CLOCK_TRG				_IOWR(CDIO_IOC_MAGIC, 56, PCDIO_DM_SET_CLOCK_TRG)				// Clock condition setting
#define	CDIO_IOC_DM_SET_INTERNAL_CLOCK			_IOWR(CDIO_IOC_MAGIC, 57, PCDIO_DM_SET_INTERNAL_CLOCK)			// Internal clock value setting
#define	CDIO_IOC_DM_SET_STOP_TRG				_IOWR(CDIO_IOC_MAGIC, 58, PCDIO_DM_SET_STOP_TRG)				// Stop condition setting
#define	CDIO_IOC_DM_SET_STOP_NUM				_IOWR(CDIO_IOC_MAGIC, 59, PCDIO_DM_SET_STOP_NUM)				// Stop number setting
#define	CDIO_IOC_DM_RESET						_IOWR(CDIO_IOC_MAGIC, 60, PCDIO_DM_RESET)						// FIFO reset
#define	CDIO_IOC_DM_SET_BUF						_IOWR(CDIO_IOC_MAGIC, 61, PCDIO_DM_SET_BUF)						// Buffer setting
#define	CDIO_IOC_DM_START						_IOWR(CDIO_IOC_MAGIC, 62, PCDIO_DM_START)						// Sampling/generating start
#define	CDIO_IOC_DM_STOP						_IOWR(CDIO_IOC_MAGIC, 63, PCDIO_DM_STOP)						// Sampling/Generating stop
#define	CDIO_IOC_DM_GET_STATUS					_IOWR(CDIO_IOC_MAGIC, 64, PCDIO_DM_GET_STATUS)					// Retrieve the transfer status
#define	CDIO_IOC_DM_GET_COUNT					_IOWR(CDIO_IOC_MAGIC, 65, PCDIO_DM_GET_COUNT)					// Retrieve the transfer count
#define	CDIO_IOC_DM_GET_FIFO_COUNT				_IOWR(CDIO_IOC_MAGIC, 66, PCDIO_DM_GET_FIFO_COUNT)				// Retrieve the FIFO count
#define	CDIO_IOC_DM_GET_INT_COUNT				_IOWR(CDIO_IOC_MAGIC, 67, PCDIO_DM_GET_INT_COUNT)				// Retrieve the BusMaster interrupt count
#define	CDIO_IOC_DM_SET_COUNT_EVENT				_IOWR(CDIO_IOC_MAGIC, 68, PCDIO_DM_SET_COUNT_EVENT)				// Specified number transfer completion notification setting
#define	CDIO_IOC_DM_TRANS_END_PROCESS			_IOWR(CDIO_IOC_MAGIC, 69, PCDIO_DM_TRANS_END_PROCESS)			// Post-process at the transfer completion interrupt (local)
#define	CDIO_IOC_DM_TH_WAIT						_IOWR(CDIO_IOC_MAGIC, 70, PCDIO_DM_TH_WAIT)						// Wait in thread for DM
#define	CDIO_IOC_DM_TH_WAKE_UP					_IOWR(CDIO_IOC_MAGIC, 71, PCDIO_DM_TH_WAKE_UP)					// Thread wake-up for DM
#define	CDIO_IOC_DM_GET_WRITEPOINTER_USER_BUF	_IOWR(CDIO_IOC_MAGIC, 72, PCDIO_DM_GET_WRITEPOINTER_USER_BUF)	// Retrieve the transfer count on user buffer

//================================================================
// Direction
//================================================================
#define PI_32							1
#define PO_32							2
#define PIO_1616						3
#define DIODM_DIR_IN					0x1
#define DIODM_DIR_OUT					0x2
//================================================================
// Start
//================================================================
#define DIODM_START_SOFT				1
#define DIODM_START_EXT_RISE			2
#define DIODM_START_EXT_FALL			3
#define DIODM_START_PATTERN				4
#define DIODM_START_EXTSIG_1			5
#define DIODM_START_EXTSIG_2			6
#define DIODM_START_EXTSIG_3			7
//================================================================
// Clock
//================================================================
#define DIODM_CLK_CLOCK					1
#define DIODM_CLK_EXT_TRG				2
#define DIODM_CLK_HANDSHAKE				3
#define DIODM_CLK_EXTSIG_1				4
#define DIODM_CLK_EXTSIG_2				5
#define DIODM_CLK_EXTSIG_3				6
//================================================================
// Internal Clock
//================================================================
#define	DIODM_TIM_UNIT_S				1
#define	DIODM_TIM_UNIT_MS				2
#define	DIODM_TIM_UNIT_US				3
#define	DIODM_TIM_UNIT_NS				4
//================================================================
// Stop
//================================================================
#define DIODM_STOP_SOFT					1
#define DIODM_STOP_EXT_RISE				2
#define DIODM_STOP_EXT_FALL				3
#define DIODM_STOP_NUM					4
#define DIODM_STOP_EXTSIG_1				5
#define DIODM_STOP_EXTSIG_2				6
#define DIODM_STOP_EXTSIG_3				7
//================================================================
// ExtSig
//================================================================
#define DIODM_EXT_START_SOFT_IN			1
#define DIODM_EXT_STOP_SOFT_IN			2
#define DIODM_EXT_CLOCK_IN				3
#define DIODM_EXT_EXT_TRG_IN			4
#define DIODM_EXT_START_EXT_RISE_IN		5
#define DIODM_EXT_START_EXT_FALL_IN		6
#define DIODM_EXT_START_PATTERN_IN		7
#define DIODM_EXT_STOP_EXT_RISE_IN		8
#define DIODM_EXT_STOP_EXT_FALL_IN		9
#define DIODM_EXT_CLOCK_ERROR_IN		10
#define DIODM_EXT_HANDSHAKE_IN			11
#define	DIODM_EXT_TRNSNUM_IN			12

#define DIODM_EXT_START_SOFT_OUT		101
#define DIODM_EXT_STOP_SOFT_OUT			102
#define DIODM_EXT_CLOCK_OUT				103
#define DIODM_EXT_EXT_TRG_OUT			104
#define DIODM_EXT_START_EXT_RISE_OUT	105
#define DIODM_EXT_START_EXT_FALL_OUT	106
#define DIODM_EXT_STOP_EXT_RISE_OUT		107
#define DIODM_EXT_STOP_EXT_FALL_OUT		108
#define DIODM_EXT_CLOCK_ERROR_OUT		109
#define DIODM_EXT_HANDSHAKE_OUT			110
#define	DIODM_EXT_TRNSNUM_OUT			111
//================================================================
// Status
//================================================================
#define DIODM_STATUS_BMSTOP				0x1
#define DIODM_STATUS_PIOSTART			0x2
#define DIODM_STATUS_PIOSTOP			0x4
#define DIODM_STATUS_TRGIN				0x8
#define DIODM_STATUS_OVERRUN			0x10
//================================================================
// Error
//================================================================
#define DIODM_STATUS_FIFOEMPTY			0x1
#define DIODM_STATUS_FIFOFULL			0x2
#define DIODM_STATUS_SGOVERIN			0x4
#define DIODM_STATUS_TRGERR				0x8
#define DIODM_STATUS_CLKERR				0x10
#define DIODM_STATUS_SLAVEHALT			0x20
#define DIODM_STATUS_MASTERHALT			0x40
//================================================================
// Reset
//================================================================
#define	DIODM_RESET_ALL					0x01
#define	DIODM_RESET_FIFO_IN				0x02
#define	DIODM_RESET_FIFO_OUT			0x04
//================================================================
// Buffer Ring
//================================================================
#define	DIODM_WRITE_ONCE				0
#define	DIODM_WRITE_RING				1

#endif

