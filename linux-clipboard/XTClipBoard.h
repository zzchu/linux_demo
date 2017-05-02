#pragma once
#include <string> 
using namespace std;
namespace XTClipBoard
{
    typedef enum tagClipType
    {
        kClipText = 1,
        kClipImage = 2
    } ClipType;

	class ClipBoard
	{
	public:
		ClipBoard(void) {};
		virtual ~ClipBoard(void){}; 
		virtual bool SetClipboardData(unsigned int clipFormat,string strData_base64) = 0;
		virtual string JPG2DIB(string jpg_base64) = 0;
		virtual string TextTranscoding(string text_base64) = 0;
		virtual void SendCtrlV() = 0;
       virtual unsigned int GetJFIFid() = 0;
       virtual unsigned int GetPNGid() = 0;
       virtual unsigned int GetGIFid() = 0;
       virtual unsigned int GetTextid() = 0;
       virtual unsigned int GetDibid() = 0;
	};

}
