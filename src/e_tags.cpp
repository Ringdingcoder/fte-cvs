/*    e_tags.cpp *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "e_tags.h"

#ifdef CONFIG_TAGS

#include "o_buflist.h"
#include "s_files.h"
#include "s_util.h"
#include "sysdep.h"

#include <fcntl.h>

struct TagData {
    int Tag;        // tag name pos
    int FileName;
    int TagBase;    // name of tag file
    int Line;
    int StrFind;    // string to find
};

struct TagStack {
    static TagStack* TStack;

    StlString CurrentTag;
    StlString FileName;
    int Line, Col;
    int TagPos;

    TagStack(const char* tag, const char* file, int line, int col, int pos) :
        CurrentTag(tag), FileName(file), Line(line), Col(col), TagPos(pos), Next(TStack)
    {
        TStack = this;
    }
    ~TagStack()
    {
        TStack = Next;
    }

private:
    TagStack *Next;
};

TagStack* TagStack::TStack = 0;

static char *TagMem = 0;
static int TagLen = 0;
static int ATagMem  = 0;

static int CTags = 0;            // number of tags
static int ATags = 0;
static TagData *TagD = 0;
static int *TagI = 0;
static int TagFileCount = 0;
static int *TagFiles = 0;

static int TagFilesLoaded = 0;   // tag files are loaded at first lookup
static char *CurrentTag;
static int TagPosition = -1;

static int AllocMem(const char *Mem, size_t Len) { /*FOLD00*/
    int N = 1024;
    char *NM;
    int TagPos = TagLen;

    while (N < TagLen + Len) N <<= 1;
    if (ATagMem < N || TagMem == 0) {
        NM = (char *)realloc((void *)TagMem, N);
        if (NM == 0)
            return -1;
        TagMem = NM;
        ATagMem = N;
    }
    memcpy(TagMem + TagLen, Mem, Len);
    TagLen += Len;
    return TagPos;
}

static int AddTag(int Tag, int FileName, int TagBase, int Line, int StrFind) { /*FOLD00*/
    int N;

    N = 1024;
    while (N < CTags + 1) N <<= 1;
    if (ATags < N || TagD == 0) {
        TagData *ND;

        ND = (TagData *)realloc((void *)TagD, N * sizeof(TagData));
        if (ND == 0)
            return -1;
        TagD = ND;
        ATags = N;
    }
    TagD[CTags].Tag = Tag;
    TagD[CTags].Line = Line;
    TagD[CTags].FileName = FileName;
    TagD[CTags].TagBase = TagBase;
    TagD[CTags].StrFind = StrFind;
    CTags++;
    return 0;
}

