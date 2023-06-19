//================================================================
//================================================================
// CONTEC Linux BusMaster Sample for DIO
//			Terminal Operation Library Source File
//													CONTEC Co.,Ltd.
//													Ver1.00
//================================================================
//================================================================

#include "TermLib.h"
#include <stdio.h>
#include <string.h>
#include <curses.h>
#include <signal.h>
#include <sys/types.h>
#include <linux/ipc.h>
#include <linux/sem.h>

//=================================================
// Define macro
//=================================================
#define	SCR_LENGTH				80
#define	SCR_HIGHT				25
#define	SCR_LINE_WIDTH			1
#define	MENU_AREA_LENGTH		15
#define	SCR_CAP_HIGHT			1
#define	MAIN_SCR_STS_HIGHT		7
#define	SET_SCR_INFO_HIGHT		10
#define	MAIN_SCR_MSG_HIGHT		SCR_HIGHT - SCR_CAP_HIGHT - MAIN_SCR_STS_HIGHT*2 - SCR_LINE_WIDTH*5
#define	SET_SCR_MSG_HIGHT		SCR_HIGHT - SCR_CAP_HIGHT - SET_SCR_INFO_HIGHT - SCR_LINE_WIDTH*4
#define	SCR_MSG_LENGTH			SCR_LENGTH - MENU_AREA_LENGTH - SCR_LINE_WIDTH * 3
#define	MSG_AREA_START_POS_X	MENU_AREA_LENGTH + SCR_LINE_WIDTH * 2
#define	MAIN_STS_OFFSET			10
#define	INFO_STR_NAME_OFFSET	1
#define	INFO_STR_OFFSET			16
#define	INFO_NUM_NAME_OFFSET	32
#define	INFO_NUM_OFFSET			51


//=================================================
// Declare global data
//=================================================
TERM_DATA	term_data;

//=================================================
// Declare function prototype
//=================================================
static long TermWindowCreate(int area);
static long TermSetInfoWindow(PSET_SCR_DATA set_scr);

//=================================================
// Process signal for resizing terminal
//=================================================
void TermResize(int sig)
{
	//--------------------------------------
	// Call signal processing in curses
	//--------------------------------------
	if (term_data.org_resize != NULL) {
		term_data.org_resize(sig);
	}
	//--------------------------------------
	// Refresh display
	//--------------------------------------
	if (term_data.active_window->pmain_window != NULL){
		prefresh(term_data.active_window->pmain_window, 0, 0, 0, 0, LINES - 1, COLS - 1);
	}
}

//================================================================
// Set Semaphore
//================================================================
static int SetSemaphore(void)
{
	union semun sem_union;

	term_data.sem_id	= semget((key_t)1000, 1, 0666 | IPC_CREAT);
	sem_union.val		= 1;
	if (semctl(term_data.sem_id, 0, SETVAL, sem_union) == -1){
		return TERM_ERR_INIT;
	}
	return TERM_ERR_SUCCESS;
}

//================================================================
// Delete Semaphore
//================================================================
static void DelSemaphore(void)
{
	union semun sem_union;

	semctl(term_data.sem_id, 0, IPC_RMID, sem_union);
}

//================================================================
// P semaphore operation
//================================================================
static void SemaphoreP(void)
{
	struct sembuf sem_b;

	sem_b.sem_num	= 0;
	sem_b.sem_op	= -1;
	sem_b.sem_flg	= SEM_UNDO;
	semop(term_data.sem_id, &sem_b, 1);
}

//================================================================
// V semaphore operation
//================================================================
static void SemaphoreV(void)
{
	struct sembuf sem_b;

	sem_b.sem_num	= 0;
	sem_b.sem_op	= 1;
	sem_b.sem_flg	= SEM_UNDO;
	semop(term_data.sem_id, &sem_b, 1);
}

