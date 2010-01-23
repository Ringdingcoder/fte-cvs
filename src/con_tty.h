
/*
 * Handling escape sequencies is quite complex
 * easiest seems to be the use of sequence table
 * instead of creating complex if/else code,
 * though there are some obviously visible bitmasks
 */

#include "conkbd.h"
#include <stdlib.h>
#include <stdio.h> // fprintf

static const struct TTYDecodeSeq {
    char seq[8];
    int key;
} tty_seq_table_c[] = {
    /* '%' replaced with 2..8  with modified  Alt | Ctrl | Shift */
    { "[1;%A", kbUp },
    { "[%A", kbUp },
    { "[A", kbUp },
    { "[a", kfShift | kbUp },

    { "[1;%B", kbDown },
    { "[%B", kbDown },
    { "[B", kbDown },
    { "[b", kfShift | kbDown },

    { "[1;%C", kbRight },
    { "[%C", kbRight },
    { "[C", kbRight },
    { "[c", kfShift | kbRight },

    { "[1;%D", kbLeft },
    { "[%D", kbLeft },
    { "[D", kbLeft },
    { "[d", kfShift | kbLeft },

    { "[1;%F", kbEnd },
    { "[%F", kbEnd },
    { "[F", kbEnd },

    { "[1;%H", kbHome },
    { "[%H", kbHome },
    { "[H", kbHome },

    { "[1;%P", kbF1 },
    { "[1;%Q", kbF2 },
    { "[1;%R", kbF3 },
    { "[1;%S", kbF4 },

    { "[15;%~", kbF5 },
    { "[15~", kbF5 },

    { "[17;%~", kbF6 },
    { "[17~", kbF6 },

    { "[18;%~", kbF7 },
    { "[18~", kbF7 },

    { "[19;%~", kbF8 },
    { "[19~", kbF8 },

    { "[20;%~", kbF9 },
    { "[20~", kbF9 },

    { "[21;%~", kbF10 },
    { "[21~", kbF10 },

    { "[23;%~", kbF11 },
    { "[23~", kbF11 },

    { "[24;%~", kbF12 },
    { "[24~", kbF12 },

    { "[1;%~", kbHome },
    { "[1~", kbHome },

    { "[2;%~", kbIns },
    { "[2~", kbIns },

    { "[3;%~", kbDel },
    { "[3~", kbDel },

    { "[4;%~", kbEnd },
    { "[4~", kbEnd },

    { "[5;%~", kbPgUp },
    { "[5~", kbPgUp },

    { "[6;%~", kbPgDn },
    { "[6~", kbPgDn },

    { "[25~", kfShift | kbF1 },
    { "[26~", kfShift | kbF2 },
    { "[28~", kfShift | kbF3 },
    { "[29~", kfShift | kbF4 },
    { "[31~", kfShift | kbF5 },
    { "[32~", kfShift | kbF6 },
    { "[33~", kfShift | kbF7 },
    { "[34~", kfShift | kbF8 },

    { "[[A", kbF1 },
    { "[[B", kbF2 },
    { "[[C", kbF3 },
    { "[[D", kbF4 },
    { "[[E", kbF5 },

    { "[P", kbPrtScr },
    { "[Z", kfShift | kbTab },

    { "O%A", kbUp },
    { "O%B", kbDown },
    { "O%C", kbRight },
    { "O%D", kbLeft },

    { "OF", kbEnd },
    { "OH", kbHome },

    { "O1;%P", kbF1 },
    { "O%P", kbF1 },
    { "OP", kbF1 },

    { "O1;%Q", kbF2 },
    { "O%Q", kbF2 },
    { "OQ", kbF2 },

    { "O1;%R", kbF3 },
    { "O%R", kbF3 },
    { "OR", kbF3 },

    { "O1;%S", kbF4 },
    { "O%S", kbF4 },
    { "OS", kbF4 },
};

/* Sorted via qsort in runtime so there is NO const here! */
static struct TTYDecodeSeq tty_esc_seq[7 * sizeof(tty_seq_table_c) / sizeof(tty_seq_table_c[0])];
static unsigned tty_seq_size = 0;

static int TTYCompSeq(const void *a, const void *b)
{
    int c = strcmp(((const TTYDecodeSeq*)a)->seq,
		   ((const TTYDecodeSeq*)b)->seq);
    if (c == 0) {
        tty_seq_size = 0;
	fprintf(stderr, "ERROR: tty_seq_table_c contains duplicate Escape sequence \"%s\"!\n",
		((const TTYDecodeSeq*)a)->seq);
    }

    return c;
}

