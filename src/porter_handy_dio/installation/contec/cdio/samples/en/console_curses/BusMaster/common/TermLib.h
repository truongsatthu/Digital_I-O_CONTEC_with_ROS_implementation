//================================================================
//================================================================
// CONTEC Linux BusMaster Sample for DIO
//			Terminal Operation Library Header File
//													CONTEC Co.,Ltd.
//													Ver1.00
//================================================================
//================================================================
#ifndef __CDM_TERM_LIB_H_DEFINED__
#define __CDM_TERM_LIB_H_DEFINED__

#include <curses.h>

//=================================================
// Define macro
//=================================================
#define	DEC_NUM					0	// Decimal
#define	HEX_NUM					1	// Hexadecimal
#define	MAIN_AREA				0	// Main window
#define	SET_AREA				1	// Setting window
#define	STATUS_AREA_TOP			0	// Top status area
#define	STATUS_AREA_BOTTOM		1	// Bottom status area
#define	MENU_NUM				10	// Number of menu
#define	MENU_TIER				5	// Tier of menu
#define	STATUS_NUM				5	// Maximum of status display
#define	SET_ITEM_MAX_COUNT		20	// Maximum of setting items
#define SELECT_ITEM_MAX_COUNT	30	// Maximum choices

//=================================================
// Define error
//=================================================
#define	TERM_ERR_SUCCESS		0	// Normal complete
#define	TERM_ERR_INIT			1	// Initialization error
#define	TERM_ERR_UN_INIT		2	// No initialization error
#define	TERM_ERR_PARAM			3	// Parameter error
#define	TERM_ERR_INIT_SCR		4	// Failed in creating window
#define	TERM_ERR_DUP_EXEC		5	// Repetitious execution error

//=================================================
// Structure
//=================================================
//=================================================
// Structure of main screen data
//=================================================
//-------------------------
//	Menu area data
//-------------------------
typedef struct{
	char name[256];				// Menu name
}MENU_ITEM, *PMENU_ITEM;
//-------------------------
//	Status area data
//-------------------------
typedef struct{
	char label[256];			// Status's item name
}STATUS_SCR;
//-------------------------
//	Main screen data
//-------------------------
typedef struct{
	char		app_name[256];							// Application name
	char		status_top_name[256];					// Name of top status area
	char		status_buttom_name[256];				// Name of bottom status area
	MENU_ITEM	menu[MENU_TIER][MENU_NUM];				// Menu[tier of menu][number of menu]
	STATUS_SCR	status_top[STATUS_NUM];					// Data of top status area
	STATUS_SCR	status_buttom[STATUS_NUM];				// Data of bottom status area
}MAIN_SCR_DATA, *PMAIN_SCR_DATA;

//=================================================
// Structure of setting screen data
//=================================================
//-------------------------
// Setting data for type of selected item
//-------------------------
typedef struct{
	int		pos_y;										// Whitch part is displayed on the screen of displayed setting value
	char	set_item_name[256];							// Item name
	struct{
		char			name[256];						// Choices
		unsigned long	num;							// Macro for menu
	}item[SELECT_ITEM_MAX_COUNT];
	unsigned long	set_num;							// Setting value(which of chioce)
}SELECT_ITEM_DATA;
//-------------------------
// Setting data for typy of input numerical value
//-------------------------
typedef struct{
	int		pos_y;										// Whitch part is displayed on the screen of displayed setting value 
	char	set_item_name[256];							// Item name
	char	unit_name[256];								// Unit name
	long	set_num;									// Setting value
	int		hex_or_dec;									// HEX_NUM or DEC_NUM
}INPUT_NUM_DATA;
//-------------------------
// Setting screen data
//-------------------------
typedef struct{
	char				window_name[256];				// Setting screen name
	MENU_ITEM			menu[MENU_TIER][MENU_NUM];		// Menu[tier][number of menu]
	SELECT_ITEM_DATA	select_item[SET_ITEM_MAX_COUNT];// Setting data for type of selected item
	INPUT_NUM_DATA		input_num[SET_ITEM_MAX_COUNT];	// Setting data for type of input numerical value
}SET_SCR_DATA, *PSET_SCR_DATA;

//=================================================
// TermLib data structure
//=================================================
//-------------------------
// Window data
//-------------------------
typedef struct{
	WINDOW	*pmain_window;				// Main
	WINDOW	*pcap_window;				// Caption
	WINDOW	*pmenu_window;				// Menu
	WINDOW	*pmessage_window;			// Message
	WINDOW	*psts_top_window;			// Top status
	WINDOW	*psts_bottom_window;		// Bottom status
	WINDOW	*pset_info_window;			// Information display
	short	menu_tier;					// Tier number of menu
}WINDOW_DATA, *PWINDOW_DATA;
//-------------------------
// Library setting data
//-------------------------
typedef struct{
	WINDOW		*std_window;			// Standard window
	WINDOW_DATA	main_window;			// Main window data
	WINDOW_DATA	set_window;				// Setting window data
	WINDOW_DATA	*active_window;			// Pointer of active window data
	void		(*org_resize)(int);		// Function for processing signal for resizing terminal in curses
	int			sem_id;					// Semaphore identifier
}TERM_DATA, *PTERM_DATA;

//================================================================
// Function prototype
//================================================================
#ifdef __cplusplus
extern"C"{
#endif
	long TermInit(PMAIN_SCR_DATA main_scr);
	long TermExit(void);
	long TermMenuSet(PMENU_ITEM menu);
	long TermGetMenuNum(char *message, int *menu_num);
	long TermGetStr(char *message, char *get_str);
	long TermSetStatus(int sts_area, int pos_y, char *status_str);
	long TermSetWindowOpen(PSET_SCR_DATA set_scr);
	long TermSetWindowClose(void);
	long TermGetSelectItem(PSET_SCR_DATA set_scr, int item_num, unsigned long *select_num);
	long TermGetInputNum(PSET_SCR_DATA set_scr, int item_num, unsigned long *input_num);
#ifdef __cplusplus
}
#endif
#endif
