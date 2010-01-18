/*    con_ncurses.cpp
 *
 *    Ncurses front-end to fte - Don Mahurin
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */
#include "feature.h"

#include <ncurses.h>
#include <unistd.h>

#include "sysdep.h"
#include "c_config.h"
#include "console.h"
#include "gui.h"
#include "con_tty.h"
#include "s_string.h"

/* Escape sequence delay in milliseconds */
#define escDelay 10

/* translate fte colors to curses*/
static const short fte_curses_colors[] =
{
	COLOR_BLACK,
	COLOR_BLUE,
	COLOR_GREEN,
	COLOR_CYAN,
	COLOR_RED,
	COLOR_MAGENTA,
	COLOR_YELLOW,
	COLOR_WHITE,
};


static PCell *SavedScreen = 0;
static int SavedW = 0, SavedH = 0;
static int MaxSavedW = 0, MaxSavedH = 0;

/* Routine to allocate/reallocate and zero space for screen buffer,
   represented as a dynamic array of dynamic PCell lines */
static void SaveScreen() {
	int NewSavedW, NewSavedH;
	ConQuerySize(&NewSavedW, &NewSavedH);
	if (!SavedScreen)
	{
		SavedScreen = (PCell *) malloc(NewSavedH *sizeof(PCell));
		for(int j=0 ; j < NewSavedH; j++)
		{
			SavedScreen[j] = (PCell)malloc(NewSavedW * sizeof(TCell));
			bzero(SavedScreen[j], sizeof(SavedScreen[j]));
		}
		MaxSavedW = SavedW = NewSavedW;
		MaxSavedH = SavedH = NewSavedH;
	}
	else
	{
		if(NewSavedW > MaxSavedW) /* Expand maximum width if needed */
		{
			for(int i=0 ; i < MaxSavedH; i++)
			{
//				assert(sizeof(SavedScreen[i]) == MaxSavedH);
				SavedScreen[i] = (PCell)realloc(SavedScreen[i], NewSavedW * sizeof(TCell));
			}
			MaxSavedW = NewSavedW;
		}
		if(NewSavedW > SavedW) /* Zero newly expanded screen */
		{
			for(int i=0 ; i < MaxSavedH; i++)
			{
				bzero(SavedScreen[i]+SavedW, NewSavedW - SavedW);
			}
		}
		if(NewSavedH > MaxSavedH) /* Expand Maximum height if needed */
		{
			SavedScreen = (PCell *)realloc(SavedScreen, NewSavedH *sizeof(PCell));
			for(int i=MaxSavedH ; i < NewSavedH; i++)
			{
				SavedScreen[i] = (PCell)malloc(MaxSavedW * sizeof(TCell));
			}
			MaxSavedH = NewSavedH;
		}
		if(NewSavedH > SavedH) /* Zero newly expanded screen */
		{
			for(int i=SavedH ; i < NewSavedH; i++)
			{
				 bzero(SavedScreen[i], MaxSavedW);
			}
		}
		SavedW = NewSavedW;
		SavedH = NewSavedH;
	}
}

static void free_savedscreen()
{
	if (! SavedScreen)
		return;
	for (int i = 0; i < MaxSavedH; i++)
	{
		if (SavedScreen[i])
		{
			free(SavedScreen[i]);
			SavedScreen[i] = NULL;
		}
	}
	free(SavedScreen);
	SavedScreen = NULL;
}

static int fte_curses_attr[256];

static int key_sup = 0;
static int key_sdown = 0;

static int conInitColors()
{
	int c = 0;
	int colors = has_colors();

	if(colors) start_color();
	for(int bgb = 0 ; bgb < 2; bgb++) /* bg bright bit */
	{
		for(int bg = 0 ; bg < 8; bg++)
		{
			for(int fgb = 0; fgb < 2; fgb++) /* fg bright bit */
			{
				for(int fg = 0 ; fg < 8; fg++, c++)
				{
					if(colors)
					{
						short pair = short(bg*8+fg);
						if(c!=0) init_pair(pair, fte_curses_colors[fg], fte_curses_colors[bg]);
						fte_curses_attr[c] = short((fgb ? A_BOLD : 0) | COLOR_PAIR(pair));
					}
					else
					{
						fte_curses_attr[c] = 0;
						if(fgb || bgb)
						{
							if(bg > fg) fte_curses_attr[c] |= (A_UNDERLINE | A_REVERSE);
							else fte_curses_attr[c] |= A_BOLD;
						}
						else if(bg > fg) fte_curses_attr[c] |= A_REVERSE;
					}
				}
			}
		}
	}
	return colors;
}

