/*    i_ascii.h
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef I_ASCII_H
#define I_ASCII_H

#include "console.h"
#include "i_oview.h"

class ExASCII: public ExView {
public:
    int Pos, LPos;

    ExASCII();
    virtual ~ExASCII();
    virtual void Activate(int gotfocus);

    virtual ExView* GetViewContext();
    virtual int BeginMacro();
    virtual void HandleEvent(TEvent &Event);
    virtual void UpdateView();
    virtual void RepaintView();
    virtual void UpdateStatus();
    virtual void RepaintStatus();
};

#endif // I_ASCII_H
