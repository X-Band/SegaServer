/*
	File:		Server_ProcessSendQ.c

	Contains:	Process SendQ items.

	Written by:	David Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<86>	 9/16/94	ATM		Clear the "dial 9" flag if they change the phone and don't set
									it again.
		<85>	 9/16/94	ATM		Change "dial 9" thing with "phone" to use debug1.
		<84>	 9/16/94	ATM		boxFlags->boxModified, playerFlags->playerModified,
									acceptChallenges->playerFlags.
		<83>	 9/15/94	DJ		setting scriptID by title to mail to "server"  (eg. title = #5
									sets scriptID to 5).
		<82>	 9/15/94	DJ		sending mail to server does normal POP lookup first in order to
									get the scriptID
		<81>	 9/12/94	DJ		changed LataLookup to POPLookup (looks up Compuserve POPs)
		<80>	 9/12/94	ATM		Hacked the "phone" mail target to add an arbitrary prefix, for
									dialing out of companies.
		<79>	 9/11/94	ATM		Got rid of NO_OS_STRINGS; now include the OS string stuff all
									the time.
		<78>	  9/7/94	ATM		Enabled STRIP_AREA_CODE, removed reference to
									extractedPhoneNumber.
		<77>	  9/2/94	DJ		storing addr book info in the playerAccount addr book now
		<76>	  9/1/94	DJ		turning off alarm if bigloop
		<75>	  9/1/94	DJ		sending mail to "bigloop" does loop 10 times (ie. 20 minutes of
									thruput testing)
		<74>	  9/1/94	ATM		Updated logging stuff.
		<73>	 8/31/94	ATM		Spiffed up date printing on mail items.
		<72>	 8/30/94	DJ		uncommented NO_OS_STRINGS define
		<71>	 8/30/94	DJ		sending mail to crashlog prints your mail in the crashlog.
									also, when you crash, i now print out which patch version was on
									your box
		<70>	 8/30/94	ATM		Changed call to UpdateUserPhoneNumber.
		<69>	 8/30/94	ATM		Converted accountChangedMask references to Account flags.
		<67>	 8/28/94	ATM		Checked it in with NO_OS_STRINGS commented out.  Whoops.
		<66>	 8/28/94	ATM		Updated mail handling for segb.
		<65>	 8/27/94	ATM		Added NO_OS_STRINGS stuff.
		<64>	 8/27/94	ATM		ChatDebug separation for 800 and X.25.
		<63>	 8/26/94	ATM		Tweaked some logmsgs.
		<62>	 8/26/94	ATM		Added Andy1's crash record printing.
		<61>	 8/25/94	DJ		added Server_SendClearMiscQueues
		<60>	 8/25/94	ATM		Added some more stuff to DumpJamsStats for Steve.
		<59>	 8/23/94	ATM		Fixed DumpJamsStats.
		<58>	 8/23/94	ATM		Added DumpJamsStats (untested!)
		<57>	 8/23/94	ATM		Added game results dump to sendq processing.
		<56>	 8/22/94	DJ		more grooming
		<55>	 8/22/94	DJ		groomed dialog text
		<54>	 8/22/94	DJ		printing chat script errors from sendq
		<53>	 8/21/94	ATM		altPopPhone stuff.
		<52>	 8/21/94	DJ		sendlocalaccessphonenum takes 2 phone numbers (2nd is a fallback
									phone number)
		<51>	 8/20/94	DJ		serverUniqueID is now in PlayerInfo, not userIdentification
		<50>	 8/20/94	BET		Changing the polarity of that last one, the server crashes
									trying to dump the data without the hexdump.
		<49>	 8/20/94	BET		Add Loghexdump for mail body under OPTIONAL compile time flag.
		<48>	 8/20/94	DJ		mail to "patch" will try to read in and download the mesg file
									you specify in the mail body.
		<47>	 8/19/94	DJ		fixed printf of mail serial number
		<46>	 8/18/94	ATM		Changed outbound mail message.
		<45>	 8/18/94	DJ		sending mail to "core" makes sunsega dump.  neato feature.
		<44>	 8/17/94	ATM		Added Crashmsg stuff.
		<43>	 8/17/94	ATM		Redo LATA lookup for "phone" e-mail.
		<42>	 8/17/94	ATM		Tweaked.
		<41>	 8/15/94	ATM		Adjusted hex dump to format correctly with updated Logmsg().
		<40>	 8/15/94	DJ		sticky dialogs for mail only connect
		<39>	 8/13/94	ATM		Don't popen /usr/ucb/mail on Mac!
		<38>	 8/13/94	ATM		Changed strncasecmp("xband") to DataBase_CompareStrings.
		<37>	 8/13/94	ATM		Added dump of data for kRestartInfoType.
		<36>	 8/12/94	ATM		Mail to "xband" goes to xband@catapent.com.
		<35>	 8/12/94	DJ		bollgles the bwain, dunnit?
		<34>	 8/12/94	DJ		bugfix
		<33>	 8/11/94	DJ		bugfix
		<32>	 8/11/94	ATM		Converted to Logmsg.
		<31>	 8/10/94	ATM		Added MakePhoneNumberPretty calls to the special "phone" and
									"server" mail targets.
		<30>	  8/8/94	DJ		can send mail to "loop" and turn on throughput testing
		<29>	  8/6/94	DJ		fix
		<28>	  8/5/94	DJ		you can mail to "redial" to get box to redial server in a loop
		<27>	  8/5/94	DJ		fixes and optimization to ProcessAddreBook
		<26>	  8/5/94	DJ		all new address book stuff!  like
									Server_SendCorrelateAddressBookEntry, etc
		<25>	  8/4/94	DJ		even less SNDQElement
		<24>	  8/4/94	DJ		no more SNDQElement
		<23>	  8/2/94	DJ		fixed mail to "phone"
		<22>	  8/2/94	DJ		update to latest mail sending scheme
		<21>	 7/27/94	DJ		sending dialagain flag when local access number is changed
		<20>	 7/25/94	DJ		nothing
		<19>	 7/20/94	DJ		added Server_Comm stuff
		<18>	 7/19/94	DJ		hmmmmm
		<17>	 7/18/94	DJ		update to latest addrbookvalidation stuff
		<16>	 7/17/94	BET		Make printf's go to gLogFile instead of stdout, which is the
									modem.
		<15>	 7/15/94	SAH		Changed some printf around new GameResult structure.
		<14>	 7/15/94	DJ		added mail processing
		<13>	 7/14/94	DJ		added processmail
		<12>	 7/12/94	DJ		using Server_TCheckError instead of TCheckError
		<11>	 7/11/94	DJ		set extractedPhoneNumber so game is registered with this new ph
									num
		<10>	  7/8/94	DJ		msgs to set phonenums on box
		 <9>	  7/6/94	DJ		address book validation
		 <8>	  7/3/94	DJ		no iconref in mail
		 <7>	  7/3/94	DJ		error tolerant
		 <6>	  7/2/94	DJ		rankings
		 <5>	 6/30/94	DJ		updated to new sendQ format
		 <4>	 6/30/94	DJ		no result FIFOs anymore.
		 <3>	 5/31/94	DJ		better mail handling through database
		 <2>	 5/29/94	DJ		added reading of mail
		 <1>	 5/27/94	DJ		first checked in

	To Do:
*/