int ConInit(int /*XSize */ , int /*YSize */ )
{
	int ch;
	const char *s;

	ESCDELAY = escDelay;
	initscr();
	conInitColors();
#ifdef CONFIG_MOUSE
	mousemask(ALL_MOUSE_EVENTS|REPORT_MOUSE_POSITION, NULL);
#endif
	/*    cbreak (); */
	raw();
	noecho();
	nonl();
	keypad(stdscr,1);
	meta(stdscr,1);
	SaveScreen();

	/* find shift up/down */
	for(ch = KEY_MAX +1;;ch++)
	{
		s = keyname(ch);
		if(s == NULL) break;

		if(!strcmp(s, "kUP"))
			key_sup = ch;
		else if(!strcmp(s, "kDN"))
			key_sdown = ch;
		if(key_sup > 0 && key_sdown > 0) break;
	}
	return 0;
}


int ConDone(void)
{
	keypad(stdscr,0);
	endwin();
	free_savedscreen();
	return 0;
}

int ConSuspend(void)
{
	return 0;
}
int ConContinue(void)
{
	return 0;
}

int ConSetTitle(char * /*Title */ , char * /*STitle */ )
{
	return 0;
}

int ConGetTitle(char *Title, size_t MaxLen, char *STitle, size_t SMaxLen)
{
	strlcpy(Title, "", MaxLen);
	strlcpy(STitle, "", SMaxLen);
	return 0;
}

int ConClear() /* not used? */
{
	refresh();
	return 0;
}

static chtype GetDch(int idx)
{
	switch(idx)
	{
		case DCH_C1: return ACS_ULCORNER;
		case DCH_C2: return ACS_URCORNER;
		case DCH_C3: return ACS_LLCORNER;
		case DCH_C4: return ACS_LRCORNER;
		case DCH_H: return ACS_HLINE;
		case DCH_V: return ACS_VLINE;
		case DCH_M1: return ACS_HLINE;
		case DCH_M2: return ACS_LTEE;
		case DCH_M3: return ACS_RTEE;
		case DCH_M4 : return 'o'; break;
		case DCH_X: return  'X'; break;
		case DCH_RPTR: return ACS_RARROW; break;
		case DCH_EOL: return ACS_BULLET; break;
		case DCH_EOF: return ACS_DIAMOND; break;
		case DCH_END: return ACS_HLINE; break;
		case DCH_AUP: return ACS_UARROW; break;
		case DCH_ADOWN: return ACS_DARROW; break;
		case DCH_HFORE: return ACS_BLOCK; break;
		case DCH_HBACK: return ACS_CKBOARD; break;
		case DCH_ALEFT: return ACS_LARROW; break;
		case DCH_ARIGHT: return ACS_RARROW; break;
		default: return '@';
	}
}

static int last_attr = A_NORMAL;

int ConPutBox(int X, int Y, int W, int H, PCell Cell)
{
	int CurX, CurY;
	getyx(stdscr, CurY, CurX);
	int yy = Y;

	if (Y + H > LINES)
		H = LINES - Y;

	for(int j =0 ; j < H; j++)
	{
		memcpy(SavedScreen[Y+j]+X, Cell, W*sizeof(TCell)); // copy outputed line to saved screen
		move(yy++,X);
		for ( int i=0; i < W; i++)
		{
			unsigned char ch = Cell->GetChar();
			int attr = fte_curses_attr[Cell->GetAttr()];
			if(attr != last_attr)
			{
				wattrset(stdscr,attr);
				last_attr = attr;
			}
			else attr = 0;

			if(ch < 32)
			{
				if(ch <= 20)
				{
					waddch(stdscr,GetDch(ch));
				}
				else
					waddch(stdscr,'.');
			}
			else if(ch < 128 || ch >= 160)
			{
				waddch(stdscr,ch);
			}
			/*		    else if(ch < 180)
			 {
			 waddch(stdscr,GetDch(ch-128));
			 }
			 */
			else
			{
				waddch(stdscr,'.');
			}
			Cell++;
		}
	}

	move(CurY,CurX);
	refresh();

	return 0;
}

