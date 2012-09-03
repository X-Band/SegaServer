/*
	File:		ServerCore.c

	Contains:	Server Core Entry Points

	Progeny of:	Dave Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<97>	 9/16/94	ATM		Didn't quite succeed with previous fix.
		<96>	 9/16/94	ATM		boxFlags->boxModified, playerFlags->playerModified,
									acceptChallenges->playerFlags.
		<95>	  9/3/94	ATM		Added gHangupOkay flag.  Stripped out old, unused junk.  See
									previous rev for "old tubs of shit docs" and the first 8
									dispatcher table initializers.
		<94>	 8/30/94	ATM		Converted accountChangedMask references to Account flags.
		<93>	 8/28/94	ATM		Changes for 'segb'.
		<92>	 8/27/94	ATM		seg9 changes, plus check on validFlags in UpdateDataBaseAccount.
		<91>	 8/26/94	DJ		clearing out dispatcher tables with NULL for safety
		<90>	 8/26/94	ATM		Added handlers for anticipated SEG9, but left them #ifdefed out
									for now.  (SEG8 + new CreditDebitInfo)
		<89>	 8/26/94	BET		msSendClearSendQueue message needs to call
									Server_SendClearMiscQueues.
		<88>	 8/25/94	BET		Add support for multiple NetErrorRecord types for both X25 and
									800 services.
		<87>	 8/25/94	DJ		added Server_SendClearMiscQueues
		<86>	 8/25/94	ATM		Now at seg7 for f2 ROMs.
		<85>	 8/21/94	DJ		update the database Account if EndCommunication is called
		<84>	 8/21/94	DJ		new ROM version.. 'seg6'
		<83>	 8/20/94	DJ		new ROM version.. 'seg5'
		<82>	 8/19/94	BET		Turn off listen timeout, sigalarm should catch the server if it
									blows.
		<81>	 8/18/94	DJ		fixed redial treat
		<80>	 8/18/94	DJ		more printfs
		<79>	 8/17/94	DJ		receiveloginversion2
		<78>	 8/16/94	DJ		made tlisten timeout 1 min instead of 5
		<77>	 8/16/94	DJ		senddateandtime
		<76>	 8/13/94	DJ		forceend
		<75>	 8/13/94	BET		Added SendNetErrors
		<74>	 8/13/94	ATM		Fixed gLogFile stuff for Mac.
		<73>	 8/13/94	ATM		Pulled logging and debugging stuff out.
		<72>	 8/12/94	DJ		calling validatesystem
		<71>	 8/12/94	DJ		supports multiple ROM versions
		<70>	 8/12/94	ATM		Reversed strings for NBA Jam.
		<69>	 8/12/94	ATM		Added Server_GameName.
		<68>	 8/12/94	DJ		turned it off
		<67>	 8/12/94	DJ		turned on personification
		<66>	 8/12/94	ATM		Updated logging stuff.
		<65>	 8/12/94	ATM		Added Statusmsg().
		<64>	 8/11/94	DJ		news sends immediately
		<63>	 8/11/94	ATM		Added Logmsg().
		<62>	 8/10/94	DJ		personification
		<61>	  8/8/94	DJ		loopback is now enabled by mail
		<60>	  8/6/94	DJ		fuck me
		<59>	  8/6/94	DJ		another
		<58>	  8/6/94	DJ		bugler
		<57>	  8/6/94	DJ		bug
		<56>	  8/6/94	DJ		playeraccount stuff
		<55>	  8/5/94	DJ		playeraccount stuff
		<54>	  8/5/94	DJ		game error results
		<54>	  8/5/94	DJ		game error results
		<53>	  8/4/94	ATM		Commented out Server_SetupGamePatches (now done on database
									side).
		<52>	  8/4/94	BET		Comment out gratuitous NetIdle call.
		<51>	  8/3/94	ATM		Added memory watch debug stuff.
		<50>	  8/4/94	ATM		Commented out Server_SetupAccounts call.
		<49>	  8/3/94	DJ		just some jizz
		<48>	  8/2/94	DJ		googar
		<47>	 7/31/94	DJ		sending new problem token and validation         token before
									endCommunication
		<46>	 7/29/94	DJ		sending new problem token and validation token before
									endCommunication
		<45>	 7/26/94	DJ		fixed serverprogress bug
		<44>	 7/26/94	BET		Add ifndef unix to STANDALONE definition
		<43>	 7/26/94	DJ		fixing crasher to ByteCopy in ServerState_Init
		<42>	 7/25/94	DJ		5 days of hacking, including: async listens so can age the waitq
		<41>	 7/20/94	DJ		no ServerState passed to dbase routines
		<40>	 7/20/94	DJ		removed configstr from serverstate
		<39>	 7/20/94	DJ		DoCommand returns true after a full connect sequence
		<38>	 7/20/94	DJ		added Server_Comm stuff
		<37>	 7/19/94	BET		#ifndef unix to the gLogFile setting
		<36>	 7/19/94	DJ		server test
		<35>	 7/18/94	DJ		driving boxType msg
		<34>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		<33>	 7/16/94	dwh		more unix-ise.
		<32>	 7/15/94	DJ		added new mail
		<31>	 7/14/94	DJ		new mail
		<30>	 7/14/94	DJ		moved news later to avoid resendack bug
		<29>	 7/13/94	dwh		unix crud.
		<28>	 7/12/94	DJ		sends news right after Login
		<27>	  7/8/94	DJ		made it work over and over if TListen failes
		<26>	  7/6/94	DJ		address book validation
		<25>	  7/5/94	DJ		added pclose if tlisten fails
		<24>	  7/4/94	DJ		using that param
		<23>	  7/3/94	DJ		added param to CloseServerConnection to avoid TClose in error
									situations cuz it can hand
		<22>	  7/1/94	DJ		making server handle errors from the comm layer
		<21>	 6/30/94	DJ		validate the system, send update patches
		<20>	 6/28/94	BET		Update for new transport interfaces for OSErr         on Open
									and Listen.
		<19>	 6/22/94	BET		Update for new phys interfaces for OSErr on Open and Listen.
									Still need to convert transport interfaces once they are ready.
		<18>	 6/15/94	DJ		added PClose
		<17>	 6/12/94	DJ		checks the game patch and also starts game play (does waitQ and
									ranking match).
		<16>	 6/11/94	DJ		removed any NetIdle stuff after PListen
		<15>	 6/11/94	DJ		removed any NetIdle stuff after PListen
		<14>	 6/10/94	BET		Don't call NetIdle after PListen, call PNetIdle.  NetIdle has
									the opportunity to take packets off the incoming fifo before the
									listen is complete, and they get tossed.
		<13>	 6/10/94	DJ		relistens after each connect, also fragmented DoCommand and
									don't call Server_Drive anymore.
		<12>	  6/9/94	BET		Fix interface change
		<11>	  6/5/94	BET		Make it multisession
		<10>	  6/5/94	BET		Change interface to InitServer
		 <9>	  6/5/94	BET		Change interfaces to TListen
		 <8>	  6/4/94	DJ		mallocing SessionRec instead of globa
		 <7>	 5/31/94	DJ		cheezewad gloggler
		 <6>	 5/29/94	DJ		gibbly treats
		 <5>	 5/29/94	DJ		test stuff for reading preformatted msgs
		 <4>	 5/27/94	DJ		added comments about identifying 'sega' and 'nint'
		 <3>	 5/26/94	DJ		removed dome printfs
		 <2>	 5/25/94	DJ		fixed driving etc
		 <4>	 5/25/94	DJ		new routines and comments etc
		 <3>	 5/23/94	BET		A starting point?
		 <2>	 5/23/94	BET		Possible poo for you

	To Do:
	
	5/24/94
	
	Validation.
	
	Subdivide SendQ so that it discriminates between mail, game results, new address books,
	and other personification shit (?).
	Clear box's sendQ
	
	Kool stuff.  Where does it go?  Sprinkle it about?
	
	Test all the driving of database - get,set items and types
		(adds sounds, taunts, images, whatever).
		
	Test drawing
	
	patching the O/S
	patching and removing message patches
	
	sending a new NGP (done at every connection probably).
	
	setting box serial number (?)
	What about setting which local service number box is to call?
	
	Competitive opCode needs work
	
	
*/


