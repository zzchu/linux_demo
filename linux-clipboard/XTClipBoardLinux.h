#pragma once
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xmu/Atoms.h>
#include <mutex>
#include <vector>
#include "XTClipBoard.h"
using namespace std;

typedef std::vector<std::pair<Atom,std::vector<unsigned char>> > XTClipBoardInfoVec;

namespace XTClipBoard
{
    typedef enum tagClipFormat
    {
        kClipFormatNone = 0,
        kClipFormatText = 1,
        kClipFormatJfif = 2,
        kClipFormatPng = 3,
        kClipFormatGif = 4,
        kClipFormatDib = 5,
        kClipFormatHtml = 6,
        kClipFormatRtf = 7,
        kClipFormatBmp = 8
    } ClipFormat;
    
    typedef enum tagThreadState {
        THREAD_RUNNING =1,
        THREAD_STOPPING = 2,
        THREAD_STOPPED = 3
    } ThreadState;
    
    class ClipBoardLinux : public ClipBoard {
    public:
        ClipBoardLinux(void);
        ~ClipBoardLinux(void);
        bool SetClipboardData(unsigned int clipFormat,std::string strData_base64);
        std::string JPG2DIB(std::string jpg_base64);
        std::string TextTranscoding(std::string text_base64);
        void SendCtrlV();
        unsigned int GetJFIFid();
        unsigned int GetPNGid();
        unsigned int GetGIFid();
        unsigned int GetTextid();
        unsigned int GetDibid();
        void ReceiveClientClipboardData(const string& clipType, const string& clipData);
        unsigned int SetMutliClipboardData();
    private:
        unsigned ListenSelectionRequest();
#if 1
        int  xcin(Display * dpy,
                  Window * win,
                  XEvent evt,
                  Atom * pty,
                  unsigned long *pos,
                  unsigned int *context);
#else
        int  xcin(Display * dpy,
                  Window * win,
                  XEvent evt,
                  Atom * pty, Atom target, unsigned char *txt, unsigned long len, unsigned long *pos,
                  unsigned int *context);
#endif
    private:
        unsigned char *m_selectionBuf;	/* buffer for selection data */
        unsigned long m_selectionLen;	/* length of sel_buf */
        ClipFormat m_clipFormat;	/* format of content */
        
        
        XTClipBoardInfoVec m_clipboardItemVect;
        //XTClipBoardInfoVec m_clipboardItemVectBuf[2];
        //unsigned int m_vecBufIndex;
        
        unsigned int m_threadState;
        mutex  m_threadStateMutex;
        Window m_win;
    };
    
}