int ConGetBox(int X, int Y, int W, int H, PCell Cell)
{
	for(int j = 0 ; j < H; j++)
	{
		memcpy(Cell, SavedScreen[Y+j]+X, W*sizeof(TCell));
		Cell+=W;
	}

	return 0;

}

int ConPutLine(int X, int Y, int W, int H, PCell Cell)
{
	for (int j=0 ; j < H; j++)
	{
		ConPutBox(X, Y+j, W, 1, Cell);
	}

	return 0;
}

int ConSetBox(int X, int Y, int W, int H, TCell Cell)
{
	PCell line = (PCell) malloc(sizeof(TCell) * W);
	int i;

	for (i = 0; i < W; i++)
		line[i] = Cell;
	ConPutLine(X, Y++, W, H, line);
	free(line);
	return 0;
}



int ConScroll(int Way, int X, int Y, int W, int H, TAttr Fill, int Count)
{
	PCell box;

	box = new TCell [W * H];

	TCell fill(' ', Fill);
	ConGetBox(X, Y, W, H, box);

	if (Way == csUp) {
		ConPutBox(X, Y, W, H - Count, box + W * Count);
		ConSetBox(X, Y + H - Count, W, Count, fill);
	} else {
		ConPutBox(X, Y + Count, W, H - Count, box);
		ConSetBox(X, Y, W, Count, fill);
	}

	delete [] (box);

	return 0;
}

int ConSetSize(int /*X */ , int /*Y */ )
{
	return -1;
}

static void ResizeWindow(int ww, int hh) {
	SaveScreen();
	if (frames)
	{
		frames->Resize(ww, hh);
		frames->Repaint();
	}
}


int ConQuerySize(int *X, int *Y)
{
	*X = COLS;
	*Y = LINES;
	return 0;
}

int ConSetCursorPos(int X, int Y)
{
	move(Y,X);
	refresh();
	return 0;
}

int ConQueryCursorPos(int *X, int *Y)
{
	getyx(stdscr, *Y, *X);
	return 0;
}

static int CurVis = 1;

int ConShowCursor()
{
	CurVis = 1;
	curs_set(1);
	return 0;
}
int ConHideCursor()
{
	CurVis = 0;
	curs_set(0);
	return 0;
}
int ConCursorVisible()
{
	return CurVis;
}

int ConSetCursorSize(int /*Start */ , int /*End */ )
{
	return 0;
}

#ifdef CONFIG_MOUSE
int ConSetMousePos(int /*X */ , int /*Y */ )
{
	return -1;
}
int ConQueryMousePos(int *X, int *Y)
{
	*X = 0;
	*Y = 0;
	return 0;
}

int ConShowMouse()
{
	return -1;
}

int ConHideMouse()
{
	return -1;
}

int ConMouseVisible()
{
	return 0;
}

int ConQueryMouseButtons(int *ButtonCount)
{
	*ButtonCount = 0;
	return 0;
}

