
/*
 * Handling escape sequencies is quite complex
 * easiest seems to be the use of sequence table
 * instead of creating complex if/else code,
 * though there are some obviously visible bitmasks
 */

#include "conkbd.h"
#include <stdlib.h>
#include <stdio.h> // fprintf

/*
 * Sorted via qsort in runtime so there is NO const here!
 */
static struct TTYEscDecode {
    char seq[8];
    int key;
} tty_esc_seq[] = {
    { "[1;2A", kfShift | kbUp },
    { "[1;3A", kfAlt | kbUp },
    { "[1;5A", kfCtrl | kbUp },
    { "[1;6A", kfCtrl | kfShift | kbUp },

    { "[2A", kfShift | kbUp },
    { "[5A", kfCtrl | kbUp },
    { "[6A", kfCtrl | kfShift | kbUp },
    { "[A", kbUp },

    { "[1;2B", kfShift | kbDown },
    { "[1;3B", kfAlt | kbDown },
    { "[1;5B", kfCtrl | kbDown },
    { "[1;6B", kfCtrl | kfShift | kbDown },

    { "[2B", kfShift | kbDown },
    { "[5B", kfCtrl | kbDown },
    { "[6B", kfCtrl | kfShift | kbDown },
    { "[B", kbDown },

    { "[1;2C", kfShift | kbRight },
    { "[1;3C", kfAlt | kbRight },
    { "[1;5C", kfCtrl | kbRight },
    { "[1;6C", kfCtrl | kfShift | kbRight },

    { "[2C", kfShift | kbRight },
    { "[5C", kfCtrl | kbRight },
    { "[6C", kfCtrl | kfShift | kbRight },
    { "[C", kbRight },

    { "[1;2D", kfShift | kbLeft },
    { "[1;3D", kfAlt | kbLeft },
    { "[1;5D", kfCtrl | kbLeft },
    { "[1;6D", kfCtrl | kfShift | kbLeft },

    { "[2D", kfShift | kbLeft },
    { "[5D", kfCtrl | kbLeft },
    { "[6D", kfCtrl | kfShift | kbLeft },
    { "[D", kbLeft },

    { "[1;2F", kfShift | kbEnd },
    { "[1;3F", kfAlt | kbEnd },
    { "[1;5F", kfCtrl | kbEnd },
    { "[1;6F", kfCtrl | kfShift | kbEnd },

    { "[2F", kfShift | kbEnd },
    { "[5F", kfCtrl | kbEnd },
    { "[6F", kfCtrl | kfShift | kbEnd },
    { "[F", kbEnd },

    { "[1;2H", kfShift | kbHome },
    { "[1;3H", kfAlt | kbHome },
    { "[1;5H", kfCtrl | kbHome },
    { "[1;6H", kfCtrl | kfShift | kbHome },

    { "[2H", kfShift | kbHome },
    { "[5H", kfCtrl | kbHome },
    { "[6H", kfCtrl | kfShift | kbHome },
    { "[H", kbHome },

    { "[1;2P", kfShift | kbF1 },
    { "[1;3P", kfAlt | kbF1 },
    { "[1;5P", kfCtrl | kbF1 },
    { "[1;6P", kfCtrl | kfShift | kbF1 },

    { "[1;2Q", kfShift | kbF2 },
    { "[1;3Q", kfAlt | kbF2 },
    { "[1;5Q", kfCtrl | kbF2 },
    { "[1;6Q", kfCtrl | kfShift | kbF2 },

    { "[1;2R", kfShift | kbF3 },
    { "[1;3R", kfAlt | kbF3 },
    { "[1;5R", kfCtrl | kbF3 },
    { "[1;6R", kfCtrl | kfShift | kbF3 },

    { "[1;2S", kfShift | kbF4 },
    { "[1;3S", kfAlt | kbF4 },
    { "[1;5S", kfCtrl | kbF4 },
    { "[1;6S", kfCtrl | kfShift | kbF4 },

    { "[15;2~", kfShift | kbF5 },
    { "[15;3~", kfAlt | kbF5 },
    { "[15;5~", kfCtrl | kbF5 },
    { "[15;6~", kfCtrl | kfShift | kbF5 },
    { "[15~", kbF5 },

    { "[17;2~", kfShift | kbF6 },
    { "[17;3~", kfAlt | kbF6 },
    { "[17;5~", kfCtrl | kbF6 },
    { "[17;6~", kfCtrl | kfShift | kbF6 },
    { "[17~", kbF6 },

    { "[18;2~", kfShift | kbF7 },
    { "[18;3~", kfAlt | kbF7 },
    { "[18;5~", kfCtrl | kbF7 },
    { "[18;6~", kfCtrl | kfShift | kbF7 },
    { "[18~", kbF7 },

    { "[19;2~", kfShift | kbF8 },
    { "[19;3~", kfAlt | kbF8 },
    { "[19;5~", kfCtrl | kbF8 },
    { "[19;6~", kfCtrl | kfShift | kbF8 },
    { "[19~", kbF8 },

    { "[20;2~", kfShift | kbF9 },
    { "[20;3~", kfAlt | kbF9 },
    { "[20;5~", kfCtrl | kbF9 },
    { "[20;6~", kfCtrl | kfShift | kbF9 },
    { "[20~", kbF9 },

    { "[21;2~", kfShift | kbF10 },
    { "[21;3~", kfAlt | kbF10 },
    { "[21;5~", kfCtrl | kbF10 },
    { "[21;6~", kfCtrl | kfShift | kbF10 },
    { "[21~", kbF10 },

    { "[23;2~", kfShift | kbF11 },
    { "[23;3~", kfAlt | kbF11 },
    { "[23;5~", kfCtrl | kbF11 },
    { "[23;6~", kfCtrl | kfShift | kbF11 },
    { "[23~", kbF11 },

    { "[24;2~", kfShift | kbF12 },
    { "[24;3~", kfAlt | kbF12 },
    { "[24;5~", kfCtrl | kbF12 },
    { "[24;6~", kfCtrl | kfShift | kbF12 },
    { "[24~", kbF12 },

    { "[1;2~", kfShift | kbHome },
    { "[1;3~", kfAlt | kbHome },
    { "[1;5~", kfCtrl | kbHome },
    { "[1;6~", kfCtrl | kfShift | kbHome },
    { "[1~", kbHome },

    { "[2;2~", kfShift | kbIns },
    { "[2;3~", kfAlt | kbIns },
    { "[2;5~", kfCtrl | kbIns },
    { "[2;6~", kfCtrl | kfShift | kbIns },
    { "[2~", kbIns },

    { "[3;2~", kfShift | kbDel },
    { "[3;3~", kfAlt | kbDel },
    { "[3;5~", kfCtrl | kbDel },
    { "[3;6~", kfCtrl | kfShift | kbDel },
    { "[3~", kbDel },

    { "[4;2~", kfShift | kbEnd },
    { "[4;3~", kfAlt | kbEnd },
    { "[4;5~", kfCtrl | kbEnd },
    { "[4;6~", kfCtrl | kfShift | kbEnd },
    { "[4~", kbEnd },

    { "[5;2~", kfShift | kbPgUp },
    { "[5;3~", kfAlt | kbPgUp },
    { "[5;5~", kfCtrl | kbPgUp },
    { "[5;6~", kfCtrl | kfShift | kbPgUp },
    { "[5~", kbPgUp },

    { "[6;2~", kfShift | kbPgDn },
    { "[6;3~", kfAlt | kbPgDn },
    { "[6;5~", kfCtrl | kbPgDn },
    { "[6;6~", kfCtrl | kfShift | kbPgDn },
    { "[6~", kbPgDn },

    { "[[A", kbF1 },
    { "[[B", kbF2 },
    { "[[C", kbF3 },
    { "[[D", kbF4 },
    { "[[E", kbF5 },

    { "[P", kbPrtScr },
    { "[Z", kfShift | kbTab },

    { "O1;2P", kfShift | kbF1 },
    { "O1;3P", kfAlt | kbF1 },
    { "O1;5P", kfCtrl | kbF1 },
    { "O1;6P", kfCtrl | kfShift | kbF1 },
    { "O2P", kfShift | kbF1 },
    { "O3P", kfAlt | kbF1 },
    { "O5P", kfCtrl | kbF1 },
    { "O6P", kfCtrl | kfShift | kbF1 },
    { "OP", kbF1 },

    { "O1;2Q", kfShift | kbF2 },
    { "O1;3Q", kfAlt | kbF2 },
    { "O1;5Q", kfCtrl | kbF2 },
    { "O1;6Q", kfCtrl | kfShift | kbF2 },
    { "O2Q", kfShift | kbF2 },
    { "O3Q", kfAlt | kbF2 },
    { "O5Q", kfCtrl | kbF2 },
    { "O6Q", kfCtrl | kfShift | kbF2 },
    { "OQ", kbF2 },

    { "O1;2R", kfShift | kbF3 },
    { "O1;3R", kfAlt | kbF3 },
    { "O1;5R", kfCtrl | kbF3 },
    { "O1;6R", kfCtrl | kfShift | kbF3 },
    { "O2R", kfShift | kbF3 },
    { "O3R", kfAlt | kbF3 },
    { "O5R", kfCtrl | kbF3 },
    { "O6R", kfCtrl | kfShift | kbF3 },
    { "OR", kbF3 },

    { "O1;2S", kfShift | kbF4 },
    { "O1;3S", kfAlt | kbF4 },
    { "O1;5S", kfCtrl | kbF4 },
    { "O1;6S", kfCtrl | kfShift | kbF4 },
    { "O2S", kfShift | kbF4 },
    { "O3S", kfAlt | kbF4 },
    { "O5S", kfCtrl | kbF4 },
    { "O6S", kfCtrl | kfShift | kbF4 },
    { "OS", kbF4 },

    { "[25~", kfShift | kbF1 },
    { "[26~", kfShift | kbF2 },
    { "[28~", kfShift | kbF3 },
    { "[29~", kfShift | kbF4 },
    { "[31~", kfShift | kbF5 },
    { "[32~", kfShift | kbF6 },
    { "[33~", kfShift | kbF7 },
    { "[34~", kfShift | kbF8 },

    { "O5A", kfCtrl | kbUp },
    { "O6A", kfCtrl | kfShift | kbUp },
    { "O5B", kfCtrl | kbDown },
    { "O6B", kfCtrl | kfShift | kbDown },
    { "O5C", kfCtrl | kbRight },
    { "O6C", kfCtrl | kfShift | kbRight },
    { "O5D", kfCtrl | kbLeft },
    { "O6D", kfCtrl | kfShift | kbLeft },

    { "OF", kbEnd },
    { "OH", kbHome },

    { "\x7f", kfAlt | kbBackSp },
    { "\r", kfAlt | kbEnter },
    { "\n", kfAlt | kbEnter },
    { "\t", kfAlt | kbTab },
    { "\x1b", kbEsc },
    { ",", kfAlt | ',' },
    { ".", kfAlt | '.' },
    { "/", kfAlt | '/' },
    { ";", kfAlt | ';' },
    { "'", kfAlt | '\'' },
    { "\\", kfAlt | '\\' },
    { "]", kfAlt | ']' },
    { "-", kfAlt | '-' },
    { "=", kfAlt | '=' },
    { "{", kfShift | kfAlt | '{' },
    { "}", kfShift | kfAlt | '}' },

    { "[", kfAlt | '[' },
};