#include "ServerCore.h"
#include "Server.h"
#include "Messages.h"
#include "ServerDataBase.h"
#include "Dates.h"
#include "Server_Comm.h"
#include "PhysicalLayer.h"
#include "NetMisc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifndef unix
#define STANDALONE
#endif

// #define EXTERNAL_MODEM


#define kServerWriteTeeFile  "ServerWriteData.tee"
#define kServerReadTeeFile  "ServerReadData.tee"
#define kServerInputFile  "ServerInputData.tee"

#define kTListenTimeout	0			/* no timeout for TListen, the alarm process should catch the server */


// Positive numbers are which message the server is processing.
// Negative numbers indication connection status during the process of connecting.
//
#define kServerConnected	0
#define	kServerNotListening	-1
#define kServerPListening	-2
#define kServerTListening	-3


static MessageDispatcher *currentDispatcher;
static MessageDispatcher dispatchers[kNumMessageDispatchers];



#if defined(STANDALONE) && !defined(unix)

#define kConfigStrLength	1000

char *FindConfigString(char *config);


main()
{
ServerState *boxState;

#ifdef EXTERNAL_MODEM
char	configStr[kConfigStrLength];

	boxState = InitServer(FindConfigString(configStr));
#else
	boxState = InitServer(NULL);
#endif

	if(boxState)
	{
		for(;;)
		{
			DoCommand(boxState);
		}
	}

	free(boxState->session);
	boxState->session = NULL;
}

#endif


