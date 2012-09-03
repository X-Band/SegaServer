/*
	File:		DataBase_News.c

	Contains:	xxx put contents here xxx

	Written by:	Jevans

	Copyright:	© 1994 by Catapult Entertainment, Inc., all rights reserved.

	Change History (most recent first):

		<34>	  9/1/94	ATM		Changed Loghexdump.
		<33>	 8/27/94	ATM		First pass at adding animation downloading.
		<32>	 8/24/94	ATM		Added reload stuff.
		<31>	 8/24/94	ATM		Make the 0x40 constants for colorFlags and pageFlags easier to
									deal with.
		<30>	 8/20/94	ATM		Put color3 fields back in.
		<29>	 8/20/94	BET		Changed a field name to backgroundSoundFX from some other
									similar name with the same semantics.
		<28>	 8/20/94	ATM		Changed KONstants and increased buffer size (again).
		<27>	 8/19/94	BET		Reconcile with Kon's news changes, set old fields to zero out of
									paranoia.
		<26>	 8/19/94	ATM		Define and handle text justification stuff.
		<25>	 8/17/94	ATM		Implemented \xnn escape, added escape processing for headlines.
		<24>	 8/15/94	ATM		Fixed pageFlags bug, increased buffer sizes to accommodate
									Joey's testing.
		<23>	 8/15/94	ATM		Updated size of News read buffer to hold longer strings (smaller
									fonts...).
		<22>	 8/13/94	ATM		Added more constants, changed atoi() to strtol(), allow constant
									subs in XXXXd font.
		<21>	 8/11/94	ATM		Fixed minor bug in ReplaceEscapedChars.
		<20>	 8/11/94	ATM		Added support for stuff like '\n'.
		<19>	 8/11/94	ATM		Converted to Logmsg.
		<18>	  8/8/94	ATM		Made Thick C happy.
		<17>	  8/8/94	ATM		Rewrote news handling.
		<16>	  8/4/94	DJ		calling PackStrings, which is also used by rankings
		<15>	 7/27/94	DJ		setting lastPageFlag for last page
		<14>	 7/26/94	DJ		setting lastpageflag for last page of each news
		<13>	 7/25/94	DJ		updated to even newer newspage
		<12>	 7/24/94	BET		updated to new newspage struct
		<11>	 7/20/94	DJ		no ServerState passed to dbase routines
		<10>	 7/18/94	DJ		nuthin mutch
		 <9>	  7/8/94	DJ		send different content for both kinds of news
		 <8>	  7/6/94	DJ		set sound field
		 <7>	  7/5/94	DJ		clearing animation fields in news page
		 <6>	  7/2/94	DJ		using form1 instead of form0
		 <5>	 6/30/94	DJ		send both kinds of news
		 <4>	 6/30/94	DJ		new news
		 <3>	 6/10/94	DJ		new packed string news format
		 <2>	  6/3/94	DJ		sending fine animated news
		 <1>	  6/2/94	BET		first checked in

	To Do:
*/

#include "Server.h"
#include "ServerDataBase.h"
#include "ServerDataBase_priv.h"
#include "ServerDataBase_FILE.h"
#include "Errors.h"

#include "PreformedMessage.h"

#include "GraphicsDB.h"
#include "Dates.h"
//#include "TextUtls.h"
# define kJustLeft	0
# define kJustCenter	1
# define kJustRight		2

#include "DBTypes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


int DataBase_SendDBFromNews(char *line);


// #of elements in a static array
#define NELEM(array)	(sizeof(array) / sizeof(array[0]))


//
// Init database news section.
//
SDBNews *SDBNews_New(void)
{
	SDBNews *news;

	news = (SDBNews *)malloc(sizeof(SDBNews));
	ASSERT(news);
	if (!news) {
		ERROR_MESG("out of mems");
		return (NULL);
	}

	news->paperList = NewList();
	return (news);
}


