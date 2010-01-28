/*    o_messages.h
 *
 *    Copyright (c) 1994-1996, Marko Macek
 *
 *    You may distribute under the terms of either the GNU General Public
 *    License or the Artistic License, as specified in the README file.
 *
 */

#ifndef O_MESSAGES_H
#define O_MESSAGES_H

#ifdef CONFIG_OBJ_MESSAGES

#include "stl_fte.h"

struct Error {
    fte::string file;
    int line;
    fte::string msg;
    fte::string text;
    int hilit;
    EBuffer *Buf;

    Error(const char *_file, int _line, const char *_msg, const char *_text, int _hilit) :
	file(_file), line(_line), msg(_msg), text(_text), hilit(_hilit), Buf(0)
    {}
};

struct aDir
{
    fte::string name;
    aDir* next;

    aDir(const char *n, aDir* l) :
	name(n), next(l)
    {}
};

class EMessages: public EList {
    fte::string Command;
    fte::string Directory;
    fte::vector<Error*> ErrList;

public:

    int Running;

    int BufLen;
    int BufPos;
    int PipeId;
    int ReturnCode;
    int MatchCount;
    char MsgBuf[4096];
    aDir*   curr_dir;                       // top of dir stack.
    
    EMessages(int createFlags, EModel **ARoot, char *Dir, char *ACommand);
    ~EMessages();
    void freeDirStack();

    virtual void NotifyDelete(EModel *Deleting);
    void FindErrorFiles();
    void FindErrorFile(unsigned err);
    void AddFileError(EBuffer *B, unsigned err);
    void FindFileErrors(EBuffer *B);
    
    virtual int GetContext() { return CONTEXT_MESSAGES; }
    virtual EEventMap *GetEventMap();
    virtual int ExecCommand(ExCommands Command, ExState &State);

    void AddError(Error *p);
    void AddError(const char *file, int line, const char *msg, const char *text, int hilit=0);
    
    void FreeErrors();
    int GetLine(char *Line, size_t MaxLen);
    void GetErrors();
    int Compile(char *Command);
    void ShowError(EView *V, unsigned err);
    void DrawLine(PCell B, int Line, int Col, ChColor color, int Width);
    char* FormatLine(int Line);
    int IsHilited(int Line);
    void UpdateList();
    int Activate(int No);
    int CanActivate(int Line);
    void NotifyPipe(int APipeId);
    const char* GetDirectory() { return Directory.c_str(); }
    virtual void GetName(char *AName, size_t MaxLen);
    virtual void GetInfo(char *AInfo, size_t MaxLen);
    virtual void GetPath(char *APath, size_t MaxLen);
    virtual void GetTitle(char *ATitle, size_t MaxLen, char *ASTitle, size_t SMaxLen);
    virtual size_t GetRowLength(int ARow);


    int RunPipe(char *Dir, char *Command);
    
    int CompilePrevError(EView *V);
    int CompileNextError(EView *V);
};

extern EMessages *CompilerMsgs;

void FreeCRegexp();

#endif // CONFIG_OBJ_MESSAGES

#endif // O_MESSAGES_H
