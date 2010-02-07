
#include "gui.h"

int GFrame::isLastFrame() {
    if (this == Next && frames == this)
        return 1;
    else
        return 0;
}

void GUI::deleteFrame(GFrame *frame) {
    if (frame->isLastFrame()) {
        delete frame;
        frames = 0;
    } else {
        //frame->Prev->Next = frame->Next;
        //frame->Next->Prev = frame->Prev;
        //if (frames == frame)
        //    frames = frame->Next;

        //frames->Activate();
        delete frame;
    }
}

int GUI::Start(int &/*argc*/, char ** /*argv*/) {
    return 0;
}

void GUI::Stop() {
}

void GUI::StopLoop() {
    doLoop = 0;
}
