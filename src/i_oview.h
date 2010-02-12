/*    i_oview.h
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef I_OVIEW_H
#define I_OVIEW_H

#include "console.h"
#include "c_commands.h"

class GxView;
class EBuffer;
class EWindow;
class EEventMap;
class ExState;

class ExView {
protected:
public:
    GxView *Win;
    ExView *Next;

    ExView();
    virtual ~ExView();

    virtual EEventMap *GetEventMap();
    virtual int ExecCommand(ExCommands Command, ExState &State);

    virtual void Activate(int gotfocus);
    virtual int GetContext();
    virtual ExView *GetViewContext();
    virtual ExView *GetStatusContext();
    virtual int BeginMacro();
    virtual void HandleEvent(TEvent &Event);
    virtual void UpdateView();
    virtual void UpdateStatus();
    virtual void RepaintView();
    virtual void RepaintStatus();
    virtual void Resize(int width, int height);
    virtual void EndExec(int NewResult);
    int IsActive();

    void Repaint();
    void Update();

    int ConPutBox(int X, int Y, int W, int H, PCell Cell);
    int ConScroll(int Way, int X, int Y, int W, int H, TAttr Fill, int Count);
    int ConQuerySize(int *X, int *Y);
    int ConSetCursorPos(int X, int Y);
    int ConShowCursor();
    int ConHideCursor();
    int ConSetCursorSize(int Start, int End);

    virtual int IsModelView();
};

#endif // I_OVIEW_H
