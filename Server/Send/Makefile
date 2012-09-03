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
#		 <4>	 8/28/94	ATM		Added -IGameTalk.
#		 <3>	  8/2/94	ATM		Removed Server_SendBitmap and Server_SendText.
#		 <2>	 7/19/94	BET		Add files
#		 <1>	 7/15/94	dwh		first checked in
#
#	To Do:
#

include ../../../UNIX/Makefile.config

INCOPTS=-I../../../Comm \
	-I../../../SegaOS/Database \
	-I../../../SegaOS/Graphics \
	-I../../../SegaOS/Interfaces \
	-I../../../SegaOS/OSCore \
	-I../../../SegaOS/Shell \
	-I../../../SegaOS/UserInterface \
	-I../../../GameTalk \
	-I../../Misc \
	-I../../Server \
	-I../../ServerDataBase

OBJS =	Server_Drive.o \
	Server_EndCommunication.o \
	Server_SendAccountInfo.o \
	Server_SendAddressBook.o \
	Server_SendBoxSerialNumber.o \
	Server_SendDialog.o \
	Server_SendKoolStuff.o \
	Server_SendLoopback.o \
	Server_SendMail.o \
	Server_SendNGP.o \
	Server_SendNews.o \
	Server_SendOpponent.o \
	Server_SendPreformedMesg.o \
	Server_SendRanking.o \
	Server_SendUnsupportedGame.o

all: $(OBJS)

clean::
	$(RM) $(OBJS)
