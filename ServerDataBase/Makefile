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
#		 <7>	 9/18/94	ATM		Added common to -I list.  Removed DataBase_List.o.
#		 <6>	  9/6/94	ATM		Added objects for Matching stuff.
#		 <5>	 8/11/94	ATM		Added DataBase_Lata.c.
#		 <4>	  8/4/94	ATM		Added ../../SegaOS/UserInterface.
#		 <3>	 7/29/94	BET		Ensure that -UCLIENTRPC is turned on for this directory only.
#		 <2>	 7/16/94	BET		Remove an unneeded file that was breaking the build.
#		 <1>	 7/15/94	dwh		first checked in
#
#	To Do:
#

include ../../UNIX/Makefile.config

CFLAGS += -UCLIENTRPC

INCOPTS= \
	-I../ServerDataBase \
	-I../Server \
	-I../Misc \
	-I../../newrpc/common \
	-I../../Comm \
	-I/usr/include \
	-I../../SegaOS/Database \
	-I../../SegaOS/Graphics \
	-I../../SegaOS/Interfaces \
	-I../../SegaOS/UserInterface \
	-I../../SegaOS/OSCore \
	-I../../SegaOS/Shell \
	-I../../SegaOS/Misc

OBJS =	DataBase_Core.o \
	DataBase_GamePatch.o \
	DataBase_GameResults.o \
	DataBase_Games.o \
	DataBase_KoolStuff.o \
	DataBase_Lata.o \
	DataBase_Mail.o \
	DataBase_NGP.o \
	DataBase_NGPVersion.o \
	DataBase_News.o \
	DataBase_Opponent.o \
	DataBase_System.o \
	DataBase_SystemPatches.o \
	DataBase_UserAccount.o \
	DataBase_Users.o \
	Matching_Matching.o \
	Matching_Util.o

all: $(OBJS)

clean::
	$(RM) $(OBJS)
