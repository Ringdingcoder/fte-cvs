/*    i_key.h
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef I_KEY_H
#define I_KEY_H

#include "i_oview.h"

class ExKey: public ExView {
public:
    char *Prompt;
    TKeyCode Key;
    char ch;
    
    ExKey(const char *APrompt);
    virtual ~ExKey();
    virtual void Activate(int gotfocus);
    
    virtual ExView* GetViewContext() { return Next; }
    virtual int BeginMacro();
    virtual void HandleEvent(TEvent &Event);
    virtual void UpdateView();
    virtual void RepaintView();
    virtual void UpdateStatus();
    virtual void RepaintStatus();
};

#endif // I_KEY_H
