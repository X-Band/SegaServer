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
#		 <5>	 9/18/94	ATM		Added common to -I list.  Removed Server_Drive.o.
#		 <4>	 8/28/94	ATM		Added GameTalk to -I list.
#		 <3>	  8/4/94	ATM		Added ../../../SegaOS/UserInterface.
#		 <2>	 7/19/94	BET		Add Server_ReceiveDebitCard.o
#		 <1>	 7/15/94	dwh		first checked in
#
#	To Do:
#

include ../../../UNIX/Makefile.config

INCOPTS= \
	-I../../Server \
	-I../../ServerDataBase \
	-I../../Misc \
	-I../../../newrpc/common \
	-I/usr/include \
	-I../../../Comm \
	-I../../../SegaOS/Database \
	-I../../../SegaOS/Graphics \
	-I../../../SegaOS/Interfaces \
	-I../../../SegaOS/UserInterface \
	-I../../../SegaOS/OSCore \
	-I../../../SegaOS/Shell \
	-I../../../GameTalk

OBJS =	Server_ReceiveCompChallenge.o \
	Server_ReceiveDebitCard.o \
	Server_ReceiveGameID.o \
	Server_ReceiveLogin.o \
	Server_ReceiveNGP.o \
	Server_ReceiveSendQ.o \
	Server_ReceiveSystemVersion.o

all: $(OBJS)

clean::
	$(RM) $(OBJS)

