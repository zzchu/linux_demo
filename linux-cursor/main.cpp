#include <memory>
#include <stdio.h>
#include <unistd.h>
#include <X11/extensions/Xfixes.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "X11CursorShapeGrabber.h"


using namespace XTCursorShapeGrabber;

#define MINIZ_NO_STDIO
#define MINIZ_NO_ARCHIVE_APIS
#define MINIZ_NO_TIME
#define MINIZ_NO_ZLIB_APIS
#include "miniz.c"

typedef unsigned char uint8;

typedef struct
{
    unsigned char r, g, b, a;
} rgb_t;


int main(void) {
    unsigned char *pImage = NULL;
    unsigned int memSize = 0;
    X11CursorShapeGrabber *cursorShapeGrabber = new X11CursorShapeGrabber();
    
    while (1) {
        if (cursorShapeGrabber->isCursorShapeChanged()) {
            cursorShapeGrabber->grab();
            
            printf("width=%d, height=%d, xHot=%d, yHot=%d \n",
                   cursorShapeGrabber->getWidth(),
                   cursorShapeGrabber->getHeight(),
                   cursorShapeGrabber->getXHotSpot(),
                   cursorShapeGrabber->getYHotSpot());
            
            int iXmax = cursorShapeGrabber->getWidth();
            int iYmax = cursorShapeGrabber->getHeight();
            const unsigned long *pPixels = (const unsigned long *)cursorShapeGrabber->getBuffer();
            
            if ((memSize < (iXmax*iYmax))){
                memSize = iXmax * iYmax;
                pImage = (uint8 *)malloc(iXmax * 4 * iYmax);
            }
            
            for(int iY = 0; iY < iYmax; iY++)
            {
                for(int iX = 0; iX < iXmax; iX++)
                {
                    rgb_t *color = (rgb_t *)(pImage + (iX * 4) + (iY * iXmax * 4));
                    
                    color->a = (unsigned char)((pPixels[iY*iXmax + iX] >> 24) & 0xff);
                    color->r = (unsigned char)((pPixels[iY*iXmax + iX] >> 16) & 0xff);
                    color->g = (unsigned char)((pPixels[iY*iXmax + iX] >>  8) & 0xff);
                    color->b = (unsigned char)((pPixels[iY*iXmax + iX] >>  0) & 0xff);
                }
            }
            
            // Now write the PNG image.
            {
                char* pFilename = "cursor_shape";
                static int num = 0;
                num ++;
                if (num > 10) {
                    break;
                }
                char filefullname[256] = {0};
                sprintf(filefullname, "%s-%d.png", pFilename, num);
                
                
                size_t png_data_size = 0;
                void *pPNG_data = tdefl_write_image_to_png_file_in_memory_ex(pImage, iXmax, iYmax, 4, &png_data_size, 6, MZ_FALSE);
                if (!pPNG_data)
                    fprintf(stderr, "tdefl_write_image_to_png_file_in_memory_ex() failed!\n");
                else
                {
                    FILE *pFile = fopen(filefullname, "wb");
                    fwrite(pPNG_data, 1, png_data_size, pFile);
                    fclose(pFile);
                    printf("Wrote %s\n", filefullname);
                }
                mz_free(pPNG_data);
//                printf("png file is generated!\n");
                
            }
            
        }
        usleep(50);
    }
    free(pImage);
    delete cursorShapeGrabber;
}