static int ConGetMouseEvent(TEvent *Event)
{
	MEVENT mevent;
	if(getmouse(&mevent) == ERR)
	{
		 Event->What = evNone;
		 return -1;
	}
	mmask_t bstate = mevent.bstate;

	Event->What = evNone;
	if(bstate & BUTTON1_PRESSED)
	{
		Event->What = Event->Mouse.What = evMouseDown;
		Event->Mouse.X = mevent.x;
		Event->Mouse.Y = mevent.y;
		Event->Mouse.Buttons = 1;
		Event->Mouse.Count = 1;
	}
	else if(bstate & BUTTON1_RELEASED)
	{
		Event->What = Event->Mouse.What = evMouseUp;
		Event->Mouse.X = mevent.x;
		Event->Mouse.Y = mevent.y;
		Event->Mouse.Buttons = 1;
		Event->Mouse.Count = (bstate & BUTTON1_DOUBLE_CLICKED)? 2: 1;
	}
	else if(bstate & BUTTON1_CLICKED)
	{
		Event->What = Event->Mouse.What = evMouseDown;
		Event->Mouse.X = mevent.x;
		Event->Mouse.Y = mevent.y;
		Event->Mouse.Buttons = 1;
		Event->Mouse.Count = 1;
		mevent.bstate ^= BUTTON1_CLICKED;
		mevent.bstate |= BUTTON1_RELEASED;
		ungetmouse(&mevent);
	}
	else if(bstate & BUTTON1_DOUBLE_CLICKED)
	{
		Event->Mouse.X = mevent.x;
		Event->Mouse.Y = mevent.y;
		Event->Mouse.Buttons = 1;
		Event->Mouse.Count = 2;
		mevent.bstate |= BUTTON1_RELEASED;
		ungetmouse(&mevent);
	}
	return 0;
}
#endif

static TEvent Prev = { evNone };

#if 1
static int ConGetEscEvent(void)
{
	int key;
	char seq[8];
	unsigned seqpos = 1;

	key = getch();
	seq[0] = (char)key;
	if (key == ERR)
		return kbEsc;

	if (key >= 'a' && key <= 'z' ) /* Alt-A == Alt-a*/
		return (kfAlt | (key + 'A' - 'a'));

	while (seqpos < 7 && (key = getch()) != ERR)
		seq[seqpos++] = (char)key;
	seq[seqpos] = 0;
	//fprintf(stderr, "DECODE %d  %s\n", seqpos, seq);

	if (seqpos == 2) {
		key = seq[0];
		if (key < 32)
			return (kfAlt| kfCtrl | (key + 'A' - 1));
	}

	return TTYEscParse(seq);
}

#else
/*
 * Routine would have to be far more complex
 * to handle majority of esq sequences
 *
 * Using string table and TTYEscParse
 */
static int ConGetEscEvent(void)
{
	int ch;
	int code = 0;

	if ((ch = getch()) == 27) {
		ch = getch();
		if (ch == '[' || ch == 'O')
			code |= kfAlt;
	}

	if (ch == ERR)
		return kbEsc;

	if (ch == '[' || ch == 'O') {
		int ch1 = getch();
		int ch2 = 0;
		if (ch1 >= '1' && ch1 <= '8') {
			if ((ch2 = getch()) == ';') {
				if ((ch1 = getch()) == ERR
                                    || ch1 < '1' || ch1 > '8')
                                        return kbEsc;
				ch2 = 0;
			}
		}

		if (ch1 == ERR) /* translate to Alt-[ or Alt-O */
			return  (kfAlt | ch);

		if (ch2 == '~' || ch2 == '$') {
			if (ch2 == '$')
				 code |= kfShift;
			switch (ch1 - '0')
			{
				case 1: code |= kbHome; break;
				case 2: code |= kbIns; break;
				case 3: code |= kbDel; break;
				case 4: code |= kbEnd; break;
				case 5: code |= kbPgUp; break;
				case 6: code |= kbPgDn; break;
				case 7: code |= kbHome; break;
				case 8: code |= kbEnd; break;
				default: code = 0; break;
			}
		} else {
			if (ch2) {
				int ctAlSh = ch2 - '1';
				if (ctAlSh & 0x4) code |= kfCtrl;
				if (ctAlSh & 0x2) code |= kfAlt;
				if (ctAlSh & 0x1) code |= kfShift;
			}

			switch(ch1) {
			case 'A': code |= kbUp; break;
			case 'B': code |= kbDown; break;
			case 'C': code |= kbRight; break;
			case 'D': code |= kbLeft; break;
			case 'F': code |= kbEnd; break;
			case 'H': code |= kbHome; break;
			case 'a': code |= (kfShift | kbUp); break;
			case 'b': code |= (kfShift | kbDown); break;
			case 'c': code |= (kfShift | kbRight); break;
			case 'd': code |= (kfShift | kbLeft); break;
			default:  code = 0; break;
			}
		}
	} else {
		code |= kfAlt;
		if(ch == '\r' || ch == '\n') code |= kbEnter;
		else if(ch == '\t') code |= kbTab;
		else if(ch < 32) code |=  (kfCtrl | (ch+ 0100));
		else {
			if(ch > 0x60 && ch < 0x7b ) /* Alt-A == Alt-a*/
				ch -= 0x20;
			code |= ch;
		}
	}

	return code;
}
#endif