#if defined(__IBMCPP__)
int _LNK_CONV cmptags(const void *p1, const void *p2) {
#else
int cmptags(const void *p1, const void *p2) {
#endif
    //return strcmp(TagMem + TagD[*(int *)p1].Tag,
    //              TagMem + TagD[*(int *)p2].Tag);
    int r = strcmp(TagMem + TagD[*(int *)p1].Tag,
		   TagMem + TagD[*(int *)p2].Tag);
    if (r != 0)
	return r;

    r = strcmp(TagMem + TagD[*(int *)p1].FileName,
	       TagMem + TagD[*(int *)p2].FileName);
    if (r != 0)
	return r;

    if (TagD[*(int *)p1].Line != TagD[*(int *)p2].Line)
	r = TagD[*(int *)p1].Line < TagD[*(int *)p2].Line ? -1 : 1;

    return r;
}

int SortTags() { /*FOLD00*/
    int *NI;
    int i;

    if (CTags == 0)
        return 0;

    NI = (int *)realloc(TagI, CTags * sizeof(int));
    if (NI == 0)
        return -1;
    TagI = NI;
    for (i = 0; i < CTags; i++)
        TagI[i] = i;

    qsort(TagI, CTags, sizeof(TagI[0]), cmptags);

    return 0;
}

int TagsLoad(int id) { /*FOLD00*/
    //char line[2048];
    char *tags;
    int fd;
    struct stat sb;
    long size;

    free(TagI);
    TagI = 0;

    if ((fd = open(TagMem + TagFiles[id], O_BINARY | O_RDONLY)) == -1)
        return -1;

    if (fstat(fd, &sb) == -1)
        return -1;

    if ((tags = (char *)malloc((size_t)sb.st_size)) == 0) {
        close(fd);
        return -1;
    }

    size = read(fd, tags, (size_t)sb.st_size);
    close(fd);
    if (size != sb.st_size)
        return -1;

    if (TagMem == 0) { // preallocate (useful when big file)
        char *NM;

        NM = (char *)realloc((void *)TagMem, TagLen + (size_t)sb.st_size);
        if (NM != 0) {
            TagMem = NM;
            ATagMem = TagLen + (int)sb.st_size;
        }
    }

    char *p = tags;
    char *e = tags + sb.st_size;

    char *LTag, *LFile, *LLine;
    int TagL, FileL/*, LineL*/;
    int MTag, MFile;

    while (p < e) {
        LTag = p;
        while (p < e && *p != '\t') p++;
        if (p < e && *p == '\t') *p++ = 0;
        else break;
        TagL = p - LTag;
        LFile = p;
        while (p < e && *p != '\t') p++;
        if (p < e && *p == '\t') *p++ = 0;
        else break;
        FileL = p - LFile;
        LLine = p;
        while (p < e && *p != '\r' && *p != '\n') p++;
        if (p < e && *p == '\r') *p++ = 0;           // optional
        if (p < e && *p == '\n') *p++ = 0;
        else break;
        //LineL = p - LLine;

        MTag = AllocMem(LTag, TagL + FileL);

        if (MTag == -1)
            break;

        MFile = MTag + TagL;

        if (LLine[0] == '/') {
            char *AStr = LLine;
            char *p = AStr + 1;
            char *d = AStr;
            int MStr;

            while (*p) {
                if (*p == '\\') {
                    p++;
                    if (*p)
                        *d++ = *p++;
                } else if (*p == '^' || *p == '$') p++;
                else if (*p == '/')
                    break;
                else
                    *d++ = *p++;
            }
            *d = 0;
            if (stricmp(d - 10, "/*FOLD00*/") == 0)
                d[-11] = 0; /* remove our internal folds */

            MStr = AllocMem(AStr, strlen(AStr) + 1);
            if (MStr == -1)
                break;

            if (AddTag(MTag, MFile, TagFiles[id], -1, MStr) == -1)
                break;
        } else {
            if (AddTag(MTag, MFile, TagFiles[id], atoi(LLine), -1) == -1)
                break;
        }
    }
    free(tags);
    return 0;
}

int TagsAdd(const char *FileName) { /*FOLD00*/
    int *NewT;
    int NewF;

    NewF = AllocMem(FileName, strlen(FileName) + 1);
    if (NewF == -1)
        return 0;

    NewT = (int *)realloc((void *)TagFiles, (TagFileCount + 1) * sizeof(int));
    if (NewT == 0)
        return 0;
    TagFiles = NewT;
    TagFiles[TagFileCount++] = NewF;
    return 1;
}

int TagsSave(FILE *fp) { /*FOLD00*/
    for (int i = 0; i < TagFileCount; i++)
        if (fprintf(fp, "T|%s\n", TagMem + TagFiles[i]) < 0)
            return ErFAIL;

    return ErOK;
}

static void ClearTagStack() { /*FOLD00*/
    free(CurrentTag);
    CurrentTag = 0;
    TagPosition = -1;
    while (TagStack::TStack)
        delete TagStack::TStack;
}

int TagLoad(const char *FileName) { /*FOLD00*/
    if (TagsAdd(FileName) == 0)
        return 0;
    ClearTagStack();
    if (TagFilesLoaded) {
        if (TagsLoad(TagFileCount - 1) == -1) {
            return 0;
        }
        if (SortTags() == -1) {
            TagClear();
            return 0;
        }
    }
    return 1;
}

static int LoadTagFiles() { /*FOLD00*/
    int i;

    assert(TagFilesLoaded == 0);
    for (i = 0; i < TagFileCount; i++)
        if (TagsLoad(i) == -1) {
            TagClear();
            return 0;
        }
    if (SortTags() == -1) {
        TagClear();
        return 0;
    }
    TagFilesLoaded = 1;
    return 1;
}

void TagClear() { /*FOLD00*/
    free(TagD);
    free(TagI);
    TagD = 0;
    TagI = 0;
    CTags = 0;
    ATags = 0;

    free(TagFiles);
    TagFiles = 0;
    TagFileCount = 0;
    TagFilesLoaded = 0;

    free(TagMem);
    TagMem = 0;
    TagLen = 0;
    ATagMem = 0;

    ClearTagStack();
}

static int GotoFilePos(EView *View, const char *FileName, int Line, int Col) { /*FOLD00*/
    if (FileLoad(0, FileName, 0, View) == 0)
        return 0;
    if (((EBuffer *)ActiveModel)->Loaded == 0)
        ((EBuffer *)ActiveModel)->Load();
    ((EBuffer *)ActiveModel)->CenterNearPosR(Col, Line);
    return 1;
}

static int GotoTag(int M, EView *View) { /*FOLD00*/
    char path[MAXPATH];
    char Dir[MAXPATH];
    TagData *TT = &TagD[TagI[M]];

    JustDirectory(TagMem + TT->TagBase, Dir, sizeof(Dir));

    if (IsFullPath(TagMem + TT->FileName)) {
        strcpy(path, TagMem + TT->FileName);
    } else {
        strcpy(path, Dir);
        Slash(path, 1);
        strcat(path, TagMem + TT->FileName);
    }
    if (TT->Line != -1) {
        if (GotoFilePos(View, path, TT->Line - 1, 0) == 0)
            return 0;
    } else {
        if (GotoFilePos(View, path, 0, 0) == 0)
            return 0;
        if (((EBuffer *)ActiveModel)->FindStr(TagMem + TT->StrFind, strlen(TagMem + TT->StrFind), 0) == 0)
            return 0;
    }
    ((EBuffer *)ActiveModel)->FindStr(TagMem + TT->Tag, strlen(TagMem + TT->Tag), 0);
    return 1;
}

static int PushPos(EBuffer *B) /*FOLD00*/
{
    (void) new TagStack(CurrentTag, B->FileName,
			B->VToR(B->CP.Row), B->CP.Col,
			TagPosition);
    return 1;
}

int TagGoto(EView *View, const char *Tag) {
    assert(Tag != 0);

    if (TagFilesLoaded == 0)
        if (LoadTagFiles() == 0)
            return 0;

    int L = 0, R = CTags, M, cmp;

    if (CTags == 0)
        return 0;

    while (L < R) {
        M = (L + R) / 2;
        cmp = strcmp(Tag, TagMem + TagD[TagI[M]].Tag);
        if (cmp == 0) {
            while (M > 0 && strcmp(Tag, TagMem + TagD[TagI[M - 1]].Tag) == 0)
                M--;

            if (GotoTag(M, View) == 0)
                return 0;

            free(CurrentTag);
            CurrentTag = strdup(Tag);
            TagPosition = M;
            return 1;
        } else if (cmp < 0) {
            R = M;
        } else {
            L = M + 1;
        }
    }
    return 0; // tag not found
}

int TagFind(EBuffer *B, EView *View, const char *Tag) { /*FOLD00*/
    assert(View != 0 && Tag != 0 && B != 0);

    if (TagFilesLoaded == 0)
        if (LoadTagFiles() == 0)
            return 0;

    int L = 0, R = CTags, M, cmp;
    //printf("TAGFIND %s  %s   tp:%d\n", Tag,  CurrentTag, TagPosition);
    if (CurrentTag) {
        if (strcmp(CurrentTag, Tag) == 0) {
            if (TagPosition != TagStack::TStack->TagPos)
                if (PushPos(B) == 0)
                    return 0;

            TagPosition = TagStack::TStack->TagPos;

            return TagNext(View);
        }
    }

    if (CTags == 0)
        return -1;

    while (L < R) {
        M = (L + R) / 2;
        cmp = strcmp(Tag, TagMem + TagD[TagI[M]].Tag);
        if (cmp == 0) {
            while (M > 0 && strcmp(Tag, TagMem + TagD[TagI[M - 1]].Tag) == 0)
                M--;

            if (PushPos(B) == 0)
                return 0;

            if (GotoTag(M, View) == 0)
                return 0;

            free(CurrentTag); // in case it already exists.
            CurrentTag = strdup(Tag);
            TagPosition = M;

            return 1;
        } else if (cmp < 0) {
            R = M;
        } else {
            L = M + 1;
        }
    }
    return 0; // tag not found
}

int TagDefined(const char *Tag) {
    int L = 0, R = CTags, M, cmp;

    if (TagFilesLoaded == 0) // !!! always?
        if (LoadTagFiles() == 0)
            return 0;

    if (CTags == 0)
        return 0;
    
    while (L < R) {
        M = (L + R) / 2;
        cmp = strcmp(Tag, TagMem + TagD[TagI[M]].Tag);
        if (cmp == 0)
            return 1;
        else if (cmp < 0)
            R = M;
        else
            L = M + 1;
    }
    return 0; // tag not found
}

int TagComplete(char **Words, int *WordsPos, int WordsMax, const char *Tag) {
    if ((Tag == NULL) || (Words == NULL) || (*WordsPos >= WordsMax))
        return 0;

    if (TagFilesLoaded == 0)
        if (LoadTagFiles() == 0)
            return 0;

    if (CTags == 0)
        return 0;

    int L = 0, R = CTags, len = strlen(Tag);

    while (L < R) {
        int c, M;

        M = (L + R) / 2;
        c = strncmp(Tag, TagMem + TagD[TagI[M]].Tag, len);
        if (c == 0) {
            while (M > 0 &&
                   strncmp(Tag, TagMem + TagD[TagI[M - 1]].Tag, len) == 0)
                M--;            // find begining
            int N = M, w = 0;
            while(strncmp(Tag, TagMem + TagD[TagI[N]].Tag, len) == 0) {
                // the first word is not tested for previous match
                if (!w || strcmp(TagMem + TagD[TagI[N]].Tag,
                                 TagMem + TagD[TagI[N-1]].Tag)) {
                    int l = strlen(TagMem + TagD[TagI[N]].Tag) - len;
                    if (l > 0) {
                        char *s = new char[l + 1];
                        if (s != NULL) {
                            strcpy(s, TagMem + TagD[TagI[N]].Tag + len);
                            Words[(*WordsPos)++] = s;
                            w++; // also mark the first usage
                            if (*WordsPos >= WordsMax)
                                break;
                        } else
                            break;  // how about using exceptions
                    }
                }
                N++;
            }
            return w;
        } else if (c < 0) {
            R = M;
        } else {
            L = M + 1;
        }
    }
    return 0; // tag not found
}

int TagNext(EView *View) { /*FOLD00*/
    assert(View != 0);

    if (CurrentTag == 0 || TagPosition == -1)
        return 0;

    if (TagPosition < CTags - 1 && strcmp(CurrentTag, TagMem + TagD[TagI[TagPosition + 1]].Tag) == 0) {
        TagPosition++;
        if (GotoTag(TagPosition, View) == 0)
            return 0;
        return 1;
    }
    View->Msg(S_INFO, "No next match for tag.");
    return 0;
}

int TagPrev(EView *View) { /*FOLD00*/
    assert(View != 0);

    if (CurrentTag == 0 || TagPosition == -1) {
        View->Msg(S_INFO, "No current tag.");
        return 0;
    }

    if (TagPosition > 0 && strcmp(CurrentTag, TagMem + TagD[TagI[TagPosition - 1]].Tag) == 0) {
        TagPosition--;
        if (GotoTag(TagPosition, View) == 0)
            return 0;
        return 1;
    }
    View->Msg(S_INFO, "No previous match for tag.");
    return 0;
}

int TagPop(EView *View) { /*FOLD00*/

    assert(View != 0);

    delete TagStack::TStack;

    TagStack* T = TagStack::TStack;
    if (T) {
        free(CurrentTag);
        CurrentTag = strdup(T->CurrentTag.c_str());
        TagPosition = T->TagPos;
        return GotoFilePos(View, T->FileName.c_str(), T->Line, T->Col);
    }
    View->Msg(S_INFO, "Tag stack empty.");
    return 0;
}

#endif
