/*    i_input.h
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef __EXINPUT_H
#define __EXINPUT_H

#include "i_oview.h"
#include "console.h"

typedef int (*Completer)(const char *Name, char *Completed, int Num);

class ExInput: public ExView {
public:
    char *Prompt;
    char *Line;
    char *MatchStr;
    char *CurStr;
    unsigned int Pos;
    unsigned int LPos;
    size_t MaxLen;
    Completer Comp;
    int TabCount;
    int HistId;
    int CurItem;
    unsigned int SelStart;
    unsigned int SelEnd;
    
    ExInput(const char *APrompt, char *ALine, size_t AMaxLen, Completer AComp, int Select, int AHistId);
    virtual ~ExInput();
    virtual void Activate(int gotfocus);
    
    virtual ExView *GetViewContext() { return Next; }
    virtual int BeginMacro();
    virtual void HandleEvent(TEvent &Event);
    virtual void UpdateView();
    virtual void RepaintView();
    virtual void UpdateStatus();
    virtual void RepaintStatus();
};

#endif