int ConGetEvent(TEventMask /*EventMask */ ,
		TEvent * Event, int WaitTime, int Delete)
{
    int rtn;
    TKeyEvent *KEvent = &(Event->Key);
    if(WaitTime == 0) return -1;

    if (Prev.What != evNone) {
	*Event = Prev;
	if (Delete)
	    Prev.What = evNone;
	return 1;
    }

    if ((rtn=WaitFdPipeEvent(Event, STDIN_FILENO, -1)) <= 0)
	return rtn;

    if (Event->What == evNotify)
	return 0; // pipe reading

    int ch = wgetch(stdscr);
    Event->What = evKeyDown;
    KEvent->Code = 0;

    //fprintf(stderr, "READKEY %d %s\n", ch, keyname(ch));
    if(SevenBit && ch > 127 && ch < 256)
    {
	KEvent->Code |= kfAlt;
	ch -= 128;
	if(ch > 0x60 && ch < 0x7b ) /* Alt-A == Alt-a*/
	    ch-=0x20;
    }

    if (ch < 0) Event->What = evNone;
    else if (ch == 27) {
		keypad(stdscr,0);
		timeout(escDelay);
		if (!(KEvent->Code = ConGetEscEvent()))
			Event->What = evNone;
		timeout(-1);
		keypad(stdscr,1);
    }
    else if(ch == '\r' || ch == '\n') KEvent->Code |= kbEnter;
    else if(ch == '\t') KEvent->Code |= kbTab;
    else if(ch < 32) KEvent->Code |=  (kfCtrl | (ch+ 0100));
    else if(ch < 256) KEvent->Code |= ch;
    else // > 255
    {
	switch(ch)
	{
	case KEY_RESIZE:
	    ResizeWindow(COLS,LINES);
	    Event->What = evNone;
	    break;
#ifdef CONFIG_MOUSE
	case KEY_MOUSE:
	    Event->What = evNone;
	    ConGetMouseEvent(Event);
	    break;
#endif
	case KEY_UP: KEvent->Code = kbUp; break;
	case KEY_SR: KEvent->Code = kfShift | kbUp; break;
	case 559: KEvent->Code = kfAlt | kbUp; break; // kUP3
	case 561: KEvent->Code = kfCtrl | kbUp; break; // kUP5

	case KEY_DOWN: KEvent->Code = kbDown; break;
	case KEY_SF: KEvent->Code = kfShift | kbDown; break;
	case 518: KEvent->Code = kfAlt | kbDown; break; // kDN3
	case 520: KEvent->Code = kfCtrl | kbDown; break; // kDN5

	case KEY_RIGHT: KEvent->Code = kbRight; break;
	case KEY_SRIGHT: KEvent->Code = kfShift | kbRight; break;
	case 553: KEvent->Code = kfAlt | kbRight; break;  // kRIT3
	case 555: KEvent->Code = kfCtrl | kbRight; break; // kRIT5
	case 556: KEvent->Code = kfCtrl | kfShift | kbRight; break; // kRIT6

	case KEY_LEFT: KEvent->Code = kbLeft; break;
	case KEY_SLEFT: KEvent->Code = kfShift | kbLeft; break;
	case 538: KEvent->Code = kfAlt | kbLeft; break; // kLFT3
	case 540: KEvent->Code = kfCtrl | kbLeft; break; // kLFT5
	case 541: KEvent->Code = kfCtrl | kfShift | kbLeft; break; // kLFT6

	case KEY_DC: KEvent->Code = kbDel; break;
	case KEY_SDC: KEvent->Code = kfShift | kbDel; break;

	case KEY_IC: KEvent->Code = kbIns; break;
	case KEY_SIC: KEvent->Code = kfShift | kbIns; break;

	case KEY_BACKSPACE: KEvent->Code = kbBackSp; break;

	case KEY_HOME: KEvent->Code = kbHome; break;
	case KEY_SHOME: KEvent->Code = kfShift | kbHome; break;
	case 528: KEvent->Code = kfCtrl | kbHome; break; // kHOM3
	case 530: KEvent->Code = kfAlt | kbHome; break; // kHOM5
	case 532: KEvent->Code = kfAlt | kfCtrl | kbHome; break; // kHOM7

	case KEY_LL: // used in old termcap/infos
	case KEY_END: KEvent->Code = kbEnd; break;
	case KEY_SEND: KEvent->Code = kfShift | kbEnd; break;
	case 523: KEvent->Code = kfAlt | kbEnd; break; // kEND3
	case 524: KEvent->Code = kfAlt | kfShift | kbEnd; break; // kEND4
	case 525: KEvent->Code = kfCtrl | kbEnd; break; // kEND5
	case 526: KEvent->Code = kfCtrl | kfShift | kbEnd; break; // kEND6
	case 527: KEvent->Code = kfAlt | kfCtrl | kbEnd; break; // kEND7

	case KEY_NPAGE: KEvent->Code = kbPgDn; break;
	case KEY_SNEXT: KEvent->Code = kfShift | kbPgDn; break;
	case 543: KEvent->Code = kfAlt | kbPgDn; break; // kNXT3
	case 545: KEvent->Code = kfCtrl | kbPgDn; break; // kNXT5
	case 547: KEvent->Code = kfAlt | kfCtrl | kbPgDn; break; // kNXT7

	case KEY_PPAGE: KEvent->Code = kbPgUp; break;
	case KEY_SPREVIOUS: KEvent->Code = kfShift | kbPgUp; break;
	case 548: KEvent->Code = kfAlt | kbPgUp; break; // kPRV3
	case 550: KEvent->Code = kfCtrl | kbPgUp; break; // kPRV5
	case 552: KEvent->Code = kfAlt | kfCtrl | kbPgUp; break; // kPRV7

	case KEY_F(1): KEvent->Code = kbF1; break;
	case KEY_F(13): KEvent->Code = kfShift | kbF1; break;
	case KEY_F(25): KEvent->Code = kfCtrl | kbF1; break;
	case KEY_F(37): KEvent->Code = kfCtrl | kfShift | kbF1; break;

	case KEY_F(2): KEvent->Code = kbF2; break;
	case KEY_F(14): KEvent->Code = kfShift | kbF2; break;
	case KEY_F(26): KEvent->Code = kfCtrl | kbF2; break;
	case KEY_F(38): KEvent->Code = kfCtrl | kfShift | kbF2; break;

	case KEY_F(3): KEvent->Code = kbF3; break;
	case KEY_F(15): KEvent->Code = kfShift | kbF3; break;
	case KEY_F(27): KEvent->Code = kfCtrl | kbF3; break;
	case KEY_F(39): KEvent->Code = kfCtrl | kfShift | kbF3; break;

	case KEY_F(4): KEvent->Code = kbF4; break;
	case KEY_F(16): KEvent->Code = kfShift | kbF4; break;
	case KEY_F(28): KEvent->Code = kfCtrl | kbF4; break;
	case KEY_F(40): KEvent->Code = kfCtrl | kfShift | kbF4; break;

	case KEY_F(5): KEvent->Code = kbF5; break;
	case KEY_F(17): KEvent->Code = kfShift | kbF5; break;
	case KEY_F(29): KEvent->Code = kfCtrl | kbF5; break;
	case KEY_F(41): KEvent->Code = kfCtrl | kfShift | kbF5; break;

	case KEY_F(6): KEvent->Code = kbF6; break;
	case KEY_F(18): KEvent->Code = kfShift | kbF6; break;
	case KEY_F(30): KEvent->Code = kfCtrl | kbF6; break;
	case KEY_F(42): KEvent->Code = kfCtrl | kfShift | kbF6; break;

	case KEY_F(7): KEvent->Code = kbF7; break;
	case KEY_F(19): KEvent->Code = kfShift | kbF7; break;
	case KEY_F(31): KEvent->Code = kfCtrl | kbF7; break;
	case KEY_F(43): KEvent->Code = kfCtrl | kfShift | kbF7; break;

	case KEY_F(8): KEvent->Code = kbF8; break;
	case KEY_F(20): KEvent->Code = kfShift | kbF8; break;
	case KEY_F(32): KEvent->Code = kfCtrl | kbF8; break;
	case KEY_F(44): KEvent->Code = kfCtrl | kfShift | kbF8; break;

	case KEY_F(9): KEvent->Code = kbF9; break;
	case KEY_F(21): KEvent->Code = kfShift | kbF9; break;
	case KEY_F(33): KEvent->Code = kfCtrl | kbF9; break;
	case KEY_F(45): KEvent->Code = kfCtrl | kfShift | kbF9; break;

	case KEY_F(10): KEvent->Code = kbF10; break;
	case KEY_F(22): KEvent->Code = kfShift | kbF10; break;
	case KEY_F(34): KEvent->Code = kfCtrl | kbF10; break;
	case KEY_F(46): KEvent->Code = kfCtrl | kfShift | kbF10; break;

	case KEY_F(11): KEvent->Code = kbF11; break;
	case KEY_F(23): KEvent->Code = kfShift | kbF11; break;
	case KEY_F(35): KEvent->Code = kfCtrl | kbF11; break;
	case KEY_F(47): KEvent->Code = kfCtrl | kfShift | kbF11; break;

	case KEY_F(12): KEvent->Code = kbF12; break;
	case KEY_F(24): KEvent->Code = kfShift | kbF12; break;
	case KEY_F(36): KEvent->Code = kfCtrl | kbF12; break;
	case KEY_F(48): KEvent->Code = kfCtrl | kfShift | kbF12; break;

	case KEY_B2: KEvent->Code = kbCenter; break;
	case KEY_ENTER: KEvent->Code = kbEnter; break; /* shift enter */

	default:
	    if(key_sdown != 0 && ch == key_sdown)
		KEvent->Code = kfShift | kbDown;
	    else if(key_sup != 0 && ch == key_sup)
		KEvent->Code = kfShift | kbUp;
	    else
	    {
		Event->What = evNone;
		//	fprintf(stderr, "Unknown 0x%x %d\n", ch, ch);
	    }
	    break;
	}
    }

    if (!Delete)
	Prev = *Event;

    return 1;
}

