//http://www1.tc711.com/tool/BASE64.htm

#include <stdio.h>
#include <unistd.h>
#include "base64.h"
#include "XTClipBoardLinux.h"
using namespace XTClipBoard;
using namespace std;


void readImageFile(const char* Imgname, string& output)
{
    FILE* imgP = fopen(Imgname, "r");
    if (imgP == NULL){
        return ;
    }
    
    fseek(imgP, 0L, SEEK_END);
    unsigned int  size = ftell(imgP);
    unsigned char* imgbuf = new unsigned char[size+1];
    
    fseek(imgP,0x0L,SEEK_SET);
    size_t count = fread(imgbuf, sizeof(imgbuf[0]), size, imgP);
    
    if (count != size) {
        printf("the actual count is:%ld, the target size is:%d\n", count, size);
    }
    
    fclose(imgP);
    
    output = base64_encode(imgbuf, size);

}

int main (int argc, char* argv[])
{
    sleep(2);
    //test the character copy&paste
    ClipBoardLinux* m_xtClipboard = new ClipBoardLinux();

    string  characterString;
    const char *pChar = "大家好，我想我应该要开始了！！";
    characterString = base64_encode((unsigned char*)pChar, strlen(pChar));
    m_xtClipboard->ReceiveClientClipboardData("TEXT", characterString);
    m_xtClipboard->SetMutliClipboardData();
    m_xtClipboard->SendCtrlV();
    
    sleep(2);
    
    string  fileString;
    //readImageFile("htmlinput", fileString);
    //m_xtClipboard->ReceiveClientClipboardData("HTML", fileString);
    
    readImageFile("rtfbig.rtf", fileString);
    m_xtClipboard->ReceiveClientClipboardData("RTF", fileString);

    m_xtClipboard->SetMutliClipboardData();
    m_xtClipboard->SendCtrlV();
    printf("end process!!!!\n");
    while(1)
    {
        sleep(1);
    };
    //sleep(5);

}
