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
#include <signal.h>
#include <sys/types.h>
#include <linux/ipc.h>
#include <linux/sem.h>


//=================================================
// Declare global data
//=================================================
TERM_DATA	term_data;

//=================================================
// Declare function prototype
//=================================================
static long TermSetInfoWindow(PSET_SCR_DATA set_scr);

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
	// Set status area name
	//--------------------------------------
	strcpy(term_data.status_top_name, main_scr->status_top_name);
	strcpy(term_data.status_buttom_name, main_scr->status_buttom_name);
	//--------------------------------------
	// Set initial displayed data
	//--------------------------------------
	// Set window name
	strcpy(term_data.caption_str, main_scr->app_name);
	// Set menu display
	TermMenuSet(main_scr->menu[0]);

	return TERM_ERR_SUCCESS;
}

//================================================================
// Function for terminating routint
//================================================================
long TermExit()
{
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
	int  count;
	char buf[256];
	
	//--------------------------------------
	// Menu data copy
	//--------------------------------------
	for(count = 0; count < MENU_NUM; count++){
		memmove(term_data.menu[count].name, menu[count].name, sizeof(char)*256);
	}
	//--------------------------------------
	// The display of a partition
	//--------------------------------------
	printf("\n==============================\n");
	puts(term_data.caption_str);
	printf("==============================\n");
	//--------------------------------------
	// Set displayed menu
	//--------------------------------------
	for(count = 0; strcmp(menu[count].name, "") && count < MENU_NUM; count++){
		printf("%d.%s\n", count, menu[count].name);
	}
	//--------------------------------------
	// The display of a partition
	//--------------------------------------
	printf("==============================\n\n");

	return TERM_ERR_SUCCESS;
}