#include "ServerCore.h"
#include "Server.h"
#include "ServerState.h"
#include "ServerDataBase.h"
#include "DBTypes.h"
#include "Mail.h"
#include "SendQ.h"
#include "BoxSer.h"

#include "Messages.h"
#include "PlayerDB.h"
#include "Server_Comm.h"
#include "AddressBook.h"

// #include "ServerMessagesUtils.h"

#include <stdio.h>
#include <string.h>

void DumpJamsStats(unsigned char *data, long size);
void PrintCrashRecord ( unsigned char * cr, long blockSize );


#ifdef unix
//
// This is for when you send mail to bigloop, wanna turn of the 4 min alarm.
//
#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <signal.h>

#endif unix




/*
typedef struct QItem {
	DBType			type;
	long			size;
	unsigned char	*data;
} QItem;

typedef struct SendQData {
	// mail
	// address book entries for validation
	// game results
	// other personification poop

	short			count;
	QItem			*items;
} SendQData;


typedef struct
{
	long	gameID;
	short	userID;
	long	userScore;
	short	opponentID;
	long	opponentScore;
} Result;

*/

/*
	Grind through the queue and deal with each type of treat:
		game results
		mail
	
	Game results:
		add them to the user's game results database (by boxSerialNumber and user number)
		update ranking
*/


int Server_SendClearSendQ(ServerState *state)
{
messOut				opCode;

	opCode = msClearSendQ;
	Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);

	return(kServerFuncOK);
}

int Server_SendClearMiscQueues(ServerState *state)  // used instead of ClearSendQ in ROM version 7 (seg7)
{
messOut				opCode;
long				controlFlags;

	opCode = msServerMiscControl;
	Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);

	controlFlags = kClearNetErrorsFlag;
	controlFlags |= kDeleteSendQFlag;
	controlFlags |= kDeleteAllAoutBoxMailFlag;
	controlFlags |= kMarkAddressBookUnchangedFlag;
	controlFlags |= kClearGameResultsFlag;
	Server_TWriteDataSync(state->session, sizeof(long), (Ptr)&controlFlags);

	return(kServerFuncOK);
}

