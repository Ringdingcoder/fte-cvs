#ifndef E_MARK_H
#define E_MARK_H

#include "e_buffer.h"

#include <stdio.h> // FILE

class EMark {
public:
    EMark(const char *aName, const char *aFileName, EPoint aPoint, EBuffer *aBuffer = 0);
    ~EMark();

    int setBuffer(EBuffer *aBuffer);
    int removeBuffer(EBuffer *aBuffer);

    char *getName() { return Name; }
    char *getFileName() { return FileName; }
    EPoint &getPoint();
    EBuffer *getBuffer() { return Buffer; }
private:
    /* bookmark */
    char *Name;
    EPoint Point;
    char *FileName;

    /* bookmark in file */
    EBuffer *Buffer;
};

class EMarkIndex {
public:
    EMarkIndex();
    ~EMarkIndex();

    EMark *insert(const char *aName, const char *aFileName, EPoint aPoint, EBuffer* aBuffer = 0);
    EMark *insert(const char *aName, EBuffer* aBuffer, EPoint aPoint);
    EMark *locate(const char *aName);
    int remove(const char *aName);
    int view(EView *aView, const char *aName);

//    int MarkPush(EBuffer *B, EPoint P);
//    int MarkPop(EView *V);
//    int MarkSwap(EView *V, EBuffer *B, EPoint P);
//    int MarkNext(EView *V);
    //    int MarkPrev(EView *V);
    EMark *pushMark(EBuffer *aBuffer, EPoint P);
    int popMark(EView *aView);

    int retrieveForBuffer(EBuffer *aBuffer);
    int storeForBuffer(EBuffer *aBuffer);

    int saveToDesktop(FILE *fp);

private:
    int markCount;
    EMark **marks;
};

extern EMarkIndex markIndex;

#endif // E_MARK_H