//================================================================
// Function for menu number acquisition
//================================================================
long TermGetMenuNum(char *message, int *menu_num)
{
	char get_char, buf[10];

	//--------------------------------------
	// Check parameter
	//--------------------------------------
	if(message == NULL || menu_num == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// Display message
	//--------------------------------------
	printf("\n%s\n: ", message);
	//--------------------------------------
	// Input number(0-9)
	//--------------------------------------
	do{
		get_char = (char)getchar();
		buf[0]	= get_char;
		buf[1] 	= '\0';
		*menu_num = atoi(buf);
	}while((*menu_num == 0) && (get_char != '0'));
	
	//--------------------------------------
	// Display menu
	//--------------------------------------
	TermMenuSet(&term_data.menu[0]);
	
	return TERM_ERR_SUCCESS;
}

//================================================================
// Function for retrieving string
//================================================================
long TermGetStr(char *message, char *get_str)
{
	int y, x;
	//--------------------------------------
	// Check parameter
	//--------------------------------------
	if(message == NULL || get_str == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// Display message
	//--------------------------------------
	printf("%s\n: ", message);
	//--------------------------------------
	// Input string
	//--------------------------------------
	scanf("%s", get_str);
	//--------------------------------------
	// Display message
	//--------------------------------------
	TermMenuSet(&term_data.menu[0]);

	return TERM_ERR_SUCCESS;
}

//================================================================
// Function for setting status string
//================================================================
long TermSetStatus(int sts_area, char *item_name, char *status_str)
{
	//--------------------------------------
	// Check parameter
	//--------------------------------------
	if(item_name == NULL || status_str == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// Set status string
	//--------------------------------------
	if(sts_area == STATUS_AREA_TOP){
		printf("%s %-10s: %s\n", term_data.status_top_name, item_name, status_str);
	}else{
		printf("%s %-10s: %s\n", term_data.status_buttom_name, item_name, status_str);
	}

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
	// Set displayed data
	//--------------------------------------
	strcpy(term_data.pre_caption_str, term_data.caption_str);
	strcpy(term_data.caption_str, set_scr->window_name);
	//--------------------------------------
	// Menu data copy
	//--------------------------------------
	for(count = 0; count < MENU_NUM; count++){
		memmove(term_data.pre_menu[count].name, term_data.menu[count].name, sizeof(char)*256);
	}
	TermMenuSet(set_scr->menu[0]);
	//--------------------------------------
	// Display information of setting screen
	//--------------------------------------
	TermSetInfoWindow(set_scr);

	return TERM_ERR_SUCCESS;
}

//================================================================
// Function for terminating setting screen
//================================================================
long TermSetWindowClose()
{
	//--------------------------------------
	// Set window name
	//--------------------------------------
	strcpy(term_data.caption_str, term_data.pre_caption_str);
	//--------------------------------------
	// Return main menu
	//--------------------------------------
	TermMenuSet(&term_data.pre_menu[0]);

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
	// Check parameter
	//--------------------------------------
	if(set_scr == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// The display of a partition
	//--------------------------------------
	printf("\n------------------------------\n");
	//--------------------------------------
	// Display setting information of selected item's type
	//--------------------------------------
	for(count = 0; strcmp(set_scr->select_item[count].set_item_name, "") && count < SET_ITEM_MAX_COUNT; count++){
		pitem_data	= &set_scr->select_item[count];
		printf("%-20s: %s\n", pitem_data->set_item_name, pitem_data->item[pitem_data->set_num].name);
	}
	//--------------------------------------
	// Display setting information of input numerical value's type
	//--------------------------------------
	for(count = 0; strcmp(set_scr->input_num[count].set_item_name, "") && count < SET_ITEM_MAX_COUNT;  count++){
		pnum_data	= &set_scr->input_num[count];
		//--------------------------------------
		// Display type on decimal
		//--------------------------------------
		if(pnum_data->hex_or_dec == DEC_NUM){
			if(strcmp(pnum_data->unit_name, "")){
				printf("%-20s: %d [%s]\n", pnum_data->set_item_name, pnum_data->set_num, pnum_data->unit_name);
			}else{
				printf("%-20s: %d\n", pnum_data->set_item_name, pnum_data->set_num);
			}
		//--------------------------------------
		// Display type on hexadecimal
		//--------------------------------------
		}else{
			if(strcmp(pnum_data->unit_name, "")){
				printf("%-20s: %X [%s]\n", pnum_data->set_item_name, pnum_data->set_num, pnum_data->unit_name);
			}else{
				printf("%-20s: %X\n", pnum_data->set_item_name, pnum_data->set_num);
			}
		}
	}
	//--------------------------------------
	// The display of a partition
	//--------------------------------------
	printf("------------------------------\n");

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
	// Check parameter
	//--------------------------------------
	if(set_scr == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// Display item name
	//--------------------------------------
	printf("< %s >\n", set_scr->select_item[item_num].set_item_name);
	//--------------------------------------
	// Display question
	//--------------------------------------
	printf("Please choose from the following choice.\n");
	//--------------------------------------
	// Display selected item
	//--------------------------------------
	strcpy(buf, "\n");
	for(count = 0; strcmp(set_scr->select_item[item_num].item[count].name, "") && count < SELECT_ITEM_MAX_COUNT; count++){
		sprintf(buf, "%s%d. %s  ", buf, count + 1, set_scr->select_item[item_num].item[count].name);
	}
	// Save maximum of items
	max_count = count;
	for(;;){
		//--------------------------------------
		// Input answer
		//--------------------------------------
		printf("%s\n> ", buf);
		scanf("%s", buf);
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
	// Display setting information
	//--------------------------------------
	TermSetInfoWindow(set_scr);
	//--------------------------------------
	// Display menu
	//--------------------------------------
	TermMenuSet(&term_data.menu[0]);

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
	// Check parameter
	//--------------------------------------
	if(set_scr == NULL){
		return TERM_ERR_PARAM;
	}
	//--------------------------------------
	// Display item name
	//--------------------------------------
	printf("< %s >\n", set_scr->input_num[item_num].set_item_name);
	//--------------------------------------
	// Display question
	//--------------------------------------
	printf("Please input a setting value.\n");
	// Display prompt
	if(strcmp(set_scr->input_num[item_num].unit_name, "")){
		printf("[%s]:", set_scr->input_num[item_num].unit_name);
	}else{
		printf(":");
	}
	//--------------------------------------
	// Input answer
	//--------------------------------------
	// Decimal
	if(set_scr->input_num[item_num].hex_or_dec == DEC_NUM){
		scanf("%d", input_num);
	// Hexadcimal
	}else{
		scanf("%x", input_num);
	}
	//--------------------------------------
	// Set input value to data of setting screen
	//--------------------------------------
	set_scr->input_num[item_num].set_num = *input_num;
	//--------------------------------------
	// Display setting information
	//--------------------------------------
	TermSetInfoWindow(set_scr);
	//--------------------------------------
	// Display menu
	//--------------------------------------
	TermMenuSet(&term_data.menu[0]);
	
	return TERM_ERR_SUCCESS;
}