int Server_SendSetBoxPhoneNumber(ServerState *state, phoneNumber *newBoxPhoneNumber)
{
messOut			opCode;
phoneNumber		strippedPhoneNumber;
int				len;

#define STRIP_AREA_CODE
#ifdef STRIP_AREA_CODE
	len = strlen(newBoxPhoneNumber->phoneNumber);
	if (len < 8 || newBoxPhoneNumber->phoneNumber[len-5] != '-') {
		Logmsg("Set box phone number: I can't strip %s!\n",
			newBoxPhoneNumber->phoneNumber);

		// send original
		opCode = msSetBoxPhoneNumber;
		Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
		Server_TWriteDataSync(state->session, sizeof(phoneNumber), (Ptr)newBoxPhoneNumber);
	} else {
		strippedPhoneNumber = *newBoxPhoneNumber;	// get the extra fields
		strcpy(strippedPhoneNumber.phoneNumber,		// overwrite with last 8
			   newBoxPhoneNumber->phoneNumber +
				   strlen(newBoxPhoneNumber->phoneNumber) - 8);

		opCode = msSetBoxPhoneNumber;
		Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
		Server_TWriteDataSync(state->session, sizeof(phoneNumber), (Ptr)&strippedPhoneNumber);
	}

#else
	opCode = msSetBoxPhoneNumber;
	Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	Server_TWriteDataSync(state->session, sizeof(phoneNumber), (Ptr)newBoxPhoneNumber);
#endif
	return(kServerFuncOK);
}

int Server_SendSetLocalAccessPhoneNumber(ServerState *state, phoneNumber *newAccessPhoneNumber, phoneNumber *fallbackAccessPhoneNumber, Boolean redial)
{
messOut				opCode;
short				dialAgainFlag;
	opCode = msSetLocalAccessPhoneNumber;
	Server_TWriteDataSync(state->session, sizeof(messOut), (Ptr)&opCode);
	Server_TWriteDataSync(state->session, sizeof(phoneNumber), (Ptr)newAccessPhoneNumber);
	Server_TWriteDataSync(state->session, sizeof(phoneNumber), (Ptr)fallbackAccessPhoneNumber);

	if(redial)
		dialAgainFlag = kRedialTheNetwork;
	else
		dialAgainFlag = kDontRedialTheNetwork;
	Server_TWriteDataSync(state->session, sizeof(short), (Ptr)&dialAgainFlag);

	Logmsg("Sent LocalAccessNumber %s [%d]  fallback %s [%d]\n",
		newAccessPhoneNumber->phoneNumber, newAccessPhoneNumber->scriptID,
		fallbackAccessPhoneNumber->phoneNumber, fallbackAccessPhoneNumber->scriptID);

	return(kServerFuncOK);
}

int Server_ProcessSendQ(ServerState *state)
{
short 		i;
QItem		*item;
Mail		*mail;
GameResult	*gameResult;

	Logmsg("Server_ProcessSendQ:\n");

	for(i = 0; i < state->sendQData.count; i++)
	{
		item = &state->sendQData.items[i];
//
// there are no types anymore, just a DBID.  8/4/94
//
		switch (item->theID) {
		case kRestartInfoType:
			// dump the crash record for Andy1
			FLogmsg(LOG_CRASH, "RESTART INFO for '%s' (%s) (OS Patch version =%ld) (theID=%ld), size=%ld\n",
				state->loginData.userID.userName,
				state->boxPhoneNumber.phoneNumber,
				state->systemVersionData.version,
				(long)item->theID, (long)item->size);
			PrintCrashRecord(item->data, item->size);
			break;
		case kChatDebugType:
		case kDebugChatScriptConst800:
			// dump the chat script debug for Brian
			if (item->theID == kChatDebugType)
				FLogmsg(LOG_DBUG, "CHAT SCRIPT DEBUG X.25 (theID=%ld, size=%ld)\n",
					(long)item->theID, (long)item->size);
			else
				FLogmsg(LOG_DBUG, "CHAT SCRIPT DEBUG 800 (theID=%ld, size=%ld)\n",
					(long)item->theID, (long)item->size);
			FLogmsg(LOG_DBUG, "SCRIPT INFO for '%s' (%s) (theID=%ld), size=%ld\n",
				state->loginData.userID.userName,
				state->boxPhoneNumber.phoneNumber,
				(long)item->theID, (long)item->size);
			FLoghexdump(LOG_DBUG, item->data, item->size);
			break;
		case kGameResultDataDBID:
			// dump the game results for Steve
			FLogmsg(LOG_GAMERESULT, "GAME RESULTS for '%s' (%s) (theID=%ld), size=%ld\n",
				state->loginData.userID.userName,
				state->boxPhoneNumber.phoneNumber,
				(long)item->theID, (long)item->size);
			DumpJamsStats(item->data, item->size);
			break;
		default:
			// just drop it
			Logmsg("Server_ProcessSendQ: ignored item with ID %ld\n",
				(long)item->theID);
			break;
		}
	}

	Logmsg("Server_ProcessSendQ done\n");
	return(kServerFuncOK);
}


//
// go thru the addr book and validate the entries.
// if valid, send em, if not, send dialog and remove msg
//
//
// This brings up the issue of slave registers, then changes playerinfo while waiting.  this is exchanged.
// but the handle is non-unique or something, so the data that was sent to the opponent during peer connect
// is invalid.  This will be dealt with by either not letting slaves change Info while waiting, or by them
// only exchanging validated Info and queuing up a changed Info if they change while waiting for 