/*
	File:		DataBase_Lata.c

	Contains:	LATA database access stuff

	Written by:	Andy McFadden

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<20>	 9/15/94	DJ		added DataBase_OverridePOP (allows you to create file
									CPSNUM.override that lets you set the POP for individual box
									phones)
		<19>	 9/13/94	ATM		Combined Reloads into one.  Removed LataLookup.  Fixed some
									error messages.
		<18>	 9/12/94	DJ		tweak in case no Compuserve POP file
		<17>	 9/12/94	DJ		added Compuserve POP parsing and lookup
		<16>	 9/12/94	ATM		Okay, try that again.
		<15>	 9/12/94	ATM		Print a message when it gets loaded.
		<14>	 9/12/94	ATM		Print an error message if a line in the LATA file is garbled.
		<13>	 9/11/94	ATM		Moved SplitPhoneNumber in here.
		<12>	  9/1/94	ATM		Add LataIsLocal and "regions".
		<11>	 8/24/94	ATM		MPW sucks.
		<10>	 8/24/94	ATM		Added reload stuff.
		 <9>	 8/24/94	ATM		Add area code wildcards to avoid "Somewhere, USA" stuff.
		 <8>	 8/21/94	ATM		Added altPopPhone stuff.
		 <7>	 8/18/94	BET		Forgot the script ID.  Also convert the LATA script IDs to
									constants.
		 <6>	 8/18/94	BET		Change default pad to compuserve
		 <5>	 8/13/94	DJ		make compile on mac
		 <4>	 8/12/94	ATM		No lata DB == okay.
		 <3>	 8/12/94	ATM		No lata file == okay.
		 <2>	 8/12/94	ATM		Made LATA reads come from a file.
		 <1>	 8/11/94	ATM		first checked in

	To Do:
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Server.h"
#include "ServerDataBase.h"
#include "ServerDataBase_priv.h"

#include <ctype.h>


#define X25SCRIPTID 5	/* compuserve */
#define X25PAD      "14089885366"

//#define X25SCRIPTID 2	/* sprint */
//#define X25PAD      "14082866340"

typedef struct Lata {
	char areaCode[4];
	char prefix[4];

	char town[kUserTownSize];
	char state[3];
	long region;
	DBID scriptID;
	char popPhone[kPhoneNumberSize];
	DBID altScriptID;
	char altPopPhone[kPhoneNumberSize];
} Lata;

static ListHead *lataDB = NULL;


#define kSDB_FakeLata	"LATA"
#define MAX_FAKE_LATA_LINE	160
#define WHITESPACE	" \t\n"




#define kSDB_CompuservePOPs "CPSNUM.catapult"
#define kSDB_OverridePOPs "CPSNUM.override"


#define COMPUSERVE_SCRIPTID 5	/* is the X25 script id */

#define COMPUSERVE_VADIC	"VADIC"
#define COMPUSERVE_224MNP	"224MNP"
#define COMPUSERVE_2400		"2400"


/*
	The structure of a Compuserve pop file is that each line of text has the
	entries at a specific char position on the line.  fields are delimited by
	spaces... no tabs.  So to find a given field, we simply index into the
	line by a specified offset.
	eg:
Calgary           AB 403/294-9120  300  1200 2400 9600      CPS V32      
Edmonton          AB 403/466-5083  300  1200 2400           CPS 224MNP   

	Here are the offsets
city = 0 - 17
state = 18-19
area code = 21-23
slash = 24
prefix = 25-27
dash = 28
suffix = 29-32
300 baud = 35- 37
1200 = 40-43
2400 = 45-48
9600 = 50-53
14.4 = 55-58
CPS = 60-62
type = 64-69
eol = 72
*/

#define CS_CITY		0
#define CS_STATE	18
#define CS_AREACODE	21
#define CS_SLASH	24
#define CS_PREFIX	25
#define CS_DASH		28
#define CS_SUFFIX	29
#define CS_300		35
#define CS_1200		40
#define CS_2400		45
#define CS_9600		50
#define CS_144		55
#define CS_CPS		60
#define CS_TYPE		64
#define CS_EOL		72



static ListHead *csDB = NULL;

