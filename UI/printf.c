/*
	File:		printf.c

	Contains:	routines to implement log window

	Written by:	Brian Topping

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <2>	 6/30/94	BET		Add TestWindow support for Timmer test framework.
		 <1>	 5/31/94	BET		first checked in
	To Do:
*/

#include "CServerWindow.h"
#include "CTestWindow.h"
#include "printf.h"
#include <stdio.h>
#include <string.h>

char	PrintBuf[1024];
void	*retAddr;

int asm bprintf(const char *, ...)
{
	move.l	(sp)+, retAddr
	pea		PrintBuf
	jsr		sprintf
	move.w	d0,-(sp)
	jsr		SetText
	move.w	(sp)+, d0
	addq.l	#4,sp
	move.l	retAddr, a0
	jmp		(a0)
}


void SetText(short length, char *buf)
{
char	*addr = buf;
	
	addr = strchr(buf, '\n');
	while (addr) {
		*addr = '\r';
		addr = strchr(buf, '\n');
		}
	CServerWindow::GetTopWindow()->AddText(buf, length);
}


int asm bTestPrintf(const char *, ...)
{
	move.l	(sp)+, retAddr
	pea		PrintBuf
	jsr		sprintf
	move.w	d0,-(sp)
	jsr		SetTestText
	move.w	(sp)+, d0
	addq.l	#4,sp
	move.l	retAddr, a0
	jmp		(a0)
}


void SetTestText(short length, char *buf)
{
char	*addr = buf;
	
	addr = strchr(buf, '\n');
	while (addr) {
		*addr = '\r';
		addr = strchr(buf, '\n');
		}
	CTestWindow::GetTopWindow()->AddText(buf, length);
}