static int TTYEscComp(const void *a, const void *b)
{
    int c = strcmp(((const TTYEscDecode*)a)->seq,
		   ((const TTYEscDecode*)b)->seq);
    if (c)
	return c;

    fprintf(stderr, "ERROR: seqtable has duplicated Esc sequence \"%s\"!\n",
	    ((const TTYEscDecode*)a)->seq);
    exit(-1);
}

static int TTYEscParse(const char *seq)
{
    unsigned H = 0, L = 0;
    unsigned R = sizeof(tty_esc_seq) / sizeof(tty_esc_seq[0]);
    int c;

    while (L < R) {
	H = L + (R - L) / 2;
	if ((c = strcmp(seq, tty_esc_seq[H].seq)) == 0) {
	    //fprintf(stderr, "Found key:      0x%x  %s  %s\n",
	    //	seqtable[H].key, seqtable[H].seq, seq);
	    return tty_esc_seq[H].key;
	} else if (c < 0)
	    R = H;
	else // (c > 0)
	    L = H + 1;
    }

    // for detecting unknown Esq sequences - sfte 2>/tmp/newesc
    fprintf(stderr, "FIXME: Unknown Esc sequence: \"%s\"\n", seq);
    return kbEsc;
}

static void TTYEscInit(void)
{
    qsort(tty_esc_seq, sizeof(tty_esc_seq)/sizeof(tty_esc_seq[0]),
	  sizeof(TTYEscDecode), TTYEscComp);

    //for (unsigned i = 0; i < sizeof(seqtable)/sizeof(seqtable[0]); i++)
    //    fprintf(stderr, "%d %s %x\n", i, seqtable[i].seq, seqtable[i].key);
}