typedef struct areaPOP {
	char areaCode[4];

	ListHead *pops;
	
	Lata	defaultPop;
} areaPOP;





//
// This lets you route specific phone numbers through a specific POP.
// Very useful for Compuserve testing.
//
// Format is similar to LATA file:
//
// boxphone    popPhone  scriptID   altPopPhone  altScriptID
//

static ListHead *overrideList = NULL;

typedef struct overridePOP {
	char boxPhone[kPhoneNumberSize];


	DBID scriptID;
	char popPhone[kPhoneNumberSize];
	DBID altScriptID;
	char altPopPhone[kPhoneNumberSize];
} overridePOP;


Err DataBase_LoadOverridePOPs(void);
Err DataBase_OverrideLookup(phoneNumber *boxPhone, phoneNumber *popPhone, phoneNumber *altPopPhone);


Err DataBase_LoadOverridePOPs(void)
{
	char 		linebuf[MAX_FAKE_LATA_LINE];
	FILE 		*fp;
	ListNode 	*lnode;
	overridePOP *lptr;
	char 		*cp;
	char 		suffix[20];
	register int a;
	areaPOP		*areaPops;
	long 		areaCode;
	long		numPops;


	if ((fp = fopen(kSDB_OverridePOPs, "rb")) == NULL) {
		Logmsg("There is no POP override file '%s'\n", kSDB_OverridePOPs);
		return (kNoError);
	}

	overrideList = NewSortedList();

	numPops = 0;
	while (1) {

#ifdef THINK_C
		
		for(a = 0; a < MAX_FAKE_LATA_LINE; a++)
		{
			if(fscanf(fp, "%c", &linebuf[a]) != 1)
				break;	// eof
			
			if(linebuf[a] == '\n' || linebuf[a] == '\r')
			{
				linebuf[a] = '\0';
				break;
			}
		}
#else
		fgets(linebuf, MAX_FAKE_LATA_LINE, fp);
#endif

		if (feof(fp) || ferror(fp)) break;

		if (linebuf[strlen(linebuf)-1] == '\n')
			linebuf[strlen(linebuf)-1] = '\0';
		if (linebuf[0] == '\0' || linebuf[0] == '#') continue;



		lptr = (overridePOP *)malloc(sizeof(overridePOP));
		ASSERT(lptr);

		// box number that is getting rerouted
		//
		if ((cp = strtok(linebuf, WHITESPACE)) == NULL)
			goto bogusline;
		strncpy(lptr->boxPhone, cp, kPhoneNumberSize-1);
		lptr->boxPhone[kPhoneNumberSize-1] = '\0';


		// popPhone
		if ((cp = strtok(NULL, WHITESPACE)) == NULL)
			goto bogusline;
		strncpy(lptr->popPhone, cp, kPhoneNumberSize-1);
		lptr->popPhone[kPhoneNumberSize-1] = '\0';

		// scriptID
		if ((cp = strtok(NULL, WHITESPACE)) == NULL)
			goto bogusline;
		lptr->scriptID = atol(cp);


		// altPopPhone
		if ((cp = strtok(NULL, WHITESPACE)) == NULL)
			goto bogusline;
		strncpy(lptr->altPopPhone, cp, kPhoneNumberSize-1);
		lptr->altPopPhone[kPhoneNumberSize-1] = '\0';

		// altScriptID
		if ((cp = strtok(NULL, WHITESPACE)) == NULL)
			goto bogusline;
		lptr->altScriptID = atol(cp);



		lnode = NewListNode((Ptr) lptr);
		AddListNodeToList(overrideList, lnode);


		numPops++;
		continue;
bogusline:
		Logmsg("OverridePOP: bogus line '%s'\n", linebuf);
		free(lptr);
		// keep going
	}



	fclose(fp);

	Logmsg("Override POP database read (%ld items)\n", numPops);

	return (kNoError);
}