//
// Start the server up from the menu.
//
ServerState *InitServer(char *config)
{
ServerState	*boxState;

	Server_LoadMessageDispatchers();
	if (Server_SwapMessageDispatcher(kSegaIdentification) != kServerFuncOK) {
		Logmsg("InitServer: Server boys are hosers\n");
		return (NULL);
	}


	boxState = NULL;
	
	Logmsg("Init server\n");

	boxState = (ServerState *)malloc(sizeof(ServerState));
	if(!boxState) 
	{
		Logmsg("no memory for ServerState");
	}
	else 
	{
		ServerDataBase_Initialize();
		//Server_SetupAccounts();
		//Server_SetupGamePatches();

		ServerState_Init(boxState, NULL, config);

		boxState->serverProgress = kServerNotListening;	// not connected

		Server_TInit();
	}
	
	return(boxState);
}

//
// Open a connection.
//

static SessionRec	*aGame = NULL;
static long 		numConnects = 0, numFailedConnects = 0;


int InitServerConnection(ServerState *boxState)
{

OSErr		err;
char		*config;
NetParamBlock	net;


	DataBase_AgeWaitQ(kMaxTimeInWaitQ);

	if(boxState->configBuffer[0])
		config = boxState->configBuffer;
	else
		config = NULL;

	if(boxState->serverProgress == kServerNotListening)
	{
		Logmsg("listening\n");

		aGame = (SessionRec *)malloc(sizeof(SessionRec));
		if(!aGame) 
		{
			Logmsg("no memory for SessionRec");
			ServerState_Init(boxState, NULL, NULL);
			return(-1);	// some kind of error notification, please
		}

#ifdef STANDALONE
		err = Server_PListenAsync(config, kUseServerProtocol);
#else
		err = Server_PListen(config, kUseServerProtocol);
#endif
		if(err != noErr)
		{
			Logmsg("PListenAsync failed with error #%ld\n", (long)err);
			Server_PCheckError();
			free(aGame);
			aGame = NULL;
			numFailedConnects++;
			return(-1);
		}
		else
		{
			boxState->serverProgress = kServerPListening;
			Logmsg("PListen succeeded\n");
		}
	}
	
	
	if (boxState->serverProgress == kServerPListening)
	{
		
		if((err = Server_TNetIdle(&net)) != noErr)
		{
			Logmsg("Server_TNetIdle returned error #%ld\n", err);
			Server_TCheckError();
			Server_PCheckError();
			boxState->serverProgress = kServerNotListening;
			free(aGame);
			aGame = NULL;
			numFailedConnects++;
			return(-1);
		}
		
		if(net.ioPhysNetState == kConnOpen)
			boxState->serverProgress = kServerTListening;
	}
	
	if(boxState->serverProgress == kServerTListening)
	{
		err = Server_TListen(aGame, -1, 0, kTListenTimeout);
		if(err != noErr)
		{
			Logmsg("TListen failed with error #%ld\n", (long)err);
			Server_TCheckError();
			if(Server_PClose() != noErr)
				Server_PCheckError();

			Logmsg("connection failed\n");
			ServerState_Init(boxState, NULL, config);
			free(aGame);
			aGame = NULL;
			boxState->serverProgress = kServerNotListening;
			numFailedConnects++;
			return(-1);
		}
		else
		{
			Logmsg("TListen succeeded\n");

			ServerState_Init(boxState, aGame, config);
			boxState->serverProgress = kServerConnected;
			Logmsg("connected\n");
			
			numConnects++;
			
			if(Server_OpenDataTee(kServerWriteTeeFile, kServerReadTeeFile, kServerInputFile) != 0)	// if we are generating a tee file of the data for debugging
			{
				Logmsg("*** Unable to open tee file ***\n");
			}
		}
	}
	
	return(0);
}

//
// Close a connection.
//
Boolean CloseServerConnection(ServerState *boxState)
{
Boolean	retVal;
OSErr		err;
#ifdef unix
extern long gHangupOkay;
#endif

	retVal = false;
	if(boxState->validFlags & kServerValidFlag_SessionRec)
	{
		if( Server_TClose(boxState->session) != noErr)	// shouldn't hang anymore
			Server_TCheckError();
			
#ifdef unix
		gHangupOkay = 1;		// okay if box hangs up and we get SIGHUP
#endif

		err = Server_PClose();
		if(err != noErr)
		{
			Logmsg("PClose failed with error #%ld\n", err);
			Server_PCheckError();
		}

		Server_CloseDataTee();	// if we are generating a tee file of the data for debugging
	}

	ServerState_Empty(boxState);	// luckily doesn't clear configString
	boxState->serverProgress = kServerNotListening;	// done
	Logmsg("Shutting down the connection\n");

	return(0);
}


//
// Final shutdown of the server from the menu.
//
Boolean KillServer(ServerState *boxState)
{
Boolean	retVal;

	retVal = CloseServerConnection(boxState);
	
	free(boxStat