//
// Count the #of news pages of a given type.
//
long DataBase_GetNumNewsPages(DBType pageType)
{
	SDBNews *news;
	SDBNewsPaper *paper;
	ListNode *node;

	news = SDB_GetNews(gSDB);
	for (node = GetFirstListNode(news->paperList); node != NULL; node = GetNextListNode(node)) {
		paper = (SDBNewsPaper *) GetListNodeData(node);
		ASSERT(paper);

		if (paper->pageType == pageType)
			return (NumListNodesInList(paper->pageList));
	}
	if (node == NULL)
		return (0);
}

//
// Return a copy of the appropriate ServerNewsPage.  Returns NULL if the
// requested page can't be found.
//
ServerNewsPage *DataBase_GetNewsPage(long pageNum, DBType pageType)
{
	SDBNews *news;
	SDBNewsPaper *paper;
	ServerNewsPage *snp;
	ListNode *node;

	news = SDB_GetNews(gSDB);

	// find the appropriate newspaper
	for (node = GetFirstListNode(news->paperList); node != NULL; node = GetNextListNode(node)) {
		paper = (SDBNewsPaper *) GetListNodeData(node);
		ASSERT(paper);

		if (paper->pageType == pageType)
			break;
	}
	if (node == NULL) {
		Logmsg("WARNING: news type %d not found!\n", pageType);
		return (NULL);
	}

	// find the requested page
	node = SearchList(paper->pageList, pageNum);
	if (node == NULL) {
		Logmsg("WARNING: news type %d page %d not found!\n",
			pageType, pageNum);
		return (NULL);
	}

	snp = (ServerNewsPage *)GetListNodeData(node);
	return (snp);
}

void DataBase_ReleaseServerNewsPage(ServerNewsPage *page)
{
	// nothing to do
}


//
// Load news into database.  A news file may contain several "newspapers",
// each of which is divided into "newspages".
//
// Syntax rules (similar to Mail stuff):
// - Blank lines and lines beginning with '#' are ignored.
// - Everything up to the first "%%" is ignored.
// - Every newspaper must be followed by "%%".  Every page must begin with
//   "&&".
// - There are no optional fields.  Some fields may be left blank, but
//   all of the tags must be there.
// - If garbage is detected, this routine scans ahead to the next %% (i.e.
//   to the next newspaper).
// - If the data tag for a line is blank, then that field is ignored.  However,
//   both data and text fields should still be there (just blank).  (This makes
//   it easier to parse, and makes sure that you aren't forgetting fields.)
// - String constant substitution is performed on the PAGTY line.  It may
//   be added for other lines.
//
//
// Tags ending with "d" are data, tags ending with "t" are text.  The text
// tags are simple strings, the data tags look like:
// 
//		fontID color0 color1 color2 color3
// 
// #
// # Sample news file.
// #
// %%
// PAGTY kOtherNews
// DBADD [ database additions, like animations ]
// &&
// HEADd 4  13 0 0 0
// HEADt THE HEADLINE
// TAGLd 0  0 9 8 0
// TAGLt Tag Line Here
// BOD1d 0  0 2 1 0
// BOD1t This is the main part of the message.
// BOD2d 0  0 2 1 0
// BOD2t Second, smaller part
// DATEd 0  0 11 10 0
// DATEt 1994 8 7
// PGNMd 0  0 15 14 0
// PGNMt Page 1
// ANIM1 0
// MUSIC 2
// BITMP kNBAJamLogo
// BITXP 2
// BITYP 13
// CLRFL 0
// &&
// HEADd 4  13 0 0 0
// HEADt NEXT HEADLINE
// TAGLd 0  0 9 8 0
// TAGLt Early Edition
//	...
// BITXP 2
// BITYP 13
// CLRFL 0
// %%
// 
// PAGTY kNewsForm
// &&
// HEADd 4  13 0 0 0
// HEADt XBAND ON THE AIR
//	...
// %%
// 
#define MY_MAX_NEWS_LINE_SIZE	800		// all lines
#define MY_MAX_SHORT_LINE_SIZE	64		// short character strings
#define STR_OFFSET 6

