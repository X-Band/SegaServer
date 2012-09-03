#
#	File:		Makefile
#
#	Contains:	xxx put contents here xxx
#
#	Written by:	David Hankinson
#
#	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.
#
#	Change History (most recent first):
#
#		 <8>	 9/18/94	ATM		Add common to -I list.  Removed Server_Log.o.
#		 <7>	 8/28/94	ATM		Added GameTalk to -I list.
#		 <6>	 8/21/94	DJ		added Server_RestoreBox.c
#		 <5>	 8/13/94	ATM		Added Server_Log.c
#		 <4>	  8/4/94	BET		Add Server_Personification.o
#		 <3>	 7/27/94	BET		../../SegaOS/UserInterface for SegaIn.h
#		 <2>	 7/20/94	BET		Add Server_Comm.o.
#		 <1>	 7/15/94	dwh		first checked in
#
#	To Do:
#

include ../../UNIX/Makefile.config

INCOPTS= \
	-I/usr/include \
	-I../../Comm \
	-I../../SegaOS/Database \
	-I../../SegaOS/Graphics \
	-I../../SegaOS/Interfaces \
	-I../../SegaOS/UserInterface \
	-I../../SegaOS/OSCore \
	-I../../SegaOS/Shell \
	-I../../SegaOS/Misc \
	-I../../newrpc/common \
	-I../Server \
	-I../ServerDataBase \
	-I../Misc \
	-I../../GameTalk

OBJS =	ServerCore.o \
	ServerState.o \
	Server_Comm.o \
	Server_GetSegaDate.o \
	Server_DebugService.o \
	Server_Personification.o \
	Server_ProcessSendQ.o \
	Server_RestoreBox.o \
	Server_StartGamePlay.o \
	Server_UpdateGamePatch.o \
	Server_UpdateNGPVersion.o \
	Server_ValidateLogin.o \
	Server_ValidateSystem.o

all: $(OBJS)

#Server_ValidateLogin.cpp: Server_ValidateLogin.c
#	$(CC) -E $(CFLAGS) Server_ValidateLogin.c > $@

clean::
	$(RM) $(OBJS)

