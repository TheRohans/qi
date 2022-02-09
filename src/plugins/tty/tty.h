#ifndef QI_PLUG_TTY_H
#define QI_PLUG_TTY_H

#define NB_COLORS 8

// https://www2.ccs.neu.edu/research/gpc/VonaUtils/vona/terminal/vtansi.htm

#define ESC_START                  '\033'

///////////////////////////////
// Device Status
///////////////////////////////
/* The following codes are used for reporting terminal/display settings, and vary depending on the implementation: */

/* Requests a Report Device Code response from the device. */
#define ESC_QUERY_DEVICE_CODE      "\033[c"

/* Generated by the device in response to Query Device Code request. */
#define ESC_REPORT_DEVICE_CODE     "\033[%d0c"

/* Requests a Report Device Status response from the device. */
#define ESC_QUERY_DEVICE_STATUS    "\033[5n"

/* Generated by the device in response to a Query Device Status request; indicates that device is functioning correctly. */
#define ESC_REPORT_DEVICE_OK       "\033[0n"

/* Generated by the device in response to a Query Device Status request; indicates that device is functioning improperly. */ 
#define ESC_REPORT_DEVICE_FAILURE  "\033[3n"

/* Requests a Report Cursor Position response from the device. */  
#define ESC_QUERY_CURSOR_POSITION  "\033[6n"

/* Generated by the device in response to a Query Cursor Position request; reports current cursor position. */   
#define ESC_REPORT_CURSOR_POSITION "\033[%d;%dR"

/////////////////////////////// 
// Cursor Control
/////////////////////////////// 
/* Sets the cursor position where subsequent
text will begin. If no row/column parameters are provided (ie. \033[H), 
the cursor will move to the home position, at the upper left of the screen. */
#define ESC_CURSOR_POS             "\033[%d;%dH"
#define ESC_CURSOR_HOME            "\033[H"

/* Moves the cursor up by COUNT rows; the default count is 1. */
#define ESC_CURSOR_UP              "\033[%dA"

/* Moves the cursor down by COUNT rows; the default count is 1. */
#define ESC_CURSOR_DOWN            "\033[%dB"

/* Moves the cursor forward by COUNT columns; the default count is 1. */
#define ESC_CURSOR_FORWARD         "\033[%dC"

/* Moves the cursor backward by COUNT columns; the default count is 1. */
#define ESC_CURSOR_BACKWARD        "\033[%dD"

/* Identical to Cursor Home. */
#define ESC_FORCE_CURSOR_POS       "\033[%d;%df"

/* Save current cursor position. */
#define ESC_SAVE_CURSOR            "\033[s"

/* Restores cursor position after a Save Cursor. */
#define ESC_UNSAVE_CURSOR          "\033[u"

/* Save current cursor position. */
// #define ESC_SAVE_CURSOR_ATTR    "\0337"

/* Restores cursor position after a Save Cursor. */
// #define ESC_RESTORE_CURSOR_ATTR "\0338"

///////////////////////////////
// Erasing Text
///////////////////////////////
/* Erases from the current cursor position to the end of the current line. */
#define ESC_ERASE_END_OF_LINE      "\033[K"

/* Erases from the current cursor position to the start of the current line. */
#define ESC_ERASE_START_OF_LINE    "\033[1K"

/* Erases the entire current line. */
#define ESC_ERASE_LINE		       "\033[2K"

/* Erases the screen from the current line down to the bottom of the screen. */
#define ESC_ERASE_DOWN		       "\033[J"

/* Erases the screen from the current line up to the top of the screen. */
#define ESC_ERASE_UP	           "\033[1J"

/* Erases the screen with the background colour and moves the cursor to home. */
#define ESC_ERASE_SCREEN		   "\033[2J"

///////////////////////////////
// Define Key
///////////////////////////////
/* Associates a string of text to a keyboard key. {key} indicates the key by its ASCII value in decimal. */
#define ESC_SET_KEY_DEFINITION	   "\033[%d;\"%s\"p"

///////////////////////////////
// Set Display Attributes
///////////////////////////////
/* Sets multiple display attribute settings. The following lists standard attributes: */
#define ESC_SET_ATTRIBUTE_MODE_3   "\033[%d;%d;%dm"
#define ESC_SET_ATTRIBUTE_MODE_2   "\033[%d;%dm"
#define ESC_SET_ATTRIBUTE_MODE_1   "\033[%dm"
/*
0	Reset all attributes
1	Bright
2	Dim
4	Underscore	
5	Blink
7	Reverse
8	Hidden

----Foreground Colours
30	Black
31	Red
32	Green
33	Yellow
34	Blue
35	Magenta
36	Cyan
37	White

----Background Colours
40	Black
41	Red
42	Green
43	Yellow
44	Blue
45	Magenta
46	Cyan
47	White
*/

int tty_init(void);
void tty_resize(int sig);

#endif