Err DataBase_LoadNewsPages()
{
FILE	*fp;
char	pageNum;
int		state, ptentry, len, i;
long	lval, year, month, day;
NewsPage tmpNewsPage, *newPage;
SDBNewsPaper *newPaper;
ServerNewsPage *snp;
DBType	pageType;
FontID	font;
SDBNews	*news;
ListNode *node;
char	col0, col1, col2, col3;
char	linebuf[MY_MAX_NEWS_LINE_SIZE+7];
int		line = 0;

// String parsing buffers.
char	tmpHead[MY_MAX_NEWS_LINE_SIZE /*MY_MAX_SHORT_LINE_SIZE*/],
		tmpTagl[MY_MAX_SHORT_LINE_SIZE],
		tmpBod1[MY_MAX_NEWS_LINE_SIZE],
		tmpBod2[MY_MAX_NEWS_LINE_SIZE],
		tmpDate[MY_MAX_SHORT_LINE_SIZE],
		tmpPgnm[MY_MAX_SHORT_LINE_SIZE];
char *fakeStrings[6];		// 6 == #of string buffers above
int numFakeStrings;

// Parse table.  This is a job for lex...
enum { kInit, kReset, kLookForPercent, kLookForAmperOrDbadd, kLookForPercOrAmper,
	kPAGTY, kDBADD, kNSFID,
	kHEADd, kHEADt, kTAGLd, kTAGLt, kBOD1d, kBOD1t,
	kBOD2d, kBOD2t, kDATEd, kDATEt, kPGNMd, kPGNMt,
	kANIM1, kMUSIC, kBITMP, kBITXP, kBITYP, kCLRFL
};
static struct ParseTable {
	int		currentState;
	char	*stateName;
	int		nextState;
} parseTable[] = {
	// (handle kReset & kInit explicitly)
	{ kLookForPercent, "%%", kPAGTY },
	{ kPAGTY,	"PAGTY",	kLookForAmperOrDbadd },
	//{ kLookForAmper, "&&",	kNSFID },
	// (handle kLookForAmperOrAnim explicitly)
	{ kNSFID,	"NSFID",	kHEADd },
	{ kHEADd,	"HEADd",	kHEADt },
	{ kHEADt,	"HEADt",	kTAGLd },
	{ kTAGLd,	"TAGLd",	kTAGLt },
	{ kTAGLt,	"TAGLt",	kBOD1d },
	{ kBOD1d,	"BOD1d",	kBOD1t },
	{ kBOD1t,	"BOD1t",	kBOD2d },
	{ kBOD2d,	"BOD2d",	kBOD2t },
	{ kBOD2t,	"BOD2t",	kDATEd },
	{ kDATEd,	"DATEd",	kDATEt },
	{ kDATEt,	"DATEt",	kPGNMd },
	{ kPGNMd,	"PGNMd",	kPGNMt },
	{ kPGNMt,	"PGNMt",	kANIM1 },
	{ kANIM1,	"ANIM1",	kMUSIC },
	{ kMUSIC,	"MUSIC",	kBITMP },
	{ kBITMP,	"BITMP",	kBITXP },
	{ kBITXP,	"BITXP",	kBITYP },
	{ kBITYP,	"BITYP",	kCLRFL },
	{ kCLRFL,	"CLRFL",	kLookForPercOrAmper },
	// (handle kLookForPercOrAmper explicitly)
};

	news = SDB_GetNews(gSDB);

	if (NumListNodesInList(news->paperList) != 0) {
		// should free old, then reload...
		ERROR_MESG("HEY: LoadNews was called twice!\n");	// fix later??
		DisposeList(news->paperList);
		news->paperList = NewList();
	}

	if ((fp = fopen(kSDB_NewsFile, "r")) == NULL) {		// do NOT want "rb"
		Logmsg("NOTE: couldn't open '%s'\n", kSDB_NewsFile);
		return (kNoError);	// little white lie
	}
	
	state = kInit;
	while (1) {
		if (state == kInit || state == kReset) {
			// (re)initialize data
			pageType = 0;
			pageNum = 0;
			memset((char *)&tmpNewsPage, 0, sizeof(tmpNewsPage));
			tmpHead[0] = tmpTagl[0] = tmpBod1[0] = tmpBod2[0] = tmpDate[0] =
				tmpPgnm[0] = '\0';
			newPaper = NULL;

			if (state == kInit)
				state = kLookForPercent;		// find initial '%%'
			else
				state = kPAGTY;					// starting new newspaper
			continue;
		}

		fgets(linebuf, MY_MAX_NEWS_LINE_SIZE+7, fp);
		line++;
		if (feof(fp) || ferror(fp)) break;

		if (linebuf[strlen(linebuf)-1] == '\n')
			linebuf[strlen(linebuf)-1] = '\0';
		if (linebuf[0] == '\0' || linebuf[0] == '#') continue;

		// Icky nasty: if line is too short, linebuf+STR_OFFSET will get
		// garbage.
		if (strlen(linebuf) < STR_OFFSET)
			linebuf[STR_OFFSET] = '\0';

		// This doesn't work as a table entry, so deal with it directly.
		if (state == kLookForPercOrAmper) {
			if (strcmp(linebuf, "&&") == 0 || strcmp(linebuf, "%%") == 0) {
				// end of page, do something appropriate
				tmpNewsPage.pageNum = ++pageNum;

				// BUG: if we get an error reading a news page, this will
				// never get set.  Either need to amputate the entire
				// newspaper, or run through at the end and set this.
				//
				// If kLastNewsPageMask doesn't get set, the right icon
				// gets highlighted even if the page isn't there.  No big
				// deal.
				//
				// (I'm leaving it be for now, assuming that moving it
				// into the database will partially resolve the problem.)
				if (strcmp(linebuf, "%%") == 0)
					tmpNewsPage.pageNum |= kLastNewsPageMask;

				// alloc a newspaper if this is a new one
				if (newPaper == NULL) {
					newPaper = (SDBNewsPaper *)malloc(sizeof(SDBNewsPaper));
					ASSERT(newPaper);
					newPaper->pageType = pageType;
					newPaper->pageList = NewSortedList();
					node = NewListNode((Ptr)newPaper);
					AddListNodeToList(news->paperList, node);

				}

				// modify the pageFlags and colorFlags
				//
				// for the DisableXBandHeadline shit:
				//	- for BANDWIDTH news, both should be set
				//	- for first page of XBAND news, both should be clear
				//	- for 2nd and later pages of XBAND news, kColorFlag is
				//	  clear, kPageFlag is set
				if (pageType == kOtherNews) {
					// XBAND news page
					tmpNewsPage.colorFlags &= ~kColorFlag_DisableXBandHeadline;

					if (pageNum == 1)
						tmpNewsPage.pageFlags &= ~kPageFlag_DisableXBandAnimation;
					else
						tmpNewsPage.pageFlags |= kPageFlag_DisableXBandAnimation;
				} else if (pageType == kOtherNews) {
					// BANDWIDTH news page
					tmpNewsPage.colorFlags |= kColorFlag_DisableXBandHeadline;
					tmpNewsPage.pageFlags |= kPageFlag_DisableXBandAnimation;
				}


				// set up fakeStrings, numFakeStrings, and len
				numFakeStrings = len = 0;
				if (tmpNewsPage.pageFlags & kPageFlag_Headline) {
					len += strlen(tmpHead) +1;
					fakeStrings[numFakeStrings++] = tmpHead;
				}
				if (tmpNewsPage.pageFlags & kPageFlag_Tag) {
					len += strlen(tmpTagl) +1;
					fakeStrings[numFakeStrings++] = tmpTagl;
				}
				if (tmpNewsPage.pageFlags & kPageFlag_Body1) {
					len += strlen(tmpBod1) +1;
					fakeStrings[numFakeStrings++] = tmpBod1;
				}
				if (tmpNewsPage.pageFlags & kPageFlag_Body2) {
					len += strlen(tmpBod2) +1;
					fakeStrings[numFakeStrings++] = tmpBod2;
				}
				if (tmpNewsPage.pageFlags & kPageFlag_Date) {
					len += strlen(tmpDate) +1;
					fakeStrings[numFakeStrings++] = tmpDate;
				}
				if (tmpNewsPage.pageFlags & kPageFlag_PageNum) {
					len += strlen(tmpPgnm) +1;
					fakeStrings[numFakeStrings++] = tmpPgnm;
				}

				// alloc the newspage
				len += (sizeof(NewsPage) -1);
				newPage = (NewsPage *)malloc(len);
				ASSERT(newPage);
				// copy the body over
				memcpy(newPage, &tmpNewsPage, sizeof(NewsPage));

				PackStrings(newPage->text, numFakeStrings, fakeStrings);

				// Need to associate the NewsPage with a length, so we
				// use a ServerNewsPage.
				snp = (ServerNewsPage *)malloc(sizeof(ServerNewsPage));
				ASSERT(snp);
				snp->type = pageType;
				snp->length = len;
				snp->page = newPage;

				// add the ServerNewsPage to the list
				node = NewSortedListNode((Ptr) snp,
					((unsigned char)snp->page->pageNum & ~kLastNewsPageMask)-1);
				AddListNodeToSortedList(newPaper->pageList, node);
				//Logmsg("Added type %d page %d\n", pageType,
				//	((unsigned char)snp->page->pageNum & ~kLastNewsPageMask)-1);

				// get on with it
				if (strcmp(linebuf, "&&") == 0) {
					state = kNSFID;
					memset((char *)&tmpNewsPage, 0, sizeof(tmpNewsPage));
					tmpHead[0] = tmpTagl[0] = tmpBod1[0] = tmpBod2[0] =
						tmpDate[0] = tmpPgnm[0] = '\0';
				} else
					state = kReset;
			}
			continue;
		}

		// Look for && or one or more DBADDs
		if (state == kLookForAmperOrDbadd) {
			if (strcmp(linebuf, "&&") == 0) {
				state = kNSFID;
				continue;
			}

			DataBase_SendDBFromNews(linebuf+STR_OFFSET);

			// stay in same state
			continue;
		}

		// Find state in table.  This could be made more efficient, but
		// it's only done during startup (and when new news arrives?), so
		// just keep it simple.
		for (ptentry = 0; ptentry < NELEM(parseTable); ptentry++) {
			if (state == parseTable[ptentry].currentState)
				break;
		}
		if (ptentry == NELEM(parseTable)) {
			ERROR_MESG("Bogus state in LoadNewsPages.");
			goto error;
		}
		if (strncmp(linebuf, parseTable[ptentry].stateName,
			strlen(parseTable[ptentry].stateName) != 0))
		{
			// got the wrong keyword here; user is a pinhead
			// (could be a garbaged file and we're scanning for next '%%')
			if (state != kLookForPercent) {
				Logmsg("NEWS: line %d: bad keyword '%s', expected '%s' (%d)\n",
					line, linebuf, parseTable[ptentry].stateName, ptentry);
			}

			state = kInit;
			continue;
		}

		//
		// We got it, now do something with it.
		//
		switch (state) {
		case kLookForPercent:
			break;
		case kPAGTY:
			// Should probably check to see if the type is a valid pageType,
			// but sometimes it's funny to watch the user hose himself.
			if (!DataBase_TranslateConstant(linebuf+STR_OFFSET, &lval)) {
				state = kInit;
				Logmsg("NEWS: line %d: missing PAGTY rhs\n", line);
				continue;
			}
			pageType = (DBType) lval;
			break;
		case kNSFID:
			if (!DataBase_TranslateConstant(linebuf+STR_OFFSET, &lval)) {
				state = kInit;
				Logmsg("NEWS: line %d: missing NSFID rhs\n", line);
				continue;
			}
			tmpNewsPage.newsFormID = lval;
			break;
		case kHEADd:
			if (!DataBase_ParseNewsDataLine(linebuf+STR_OFFSET,
				&font, &col0, &col1, &col2, &col3))
			{
				break;
			}
			tmpNewsPage.headlineFontID = font;
			tmpNewsPage.headlineColor0 = col0;
			tmpNewsPage.headlineColor1 = col1;
			tmpNewsPage.headlineColor2 = col2;
			tmpNewsPage.headlineDrawFlags = col3;
			tmpNewsPage.pageFlags |= kPageFlag_Headline;
			break;
		case kHEADt:
			if (tmpNewsPage.pageFlags & kPageFlag_Headline) {
				strcpy(tmpHead, linebuf+STR_OFFSET);
				DataBase_ReplaceEscapedChars(tmpHead);
			}
			break;
		case kTAGLd:
			if (!DataBase_ParseNewsDataLine(linebuf+STR_OFFSET,
				&font, &col0, &col1, &col2, &col3))
			{
				break;
			}
			tmpNewsPage.taglineFontID = font;
			tmpNewsPage.taglineColor0 = col0;
			tmpNewsPage.taglineColor1 = col1;
			tmpNewsPage.taglineColor2 = col2;
			tmpNewsPage.taglineNotUsed = col3;
			tmpNewsPage.pageFlags |= kPageFlag_Tag;
			break;
		case kTAGLt:
			if (tmpNewsPage.pageFlags & kPageFlag_Tag)
				strcpy(tmpTagl, linebuf+STR_OFFSET);
			break;
		case kBOD1d:
			if (!DataBase_ParseNewsDataLine(linebuf+STR_OFFSET,
				&font, &col0, &col1, &col2, &col3))
			{
				break;
			}
			tmpNewsPage.body1FontID = font;
			tmpNewsPage.body1Color0 = col0;
			tmpNewsPage.body1Color1 = col1;
			tmpNewsPage.body1Color2 = col2;
			tmpNewsPage.body1DrawFlags = col3;
			tmpNewsPage.pageFlags |= kPageFlag_Body1;
			break;
		case kBOD1t:
			if (tmpNewsPage.pageFlags & kPageFlag_Body1) {
				strcpy(tmpBod1, linebuf+STR_OFFSET);
				DataBase_ReplaceEscapedChars(tmpBod1);
			}
			break;
		case kBOD2d:
			if (!DataBase_ParseNewsDataLine(linebuf+STR_OFFSET,
				&font, &col0, &col1, &col2, &col3))
			{
				break;
			}
			tmpNewsPage.body2FontID = font;
			tmpNewsPage.body2Color0 = col0;
			tmpNewsPage.body2Color1 = col1;
			tmpNewsPage.body2Color2 = col2;
			tmpNewsPage.body2DrawFlags = col3;
			tmpNewsPage.pageFlags |= kPageFlag_Body2;
			break;
		case kBOD2t:
			if (tmpNewsPage.pageFlags & kPageFlag_Body2) {
				strcpy(tmpBod2, linebuf+STR_OFFSET);
				DataBase_ReplaceEscapedChars(tmpBod2);
			}
			break;
		case kDATEd:
			if (!DataBase_ParseNewsDataLine(linebuf+STR_OFFSET,
				&font, &col0, &col1, &col2, &col3))
			{
				break;
			}
			tmpNewsPage.dateFontID = font;
			tmpNewsPage.dateColor0 = col0;
			tmpNewsPage.dateColor1 = col1;
			tmpNewsPage.dateColor2 = col2;
			tmpNewsPage.dateNotUsed = col3;
			tmpNewsPage.pageFlags |= kPageFlag_Date;
			break;
		case kDATEt:
			if (tmpNewsPage.pageFlags & kPageFlag_Date)
				strcpy(tmpDate, linebuf+STR_OFFSET);
			break;
		case kPGNMd:
			if (!DataBase_ParseNewsDataLine(linebuf+STR_OFFSET,
				&font, &col0, &col1, &col2, &col3))
			{
				break;
			}
			tmpNewsPage.pageNumFontID = font;
			tmpNewsPage.pageNumColor0 = col0;
			tmpNewsPage.pageNumColor1 = col1;
			tmpNewsPage.pageNumColor2 = col2;
			tmpNewsPage.pageNumNotUsed1 = col3;
			tmpNewsPage.pageFlags |= kPageFlag_PageNum;
			break;
		case kPGNMt:
			if (tmpNewsPage.pageFlags & kPageFlag_PageNum)
				strcpy(tmpPgnm, linebuf+STR_OFFSET);
			break;
		case kANIM1:
			if (!strlen(linebuf+STR_OFFSET)) {
				state = kInit;
				Logmsg("NEWS: line %d: missing ANIM1 rhs\n", line);
				continue;
			}
			tmpNewsPage.animation1 = strtol(linebuf+STR_OFFSET, NULL, 0);
			break;
		case kMUSIC:
			if (!strlen(linebuf+STR_OFFSET)) {
				state = kInit;
				Logmsg("NEWS: line %d: missing MUSIC rhs\n", line);
				continue;
			}
			tmpNewsPage.backgroundSoundFX = strtol(linebuf+STR_OFFSET, NULL, 0);
			break;
		case kBITMP:
			if (!DataBase_TranslateConstant(linebuf+STR_OFFSET, &lval)) {
				state = kInit;
				Logmsg("NEWS: line %d: missing BITMP rhs\n", line);
				continue;
			}
			tmpNewsPage.bitmapID = lval;
			break;
		case kBITXP:
			if (!strlen(linebuf+STR_OFFSET)) {
				state = kInit;
				Logmsg("NEWS: line %d: missing BITXP rhs\n", line);
				continue;
			}
			tmpNewsPage.bitmapXpos = strtol(linebuf+STR_OFFSET, NULL, 0);
			break;
		case kBITYP:
			if (!strlen(linebuf+STR_OFFSET)) {
				state = kInit;
				Logmsg("NEWS: line %d: missing kBITYP rhs\n", line);
				continue;
			}
			tmpNewsPage.bitmapYpos = strtol(linebuf+STR_OFFSET, NULL, 0);
			break;
		case kCLRFL:
			if (!strlen(linebuf+STR_OFFSET)) {
				state = kInit;
				Logmsg("NEWS: line %d: missing CLRFL rhs\n", line);
				continue;
			}
			tmpNewsPage.colorFlags = strtol(linebuf+STR_OFFSET, NULL, 0);
			break;
		default:
			ERROR_MESG("Invalid state %d in LoadNews");
			goto error;
			break;
		}

		// On to the next state.
		state = parseTable[ptentry].nextState;
	}

error:
	fclose(fp);
	//free(fakem);
	return (kNoError);	// little white lie when exiting via error
}
#undef MY_MAX_NEWS_LINE_SIZE
#undef MY_MAX_SHORT_LINE_SIZE
#undef STR_OFFSET

//
// Parse an XXXXd line from the NewsPages file.
//
Boolean DataBase_ParseNewsDataLine(char *line, FontID *fontid, \
	char *color0, char *color1, char *color2, char *color3)
{
	long f, c0, c1, c2, c3;
	char fontBuf[64], justBuf[64];

	if (sscanf(line, "%s %ld %ld %ld %s", fontBuf, &c0, &c1, &c2, justBuf) != 5)
		return (false);
	DataBase_TranslateConstant(fontBuf, &f);
	*fontid = f;
	*color0 = c0;
	*color1 = c1;
	*color2 = c2;
	DataBas