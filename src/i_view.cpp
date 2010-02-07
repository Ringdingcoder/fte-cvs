/*    i_view.cpp
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#include "i_view.h"

#include "c_config.h"
#include "i_ascii.h"
#include "i_choice.h"
#include "i_complete.h"
#include "i_key.h"
#include "o_buflist.h"
#include "s_files.h"

//#include <stdio.h>

GxView::GxView(GFrame *Parent) :
    GView(Parent, -1, -1),
    Top(0),
    Bottom(0),
    MouseCaptured(0)
{
}

GxView::~GxView() {
    //fprintf(stderr, "DELETE GXVIEW %p   %p\n", Top, this);
    while (Top) {
	ExView *V = Top;
	Top = Top->Next;
	V->Win = 0;
	delete V;
    }
}

void GxView::PushView(ExView *view) {
    int W, H;
    ConQuerySize(&W, &H);

    //fprintf(stderr, "PUSHVIEW %p   %p\n", view, this);
    view->Win = this;
    if (Top == 0) {
        Top = Bottom = view;
        view->Next = 0;
    } else {
        Top->Activate(0);
        view->Next = Top;
        Top = view;
        Top->Activate(1);
    }
    Top->Resize(W, H);
}

ExView *GxView::PopView() {

    //fprintf(stderr, "POPVIEW %p   %p\n", Top, this);
    if (Top == 0)
        return 0;

    Top->Activate(0);

    ExView *V = Top;
    Top = Top->Next;

    if (Top == 0)
        Bottom = 0;
    else {
        Top->Activate(1);
        Top->Repaint();
    }
    V->Win = 0;

    return V;
}

void GxView::NewView(ExView * /*view*/) {
}

EEventMap *GxView::GetEventMap() {
    return (Top) ? Top->GetEventMap() : 0;
}

int GxView::ExecCommand(ExCommands Command, ExState &State) {
    return (Top) ? Top->ExecCommand(Command, State) : 0;
}

int GxView::BeginMacro() {
    return (Top) ?  Top->BeginMacro() : 1;
}

int GxView::GetContext() {
    return (Top) ? Top->GetContext() : CONTEXT_NONE;
}

void GxView::HandleEvent(TEvent &Event) {
    GView::HandleEvent(Event);
    Top->HandleEvent(Event);

#ifdef CONFIG_MOUSE
    if (Event.What & evMouse) {
        int W, H;

        ConQuerySize(&W, &H);

        if (Event.What != evMouseDown || Event.Mouse.Y == H - 1) {
            switch (Event.What) {
            case evMouseDown:
		if (CaptureMouse(1)) {
		    MouseCaptured = 1;
		    Event.What = evNone;
		}
                break;
            case evMouseMove:
                if (MouseCaptured) {
                    if (Event.Mouse.Y != H - 1)
                        ExpandHeight(Event.Mouse.Y - H + 1);
                    Event.What = evNone;
                }
                break;
            case evMouseAuto:
                if (MouseCaptured)
                    Event.What = evNone;
                break;
            case evMouseUp:
		if (MouseCaptured) {
                    CaptureMouse(0);
		    MouseCaptured = 0;
		    Event.What = evNone;
		}
                break;
            }
        }
    }
#endif
}

void GxView::Update() {
    if (Top)
        Top->Update();
}

void GxView::Repaint() {
    if (Top) {
        Top->Repaint();
    } else {
        TDrawBuffer B;
        int X, Y;

        ConQuerySize(&X, &Y);
        MoveCh(B, ' ', 0x07, X);
        ConPutLine(0, 0, X, Y, B);
    }
}

void GxView::Resize(int width, int height) {

    GView::Resize(width, height);

    for (ExView *V = Top; V; V = V->Next)
        V->Resize(width, height);
}

void GxView::Activate(int gotfocus) {
    if (Top)
        Top->Activate(gotfocus);
    GView::Activate(gotfocus);
}

void GxView::UpdateTitle(const char *Title, const char *STitle) {
    if (Parent && Parent->Active == this)
        Parent->ConSetTitle(Title, STitle);
}

int GxView::GetStr(const char *Prompt, size_t BufLen, char *Str, int HistId) {
    if ((HaveGUIDialogs & GUIDLG_PROMPT) && GUIDialogs)
        return DLGGetStr(this, Prompt, BufLen, Str, HistId, 0);

    return ReadStr(Prompt, BufLen, Str, 0, 1, HistId);
}

int GxView::GetFile(const char *Prompt, size_t BufLen, char *Str, int HistId, int Flags) {
    if ((HaveGUIDialogs & GUIDLG_FILE) && GUIDialogs)
        return DLGGetFile(this, Prompt, BufLen, Str, Flags);
    else
        return ReadStr(Prompt, BufLen, Str, CompletePath, SelectPathname, HistId);
}

int GxView::ReadStr(const char *Prompt, size_t BufLen, char *Str, Completer Comp, int Select, int HistId) {
    int rc;
    ExInput input(Prompt, Str, BufLen, Comp, Select, HistId);

    PushView(&input);

    rc = Execute();

    PopView();

    Repaint();

    if (rc == 1) {
        strncpy(Str, input.Line, BufLen - 1);
        Str[BufLen - 1] = 0;
    }

    return rc;
}

int GxView::Choice(unsigned long Flags, const char *Title, int NSel, ... /* choices, format, args */) {
    int rc;
    va_list ap;

    if ((HaveGUIDialogs & GUIDLG_CHOICE) && GUIDialogs) {
        va_start(ap, NSel);
        rc = DLGPickChoice(this, Title, NSel, ap, Flags);
        va_end(ap);
        return rc;
    } else {
        ExChoice *choice;

        va_start(ap, NSel);

        choice = new ExChoice(Title, NSel, ap);

        va_end(ap);

        if (choice == 0)
            return 0;

        PushView(choice);
        rc = Execute();
        PopView();
        Repaint();

        delete choice;

        return rc;
    }
}

TKeyCode GxView::GetChar(const char *Prompt) {
    int rc;
    ExKey *key;
    TKeyCode K = 0;

    key = new ExKey(Prompt);
    if (key == 0)
        return 0;

    PushView(key);
    rc = Execute();
    PopView();
    Repaint();

    if (rc == 1)
        K = key->Key;
    delete key;

    return K;
}

#ifdef CONFIG_I_SEARCH
int GxView::IncrementalSearch(EView *View) {
    int rc;
    ExISearch *search;
    EBuffer *B = 0;

    if (View->GetContext() != CONTEXT_FILE)
        return 0;

    B = (EBuffer *)View->Model;

    search = new ExISearch(B);

    if (search == 0)
        return 0;

    PushView(search);
    rc = Execute();
    PopView();
    Repaint();

    delete search;

    return rc;
}
#endif

#ifdef CONFIG_I_ASCII
int GxView::PickASCII() {
    int rc;
    ExASCII *ascii;

    ascii = new ExASCII();
    if (ascii == 0)
        return 0;

    PushView(ascii);
    rc = Execute();
    PopView();
    Repaint();

    delete ascii;
    return rc;
}
#endif

#ifdef CONFIG_I_COMPLETE
int GxView::ICompleteWord(EView *View) {
    int rc = 0;

    if (View->GetContext() == CONTEXT_FILE) {
        ExComplete *c = new ExComplete((EBuffer *)View->Model);
        if (c != NULL) {

            if (c->IsSimpleCase())
            {
                rc = c->DoCompleteWord();
            } else
            {
                PushView(c);
                rc = Execute();
                PopView();
            }

            Repaint();

            delete c;
        }
    }
    return rc;
}
#endif