static int TTYParseEsc(const char *seq)
{
    if (seq[1] == 0) {
	char ch = seq[0];
	if (ch < 32) {
	    switch (ch) {
	    case 0: return kbEsc;
	    case '\t': return (kfShift | kbTab);
	    case '\n': return (kfAlt | kbEnter);
	    case 8:    return (kfAlt | kbBackSp);
	    case 27:   return (kfAlt | kbEsc);
	    default:   return (kfAlt | kfCtrl | (ch + 'A' - 1));
	    }
	} else if (ch == 0x7f)
	    return kfAlt | kbBackSp;
	else if (ch >= 'a' && ch <= 'z')
	    return (kfAlt | (ch + 'A' - 'a'));
	else if (ch >= 'A' && ch <= 'Z')
	    return (kfAlt | kfShift | ch);
	else if (strchr("`1234567890-=[];'\\,./", ch))
	    return (kfAlt | ch);
	else if (strchr("~!@#$%%^&*()_+{}:\"|<>?", ch))
	    return (kfAlt | kfShift | ch);
    }

    // standard routine for binary search
    unsigned i, H = 0, L = 0;
    unsigned R = tty_seq_size;

    while (L < R) {
        int c;
	H = L + (R - L) / 2;

	//if ((c = strcmp(seq, tty_esc_seq[H].seq)) == 0) {
	// replace strcmp with direct char compare
	for (i = 0; (tty_esc_seq[H].seq[i]
		     && !(c = (seq[i] - tty_esc_seq[H].seq[i]))); ++i)
	    ;
	if (c == 0 && tty_esc_seq[H].seq[i] == 0) {
	    //fprintf(stderr, "Found key: 0x%x  %s  %s\n",
	    //        tty_esc_seq[H].key, tty_esc_seqast [H].seq, seq);
	    return tty_esc_seq[H].key;
	} else if (c < 0)
	    R = H;
	else // (c > 0)
	    L = H + 1;
    }

    // for detecting unknown Esq sequences - sfte 2>/tmp/newesc
    //fprintf(stderr, "FIXME: Unknown Esc sequence: \"%s\"\n", seq);
    for (int i = 0; seq[i]; ++i) fprintf(stderr, "FIXME: Unknown Esc sequence: \"%d  %x  %c\"\n", seq[i], seq[i], isprint(seq[i]) ? seq[i] : ' ');
    return kbEsc;
}

/*
 * Create table from template and replace '%' with character numbers
 * from '2' to '8' with appropriate Alt, Ctrl, Shift modifiers
 */
static int TTYInitTable(void)
{
    for (unsigned i = 0; i < sizeof(tty_seq_table_c)/sizeof(tty_seq_table_c[0]); ++i) {
	for (unsigned j = '2'; j <= '8'; ++j) {
	    static const int kf[] = {
		kfShift, // '2'
		kfAlt,
		kfAlt | kfShift,
		kfCtrl,
		kfCtrl | kfShift,
		kfCtrl | kfAlt,
		kfAlt | kfCtrl | kfShift // '8'
	    };
	    tty_esc_seq[tty_seq_size].key = tty_seq_table_c[i].key;
	    strcpy(tty_esc_seq[tty_seq_size].seq, tty_seq_table_c[i].seq);
	    char *r = strchr(tty_esc_seq[tty_seq_size].seq, '%');
	    if (!r) {
		tty_seq_size++;
		break;
	    }
	    *r = (char)j;
	    tty_esc_seq[tty_seq_size++].key |= kf[j - '2'];
	}
    }

    qsort(tty_esc_seq, tty_seq_size, sizeof(TTYDecodeSeq), TTYCompSeq);

    if (!tty_seq_size)
        return 1;

    return 0;

#if 0
    const char * const test[] = { "[1;2A", "OF", "ODS", "[1;8R", 0 };
    for (unsigned i = 0; test[i]; i++)
        fprintf(stderr, "search %s  %d\n", test[i], TTYParseEsc(test[i]));


    for (unsigned i = 0; i < tty_seq_size; ++i)
	fprintf(stderr, "%d %s %x\n", i, tty_esc_seq[i].seq, tty_esc_seq[i].key);

    return 1;
#endif
}