//
// Load the Compuserve POP database
//
Err DataBase_LoadCompuservePOPs(void)
{
	char 		linebuf[MAX_FAKE_LATA_LINE];
	FILE 		*fp;
	ListNode 	*lnode;
	Lata 		*lptr;
	char 		*cp;
	char 		suffix[20];
	register int a;
	areaPOP		*areaPops;
	long 		areaCode;
	long		numPops;


	if ((fp = fopen(kSDB_CompuservePOPs, "rb")) == NULL) {
		Logmsg("Unable to find CompuServe POP db '%s'\n", kSDB_CompuservePOPs);
		return (kFucked);
	}

	//
	// Don't recreate if some POPs are already loaded.
	//

	if(!csDB)
		csDB = NewSortedList();

	numPops = 0;
	while (1) {

#ifdef THINK_C
		
		for(a = 0; a < MAX_FAKE_LATA_LINE; a++)
		{
			if(fscanf(fp, "%c", &linebuf[a]) != 1)
				break;	// eof
			
			if(linebuf[a] == '\n' || linebuf[a] == '\r')
			{
				linebuf[a] = '\0';
				break;
			}
		}
#else
		fgets(linebuf, MAX_FAKE_LATA_LINE, fp);
#endif

		if (feof(fp) || ferror(fp)) break;

		if (linebuf[strlen(linebuf)-1] == '\n')
			linebuf[strlen(linebuf)-1] = '\0';
		if (linebuf[0] == '\0' || linebuf[0] == '#') continue;


		//
		// In major cities, Compuserve has some 1200 baud only VADIC type modems.
		// and some 9600 only v32 and v32.bis modems.
		// We don't want to add those to our POP db.
		//
/*
		linebuf[CS_EOL - 1] = '\0';
		cp = &linebuf[CS_TYPE];
		if(strncmp(cp, COMPUSERVE_224MNP, strlen(COMPUSERVE_224MNP)))
			continue;
*/

		//
		// Only store numbers that support 2400 baud.
		//
		cp = &linebuf[CS_2400];
		if(strncmp(cp, COMPUSERVE_2400, strlen(COMPUSERVE_2400)) != 0)
			continue;

		

		lptr = (Lata *)malloc(sizeof(Lata));
		ASSERT(lptr);


		cp = &linebuf[CS_CITY];
		linebuf[CS_STATE - 1] = '\0';
		for(a = CS_STATE - 2; a >= CS_CITY && linebuf[a] == ' '; a--)	// eat off trailing whitespace.
			linebuf[a] = '\0';
		strncpy(lptr->town, cp, kUserTownSize-1);
		lptr->town[kUserTownSize-1] = '\0';

		cp = &linebuf[CS_STATE];
		linebuf[CS_AREACODE - 1] = '\0';
		strncpy(lptr->state, cp, 2);
		lptr->state[2] = '\0';

		cp = &linebuf[CS_AREACODE];
		linebuf[CS_SLASH] = '\0';
		strncpy(lptr->areaCode, cp, 3);
		lptr->areaCode[3] = '\0';
		
		cp = &linebuf[CS_PREFIX];
		linebuf[CS_DASH] = '\0';
		strncpy(lptr->prefix, cp, 3);
		lptr->prefix[3] = '\0';

		lptr->region = 0;		// region 0 is the wildcard (matches both east and west coast).
		
		cp = &linebuf[CS_SUFFIX];
		linebuf[CS_300 - 1] = '\0';
		strncpy(suffix, cp, 4);
		suffix[4] = '\0';

		lptr->scriptID = COMPUSERVE_SCRIPTID;
		
		// now create the popphone.
		sprintf(lptr->popPhone, "1%s%s%s", lptr->areaCode, lptr->prefix, suffix);
		
		strncpy(lptr->altPopPhone, lptr->popPhone, kPhoneNumberSize - 1);	// make 'em equal.
		lptr->altScriptID = COMPUSERVE_SCRIPTID;

		areaCode = (long)atoi(lptr->areaCode);
		lnode = SearchList(csDB, areaCode);
		if(!lnode)
		{
			areaPops = (areaPOP *)malloc(sizeof(areaPOP));
			ASSERT(areaPops);
			strcpy(areaPops->areaCode, lptr->areaCode);
			areaPops->pops = NewList();

			lnode = NewSortedListNode((Ptr)areaPops, areaCode);
			AddListNodeToSortedList(csDB, lnode);
			
			areaPops->defaultPop = *lptr;	// use the first POP as the default for this area code.
		}
		else
		{
			areaPops = (areaPOP *)GetListNodeData(lnode);
			ASSERT(areaPops);
		}

		lnode = NewListNode((Ptr) lptr);
		AddListNodeToList(areaPops->pops, lnode);

		numPops++;
		continue;
bogusline:
		Logmsg("COMPUSERVE: bogus line '%s'\n", linebuf);
		free(lptr);
		// keep going
	}

	fclose(fp);

	Logmsg("Compuserve POP database read (%ld items)\n", numPops);

	DataBase_LoadOverridePOPs();

	return (kNoError);
}