char ConGetDrawChar(unsigned int idx)
{
	//    return 128+idx;
	return (char)idx;
}

int ConPutEvent(TEvent Event)
{
	Prev = Event;
	return 0;
}

GUI::GUI(int &argc, char **argv, int XSize, int YSize)
{
	fArgc = argc;
	fArgv = argv;
	::ConInit(-1, -1);
	::ConSetSize(XSize, YSize);
	TTYEscInit();
	gui = this;
}

GUI::~GUI()
{
	::ConDone();
	gui = 0;
}

int GUI::ConSuspend(void)
{
	return::ConSuspend();
}

int GUI::ConContinue(void)
{
	return::ConContinue();
}

int GUI::ShowEntryScreen()
{
	return 1;
}

int GUI::RunProgram(int /*mode */ , char *Command)
{
	int rc, W, H, W1, H1;

	ConQuerySize(&W, &H);
#ifdef CONFIG_MOUSE
	ConHideMouse();
#endif
	ConSuspend();

	if (*Command == 0)		// empty string = shell
		Command = getenv("SHELL");

	rc = system(Command);

	ConContinue();
#ifdef CONFIG_MOUSE
	ConShowMouse();
#endif
	ConQuerySize(&W1, &H1);

	if (W != W1 || H != H1) {
		frames->Resize(W1, H1);
	}
	frames->Repaint();
	return rc;
}
