/*
	File:		printf.h

	Contains:	routines to implement log window

	Written by:	Brian Topping

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <2>	 6/30/94	BET		Add TestWindow support for Timmer test framework.
		 <1>	 5/31/94	BET		first checked in
	To Do:
*/

#if defined(SCRIPT) && !defined(printf)
#define printf bTestPrintf
#endif

extern "C" {
int asm bprintf(const char *, ...);
void SetText(short length, char *buf);
int asm bTestPrintf(const char *, ...);
void SetTestText(short length, char *buf);
}