//================================================================
// Function for creating window
//================================================================
static long TermWindowCreate(int area)
{
	PWINDOW_DATA	pwnd;

	//--------------------------------------
	// Select structure of creating window
	//--------------------------------------
	if(area == MAIN_AREA){
		pwnd	= &term_data.main_window;
	}else{
		pwnd	= &term_data.set_window;
	}
	//--------------------------------------
	// Create main screen
	//--------------------------------------
	pwnd->pmain_window = newpad(SCR_HIGHT, SCR_LENGTH);
	// Check error
	if(pwnd->pmain_window == NULL){
		return TERM_ERR_INIT_SCR;
	}
	//--------------------------------------
	// Create sub screen
	//--------------------------------------
	// Create caption area
	pwnd->pcap_window			= subpad(pwnd->pmain_window, SCR_CAP_HIGHT, SCR_LENGTH, 1, 0);
	// Create menu area
	pwnd->pmenu_window			= subpad(pwnd->pmain_window, 
										SCR_HIGHT - SCR_CAP_HIGHT - SCR_LINE_WIDTH * 3, MENU_AREA_LENGTH, 
										SCR_CAP_HIGHT + SCR_LINE_WIDTH * 2, SCR_LINE_WIDTH);
	// Check error
		if(pwnd->pcap_window == NULL || pwnd->pmenu_window == NULL){
		return TERM_ERR_INIT_SCR;
	}
	//--------------------------------------
	// Main window
	//--------------------------------------
	if(area == MAIN_AREA){
		// Create message area
		pwnd->pmessage_window		= subpad(pwnd->pmain_window, MAIN_SCR_MSG_HIGHT, SCR_MSG_LENGTH,
											SCR_CAP_HIGHT + SCR_LINE_WIDTH * 2, MSG_AREA_START_POS_X);
		// Create top status area
		pwnd->psts_top_window		= subpad(pwnd->pmain_window, MAIN_SCR_STS_HIGHT,  SCR_MSG_LENGTH,
											SCR_HIGHT - MAIN_SCR_STS_HIGHT * 2 - SCR_LINE_WIDTH * 2, MSG_AREA_START_POS_X);
		// create bottom status area
		pwnd->psts_bottom_window	= subpad(pwnd->pmain_window, MAIN_SCR_STS_HIGHT,  SCR_MSG_LENGTH,
											SCR_HIGHT - MAIN_SCR_STS_HIGHT - SCR_LINE_WIDTH, MSG_AREA_START_POS_X);
		// Check error
		if(	pwnd->pmessage_window == NULL || pwnd->psts_top_window == NULL || pwnd->psts_bottom_window == NULL){
		return TERM_ERR_INIT_SCR;
		}
	//--------------------------------------
	// Setting window
	//--------------------------------------
	}else{
		// Create message area
		pwnd->pmessage_window		= subpad(pwnd->pmain_window, MAIN_SCR_MSG_HIGHT, SCR_MSG_LENGTH,
											SCR_CAP_HIGHT + SCR_LINE_WIDTH * 2, MSG_AREA_START_POS_X);
		// Create setting information area
		pwnd->pset_info_window		= subpad(pwnd->pmain_window, SET_SCR_INFO_HIGHT, SCR_MSG_LENGTH,
											SCR_HIGHT - SET_SCR_INFO_HIGHT - SCR_LINE_WIDTH, MSG_AREA_START_POS_X);
		// Check error
		if(	pwnd->pmessage_window == NULL || pwnd->pset_info_window == NULL){
		return TERM_ERR_INIT_SCR;
		}
	}
	//--------------------------------------
	// Create line
	//--------------------------------------
	//--------------------------------------
	// All frames
	//--------------------------------------
	box(pwnd->pmain_window,ACS_VLINE, ACS_HLINE);
	//--------------------------------------
	// Horizontal line
	//--------------------------------------
	mvwhline(pwnd->pmain_window, SCR_CAP_HIGHT + SCR_LINE_WIDTH, SCR_LINE_WIDTH, ACS_HLINE, SCR_LENGTH - SCR_LINE_WIDTH * 2);
	// Main window
	if(area == MAIN_AREA){
		mvwhline(pwnd->pmain_window, 
				SCR_CAP_HIGHT + MAIN_SCR_MSG_HIGHT + SCR_LINE_WIDTH * 2, MENU_AREA_LENGTH + SCR_LINE_WIDTH * 2, 
				ACS_HLINE, SCR_LENGTH - MENU_AREA_LENGTH - SCR_LINE_WIDTH * 3);
		mvwhline(pwnd->pmain_window, 
				SCR_HIGHT - MAIN_SCR_STS_HIGHT - SCR_LINE_WIDTH * 2, MENU_AREA_LENGTH + SCR_LINE_WIDTH * 2, 
				ACS_HLINE, SCR_LENGTH - MENU_AREA_LENGTH - SCR_LINE_WIDTH * 3);
	// Setting window
	}else{
		mvwhline(pwnd->pmain_window, 
				SCR_CAP_HIGHT + SET_SCR_MSG_HIGHT + SCR_LINE_WIDTH * 2, MENU_AREA_LENGTH + SCR_LINE_WIDTH * 2, 
				ACS_HLINE, SCR_LENGTH - MENU_AREA_LENGTH - SCR_LINE_WIDTH * 3);
	}
	//--------------------------------------
	// Vertical line
	//--------------------------------------
	mvwvline(pwnd->pmain_window, 
			SCR_CAP_HIGHT + SCR_LINE_WIDTH * 2, MENU_AREA_LENGTH + SCR_LINE_WIDTH, 
			ACS_VLINE, SCR_HIGHT - SCR_CAP_HIGHT - SCR_LINE_WIDTH * 3);

	return TERM_ERR_SUCCESS;
}

