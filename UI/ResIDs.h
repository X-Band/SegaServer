/*
	File:		ResIDs.h

	Contains:	xxx put contents here xxx

	Written by:	Brian Topping

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		 <7>	 6/30/94	BET		Add some constants for test framework
		 <6>	 6/14/94	BET		Add Poo
		 <4>	  6/3/94	BET		"Play with windows and stuff"
		 <3>	  6/1/94	BET		Added the basher command
		 <2>	 5/31/94	BET		Added the Brian Cmd
		 <2>	 5/22/94	BET		Fix a screwup in goodies
		 <1>	 5/22/94	BET		first checked in

	To Do:
*/

// ===========================================================================
//	ResIDs.h					©1994 Brian Topping, All rights reserved.
// ===========================================================================

#pragma once
enum {
	kNewServerCmd = 1000,
	kNewTestingCmd,
	kNewScriptCmd,
	kServerDefWait = 10000,
	kServerDefOpen,
	kServerWaitForConnectCmd,
	kServerOpenConnectCmd,
	kServerCloseConnectCmd,
	kTestDefWait = 10100,
	kTestDefOpen,
	kTestWait,
	kTestOpen,
	kTestCloseConnectCmd,
	kSendFileMessageCmd,
	kUseGameTalkCmd,
	kExecTimmerScript,
	kStartBashCmd = 20000,
	kRecieveBashCmd,
	kTestSendText,
	kAppConfigPorts = 30000
};


typedef Int16 AlertIDT;
enum {
	Alrt_About=128,
	Alrt_Fatal
};