Err DataBase_OverrideLookup(phoneNumber *boxPhone, phoneNumber *popPhone, phoneNumber *altPopPhone)
{
	ListNode *lnode;
	overridePOP *latap;

	if(!overrideList)
		return(kNoError);



	for (lnode = GetFirstListNode(overrideList); lnode; lnode = GetNextListNode(lnode)) {
		latap = (overridePOP *)GetListNodeData(lnode);

		if(!strcmp(boxPhone->phoneNumber, latap->boxPhone))
		{
			// found a match!
			Logmsg("Overrode POP for boxPhone #%s.  Normal is %s, overridden is %s\n",
					boxPhone->phoneNumber, popPhone->phoneNumber, latap->popPhone);

			popPhone->scriptID = latap->scriptID;
			strcpy(popPhone->phoneNumber, latap->popPhone);
			altPopPhone->scriptID = latap->altScriptID;
			strcpy(altPopPhone->phoneNumber, latap->altPopPhone);

			break;
		}
	}

	return(kNoError);
}


//
// Consult the LATA database to get the user's POP phone and hometown.
//
// Stuffs the results into popPhone and homeTown.
//
Err DataBase_POPLookup(phoneNumber *boxPhone, phoneNumber *popPhone, phoneNumber *altPopPhone, Hometown homeTown)
{
	static phoneNumber defaultPopPhone = { X25SCRIPTID, 0, X25PAD };
	char popAreaCode[kPhoneNumberSize], popPrefix[kPhoneNumberSize],
		popSuffix[kPhoneNumberSize];
	char altPopAreaCode[kPhoneNumberSize], altPopPrefix[kPhoneNumberSize],
		altPopSuffix[kPhoneNumberSize];
	char boxAreaCode[kPhoneNumberSize], boxPrefix[kPhoneNumberSize],
		boxSuffix[kPhoneNumberSize];
	Lata *latap;
	ListNode *lnode;
	char area[4];
	areaPOP *areaPops;
	long	areaCode;


	// Initialize the result to a default.
	//
	*popPhone = defaultPopPhone;
	*altPopPhone = defaultPopPhone;
	strcpy(homeTown, "Unknown, USA");

	if (!csDB)
	{
		DataBase_OverrideLookup(boxPhone, popPhone, altPopPhone);
		return (kNoError);		// return default
	}

	DataBase_SplitPhone(boxPhone->phoneNumber,
		boxAreaCode, boxPrefix, boxSuffix);

	// Assume any box without an area code is in 408
	if (!strlen(boxAreaCode)) {
		strcpy(boxAreaCode, "1408");
		Logmsg("GLITCH: POPLookup on phone number with no area code\n");
	}

	
	// grab the three-digit box area code for the LATA lookup
	if (strlen(boxAreaCode) == 4)
		strcpy(area, boxAreaCode+1);
	else
		strcpy(area, boxAreaCode);



	//
	// Now find the list of POPs for this area code.
	//
	if(csDB)
	{
		areaCode = (long)atoi(area);
		lnode = SearchList(csDB, areaCode);
		if(lnode)
		{
			areaPops = (areaPOP *)GetListNodeData(lnode);
			ASSERT(areaPops);
			
			
			for (lnode = GetFirstListNode(areaPops->pops); lnode; lnode = GetNextListNode(lnode)) {
				latap = (Lata *)GetListNodeData(lnode);
				ASSERT(strcmp(area, latap->areaCode) == 0 );
	
				if(strcmp(boxPrefix, latap->prefix) == 0 ||
				   strcmp(latap->prefix, "