//================================================================
// Function for initializing routine
//================================================================
long TermInit(PMAIN_SCR_DATA main_scr)
{
	char	buf[256];
	int		count, pos_y;
	int		lret;
	//--------------------------------------
	// Check parameter
	//--------------------------------------
	if(main_scr == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// Initialize global data
	//--------------------------------------
	memset(&term_data, 0, sizeof(TERM_DATA));
	//--------------------------------------
	// Retrieve tier number of menu
	//--------------------------------------
	for(count = 0; strcmp(main_scr->menu[count][0].name, "") && count < MENU_NUM; count++){}
	term_data.main_window.menu_tier	= count;
	//--------------------------------------
	// Set semaphore
	//--------------------------------------
	lret = SetSemaphore();
	if(lret != TERM_ERR_SUCCESS){
		return lret;
	}
	//--------------------------------------
	// Initialize curses
	//--------------------------------------
	term_data.std_window = initscr();
	if(term_data.std_window == NULL){
		return TERM_ERR_INIT;
	}
	//--------------------------------------
	// Set curses mode
	//--------------------------------------
	cbreak();
	curs_set(0);
	//--------------------------------------
	// Creat main window
	//--------------------------------------
	lret = TermWindowCreate(MAIN_AREA);
	if(lret != TERM_ERR_SUCCESS){
		//--------------------------------------
		// Terminate curses
		//--------------------------------------
		endwin();
		return lret;
	}
	//--------------------------------------
	// Register active window
	//--------------------------------------
 	term_data.active_window	= &term_data.main_window;
	//--------------------------------------
	// Register signal for resizing window
	//--------------------------------------
	term_data.org_resize	= signal(SIGWINCH, TermResize);
	//--------------------------------------
	// Set initial displayed data
	//--------------------------------------
	// Set window name
	mvwaddstr(term_data.main_window.pcap_window, 0, 3, main_scr->app_name);
	// Set name of top status area
	mvwaddstr(term_data.main_window.pmain_window,
				SCR_CAP_HIGHT + MAIN_SCR_MSG_HIGHT + SCR_LINE_WIDTH * 2, MENU_AREA_LENGTH + SCR_LINE_WIDTH * 2 + 1, 
				main_scr->status_top_name);
	// Set name of bottom status area
	mvwaddstr(term_data.main_window.pmain_window, 
				SCR_HIGHT - MAIN_SCR_STS_HIGHT - SCR_LINE_WIDTH * 2, MENU_AREA_LENGTH + SCR_LINE_WIDTH * 2 + 1, 
				main_scr->status_buttom_name);
	// Set displayed menu
	TermMenuSet(main_scr->menu[0]);
	// Set top status display
	for(count = 0, pos_y = 1; strcmp(main_scr->status_top[count].label, "") && count < STATUS_NUM; count++, pos_y++){
		sprintf(buf, "%s:", main_scr->status_top[count].label);
		mvwaddstr(term_data.main_window.psts_top_window, pos_y, 1, buf);
	}
	// Set bottom status display
	for(count = 0, pos_y = 1; strcmp(main_scr->status_buttom[count].label, "") && count < STATUS_NUM; count++, pos_y++){
		sprintf(buf, "%s:", main_scr->status_buttom[count].label);
		mvwaddstr(term_data.main_window.psts_bottom_window, pos_y, 1, buf);
	}
	wmove(term_data.main_window.pmain_window,0,0);
	//--------------------------------------
	// Display displayed data to screen
	//--------------------------------------
	prefresh(term_data.main_window.pmain_window, 0, 0, 0, 0, LINES-1, COLS-1);
	
	return TERM_ERR_SUCCESS;
}

//================================================================
// Function for terminating routint
//================================================================
long TermExit()
{
	//--------------------------------------
	// Check initialization
	//--------------------------------------
	if(term_data.std_window == NULL){
		return TERM_ERR_UN_INIT;
	}
	//--------------------------------------
	// Delete window
	//--------------------------------------
	delwin(term_data.main_window.psts_bottom_window);
	delwin(term_data.main_window.psts_top_window);
	delwin(term_data.main_window.pmessage_window);
	delwin(term_data.main_window.pmenu_window);
	delwin(term_data.main_window.pcap_window);
	delwin(term_data.main_window.pmain_window);
	//--------------------------------------
	// Set curses mode
	//--------------------------------------
	nocbreak();
	//--------------------------------------
	// Terminate curses
	//--------------------------------------
	endwin();
	//--------------------------------------
	// Delete semaphore
	//--------------------------------------
	DelSemaphore();
	//--------------------------------------
	// Initialize global data
	//--------------------------------------
	memset(&term_data, 0, sizeof(TERM_DATA));
	
	return TERM_ERR_SUCCESS;
}

//================================================================
// Function for setting menu
//================================================================
long TermMenuSet(PMENU_ITEM menu)
{
	int  count, pos_y;
	char buf[256];
	
	//--------------------------------------
	// Clear menu area
	//--------------------------------------
	wclear(term_data.active_window->pmenu_window);
	//--------------------------------------
	// Set displayed menu
	//--------------------------------------
	for(count = 0, pos_y = 1; strcmp(menu[count].name, "") && count < MENU_NUM; count++, pos_y ++){
		sprintf(buf, "%d.%s", count, menu[count].name);
		mvwaddstr(term_data.active_window->pmenu_window, pos_y, 1, buf);
	}
	//--------------------------------------
	// Display displayed data to screen
	//--------------------------------------
	prefresh(term_data.active_window->pmain_window, 0, 0, 0, 0, LINES-1, COLS-1);

	return TERM_ERR_SUCCESS;
}

//================================================================
// Function for menu number acquisition
//================================================================
long TermGetMenuNum(char *message, int *menu_num)
{
	char get_char, buf[10];
	//--------------------------------------
	// Check initialization
	//--------------------------------------
	if(term_data.std_window == NULL){
		return TERM_ERR_UN_INIT;
	}
	//--------------------------------------
	// Check parameter
	//--------------------------------------
	if(message == NULL || menu_num == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// Clear message area
	//--------------------------------------
	wclear(term_data.active_window->pmessage_window);
	//--------------------------------------
	// Display message
	//--------------------------------------
	mvwaddstr(term_data.active_window->pmessage_window, 0, 1, message);
	//--------------------------------------
	// Clear input buffer
	//--------------------------------------
	flushinp();
	//--------------------------------------
	// Display displayed data to screen
	//--------------------------------------
	prefresh(term_data.active_window->pmain_window, 0, 0, 0, 0, LINES-1, COLS-1);
	//--------------------------------------
	// Input number(0-9)
	//--------------------------------------
	do{
		get_char = wgetch(term_data.active_window->pmessage_window);
		buf[0] = get_char;
		buf[1] = '\0';
		*menu_num = atoi(buf);
	}while((*menu_num == 0) && (get_char != '0'));
	
	return TERM_ERR_SUCCESS;
}

//================================================================
// Function for retrieving string
//================================================================
long TermGetStr(char *message, char *get_str)
{
	int y, x;
	//--------------------------------------
	// Check initialization
	//--------------------------------------
	if(term_data.std_window == NULL){
		return TERM_ERR_UN_INIT;
	}
	//--------------------------------------
	// Check parameter
	//--------------------------------------
	if(message == NULL || get_str == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// Clear message area
	//--------------------------------------
	wclear(term_data.active_window->pmessage_window);
	//--------------------------------------
	// Display message
	//--------------------------------------
	mvwaddstr(term_data.active_window->pmessage_window, 0, 1, message);
	waddstr(term_data.active_window->pmessage_window, "\n :");
	//--------------------------------------
	// Display displayed data to screen
	//--------------------------------------
	prefresh(term_data.active_window->pmain_window, 0, 0, 0, 0, LINES-1, COLS-1);
	//--------------------------------------
	// Copy current screen to standard screen
	// Because input string is not diaplayed on pad
	//--------------------------------------
	overwrite(term_data.active_window->pmain_window, term_data.std_window);
	getyx(term_data.active_window->pmessage_window, y, x);
	y = SCR_CAP_HIGHT + SCR_LINE_WIDTH * 2 + y;
	x = MSG_AREA_START_POS_X + x + 1;
	move(y, x);
	//--------------------------------------
	// Input string
	//--------------------------------------
	curs_set(1);
	getstr(get_str);
	curs_set(0);
	
	return TERM_ERR_SUCCESS;
}

//================================================================
// Function for setting status string
//================================================================
long TermSetStatus(int sts_area, int pos_y, char *status_str)
{
	//--------------------------------------
	// P semaphore operation
	//--------------------------------------
	SemaphoreP();
	//--------------------------------------
	// Check initialization
	//--------------------------------------
	if(term_data.std_window == NULL){
		return TERM_ERR_UN_INIT;
	}
	//--------------------------------------
	// Check parameter
	//--------------------------------------
	if(status_str == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// Set status string
	//--------------------------------------
	if(sts_area == STATUS_AREA_TOP){
		mvwaddstr(term_data.main_window.psts_top_window, pos_y, MAIN_STS_OFFSET, status_str);
		wclrtoeol(term_data.main_window.psts_top_window);
	}else{
		mvwaddstr(term_data.main_window.psts_bottom_window, pos_y, MAIN_STS_OFFSET, status_str);
		wclrtoeol(term_data.main_window.psts_bottom_window);
	}
	//--------------------------------------
	// Display displayed data to screen
	//--------------------------------------
	prefresh(term_data.main_window.pmain_window, 0, 0, 0, 0, LINES-1, COLS-1);
	//--------------------------------------
	// V semaphore operation
	//--------------------------------------
	SemaphoreV();

	return TERM_ERR_SUCCESS;
}

//================================================================
// Function for initializing setting screen
//================================================================
long TermSetWindowOpen(PSET_SCR_DATA set_scr)
{
	char buf[256];
	int count, pos_y, lret;
	
	//--------------------------------------
	// Check initialization
	//--------------------------------------
	if(term_data.std_window == NULL){
		return TERM_ERR_UN_INIT;
	}
	//--------------------------------------
	// Repetitious execution error
	//--------------------------------------
	if(term_data.set_window.pmain_window != NULL){
		return TERM_ERR_DUP_EXEC;
	}
	//--------------------------------------
	// Check parameter
	//--------------------------------------
	if(set_scr == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// Retrieve tier number of menu
	//--------------------------------------
	for(count = 0; strcmp(set_scr->menu[count][0].name, "") || count < MENU_NUM; count++){}
	term_data.set_window.menu_tier	= count;
	//--------------------------------------
	// Create setting screen
	//--------------------------------------
	lret = TermWindowCreate(SET_AREA);
	if(lret != TERM_ERR_SUCCESS){
		return lret;
	}
	//--------------------------------------
	// Register active window
	//--------------------------------------
	term_data.active_window	= &term_data.set_window;
	//--------------------------------------
	// Set initial displayed data
	//--------------------------------------
	// Set window name
	mvwaddstr(term_data.set_window.pcap_window, 0, 3, set_scr->window_name);
	// Set displayed menu
	TermMenuSet(set_scr->menu[0]);
	//--------------------------------------
	// Display information of setting screen
	//--------------------------------------
	TermSetInfoWindow(set_scr);
	//--------------------------------------
	// Move cursor at the upper left
	//--------------------------------------
	wmove(term_data.active_window->pmain_window, 0, 0);
	//--------------------------------------
	// Display displayed data to screen
	//--------------------------------------
	prefresh(term_data.active_window->pmain_window, 0, 0, 0, 0, LINES-1, COLS-1);

	return TERM_ERR_SUCCESS;
}

//================================================================
// Function for terminating setting screen
//================================================================
long TermSetWindowClose()
{
	//--------------------------------------
	// Check initialization
	//--------------------------------------
	if(term_data.std_window == NULL ||
		term_data.set_window.pmain_window == NULL){
		return TERM_ERR_UN_INIT;
	}
	//--------------------------------------
	// Register active window
	//--------------------------------------
 	term_data.active_window	= &term_data.main_window;
	//--------------------------------------
	// Delete window
	//--------------------------------------
	delwin(term_data.set_window.pset_info_window);
	delwin(term_data.set_window.pmessage_window);
	delwin(term_data.set_window.pmenu_window);
	delwin(term_data.set_window.pcap_window);
	delwin(term_data.set_window.pmain_window);
	//--------------------------------------
	// Initialize structure of setting screen window
	//--------------------------------------
	memset(&term_data.set_window, 0, sizeof(WINDOW_DATA));
	//--------------------------------------
	// Display displayed data to screen
	//--------------------------------------
	prefresh(term_data.main_window.pmain_window, 0, 0, 0, 0, LINES-1, COLS-1);

	return TERM_ERR_SUCCESS;
}

//================================================================
// Function for displaying information of setting screen
//================================================================
static long TermSetInfoWindow(PSET_SCR_DATA set_scr)
{
	SELECT_ITEM_DATA	*pitem_data;
	INPUT_NUM_DATA		*pnum_data;
	char buf[256];
	int count;
	
	//--------------------------------------
	// Check initialization
	//--------------------------------------
	if(term_data.std_window == NULL ||
		term_data.set_window.pmain_window == NULL){
		return TERM_ERR_UN_INIT;
	}
	//--------------------------------------
	// Check parameter
	//--------------------------------------
	if(set_scr == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// Clear message area
	//--------------------------------------
	wclear(term_data.set_window.pset_info_window);
	//--------------------------------------
	// Display setting information of selected item's type
	//--------------------------------------
	for(count = 0; strcmp(set_scr->select_item[count].set_item_name, "") && count < SET_ITEM_MAX_COUNT; count++){
		pitem_data	= &set_scr->select_item[count];
		sprintf(buf, "%s:", pitem_data->set_item_name);
		mvwaddstr(term_data.set_window.pset_info_window, count, INFO_STR_NAME_OFFSET, buf);
		strcpy(buf, pitem_data->item[pitem_data->set_num].name);
		mvwaddstr(term_data.set_window.pset_info_window, count, INFO_STR_OFFSET, buf);
	}
	//--------------------------------------
	// Display setting information of input numerical value's type
	//--------------------------------------
	for(count = 0; strcmp(set_scr->input_num[count].set_item_name, "") && count < SET_ITEM_MAX_COUNT;  count++){
		pnum_data	= &set_scr->input_num[count];
		sprintf(buf, "%s:", pnum_data->set_item_name);
		mvwaddstr(term_data.set_window.pset_info_window, count, INFO_NUM_NAME_OFFSET, buf);
		//--------------------------------------
		// Display type on decimal
		//--------------------------------------
		if(pnum_data->hex_or_dec == DEC_NUM){
			if(strcmp(pnum_data->unit_name, "")){
				sprintf(buf, "%d [%s]", pnum_data->set_num, pnum_data->unit_name);
			}else{
				sprintf(buf, "%d", pnum_data->set_num);
			}
		//--------------------------------------
		// Display type on hexadecimal
		//--------------------------------------
		}else{
			if(strcmp(pnum_data->unit_name, "")){
				sprintf(buf, "%X [%s]", pnum_data->set_num, pnum_data->unit_name);
			}else{
				sprintf(buf, "%X", pnum_data->set_num);
			}
		}
		mvwaddstr(term_data.set_window.pset_info_window, count, INFO_NUM_OFFSET, buf);
	}
	//--------------------------------------
	// Display displayed screen to screen
	//--------------------------------------
	prefresh(term_data.set_window.pmain_window, 0, 0, 0, 0, LINES-1, COLS-1);

	return TERM_ERR_SUCCESS;
}	

//================================================================
// Function for retrieving setting content(select item)
//================================================================
long TermGetSelectItem(PSET_SCR_DATA set_scr, int item_num, unsigned long *select_num)
{
	char buf[1000];
	int input_num, count, max_count, y, x;
	//--------------------------------------
	// Check initialization
	//--------------------------------------
	if(term_data.std_window == NULL ||
		term_data.set_window.pmain_window == NULL){
		return TERM_ERR_UN_INIT;
	}
	//--------------------------------------
	// Check parameter
	//--------------------------------------
	if(set_scr == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// Display item name
	//--------------------------------------
	sprintf(buf, "< %s >\n", set_scr->select_item[item_num].set_item_name);
	mvwaddstr(term_data.set_window.pmessage_window, 0, 0, buf);
	//--------------------------------------
	// Display question
	//--------------------------------------
	waddstr(term_data.set_window.pmessage_window, "Please choose from the following choice.\n");
	//--------------------------------------
	// Display selected item
	//--------------------------------------
	strcpy(buf, "\n");
	for(count = 0; strcmp(set_scr->select_item[item_num].item[count].name, "") && count < SELECT_ITEM_MAX_COUNT; count++){
		sprintf(buf, "%s%d. %s  ", buf, count + 1, set_scr->select_item[item_num].item[count].name);
	}
	// Save maximum of items
	max_count = count;
	waddstr(term_data.set_window.pmessage_window, buf);
	// Display prompt
	waddstr(term_data.set_window.pmessage_window, " > ");
	// Display displayed data to screen
	prefresh(term_data.set_window.pmain_window, 0, 0, 0, 0, LINES-1, COLS-1);
	//--------------------------------------
	// Copy current screen to standard screen
	// Because input string is not diaplayed on pad
	//--------------------------------------
	overwrite(term_data.set_window.pmain_window, term_data.std_window);
	getyx(term_data.set_window.pmessage_window, y, x);
	y = SCR_CAP_HIGHT + SCR_LINE_WIDTH * 2 + y;
	x = MSG_AREA_START_POS_X + x + 1;
	for(;;){
		//--------------------------------------
		// Input answer
		//--------------------------------------
		move(y, x);
		refresh();
		curs_set(1);
		getstr(buf);
		curs_set(0);
		input_num = atoi(buf);
		if(0 < input_num && input_num <=  max_count){
			break;
		}
	}
	//--------------------------------------
	// Set input value to setting screen data
	//--------------------------------------
	set_scr->select_item[item_num].set_num = input_num - 1;
	//--------------------------------------
	// Set return data
	//--------------------------------------
	*select_num	= set_scr->select_item[item_num].item[input_num - 1].num;
	//--------------------------------------
	// Clear message area
	//--------------------------------------
	wclear(term_data.set_window.pmessage_window);
	//--------------------------------------
	// Display setting information
	//--------------------------------------
	TermSetInfoWindow(set_scr);

	return TERM_ERR_SUCCESS;
}

//================================================================
// Function for retrieving setting content(input numerical value)
//================================================================
long TermGetInputNum(PSET_SCR_DATA set_scr, int item_num, unsigned long *input_num)
{
	char buf[1000];
	int count, y, x;
	//--------------------------------------
	// Check initialization
	//--------------------------------------
	if(term_data.std_window == NULL ||
		term_data.set_window.pmain_window == NULL){
		return TERM_ERR_UN_INIT;
	}
	//--------------------------------------
	// Check parameter
	//--------------------------------------
	if(set_scr == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// Display item name
	//--------------------------------------
	sprintf(buf, "< %s >\n", set_scr->input_num[item_num].set_item_name);
	mvwaddstr(term_data.set_window.pmessage_window, 0, 0, buf);
	//--------------------------------------
	// Display question
	//--------------------------------------
	waddstr(term_data.set_window.pmessage_window, "Please input a setting value.\n");
	if(strcmp(set_scr->input_num[item_num].unit_name, "")){
		sprintf(buf, "[%s]:", set_scr->input_num[item_num].unit_name);
	}else{
		sprintf(buf, ":");
	}
	// Display prompt
	waddstr(term_data.set_window.pmessage_window, buf);
	// Display displayed data to screen
	prefresh(term_data.set_window.pmain_window, 0, 0, 0, 0, LINES-1, COLS-1);
	//--------------------------------------
	// Copy current screen to standard screen
	// Because input string is not diaplayed on pad
	//--------------------------------------
	overwrite(term_data.set_window.pmain_window, term_data.std_window);
	getyx(term_data.set_window.pmessage_window, y, x);
	y = SCR_CAP_HIGHT + SCR_LINE_WIDTH * 2 + y;
	x = MSG_AREA_START_POS_X + x + 1;
	move(y, x);
	refresh();
	//--------------------------------------
	// Input answer
	//--------------------------------------
	curs_set(1);
	// Decimal
	if(set_scr->input_num[item_num].hex_or_dec == DEC_NUM){
		scanw("%d", input_num);
	// Hexadcimal
	}else{
		scanw("%x", input_num);
	}
	curs_set(0);
	//--------------------------------------
	// Set input value to data of setting screen
	//--------------------------------------
	set_scr->input_num[item_num].set_num = *input_num;
	//--------------------------------------
	// Clear message area
	//--------------------------------------
	wclear(term_data.set_window.pmessage_window);
	//--------------------------------------
	// Display setting information
	//--------------------------------------
	TermSetInfoWindow(set_scr);
	
	return TERM_ERR_SUCCESS;
}

