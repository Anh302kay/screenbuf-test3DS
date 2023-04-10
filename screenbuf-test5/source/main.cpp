#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>
#include <iostream>
#include <string>

#include <chrono>

#include "text.hpp"

#define topscrADR ((void *)0x3005DC00)
#define BUF_SIZE (400 * 240*2)
#define PI 3.14159265358979323846

struct Colour
{
    u8 r = 0, g = 0, b = 0;
};

void plotPixel(u8* buf, int x, int y, Colour colour)
{
    if( x < 0 || x >= 400 || y < 0 || y >= 240)
        return;
    
    //const int pos = (((x + 1) * 240) - y - 1) * 3;
    const int pos = ((x + 1) * 240 - y - 1);

    if(pos > 400*240)
        return;

    u16* buf16 = (u16*)buf;

    u16 c = RGB8_to_565(colour.r, colour.g, colour.b);

    buf16[pos] = c;

    

}

void clearBuf(u8* buf, Colour colour)
{
    u16* buf16 = (u16*)buf;
    for(int i = 0; i < 400*240;i++)
        buf16[i] = RGB8_to_565(colour.r, colour.g, colour.b);
}

void renderRect(u8* buf, int x, int y, unsigned int w, unsigned int h, Colour colour)
{
    //this func is faster than the other one
    if (x >= 400 || y >= 240 || (signed int)w + x <= 0 || (signed int)h + y <= 0)
        return;

    if (y + h > 240)
        h = 240 - y;

    if ( y < 0)
    {
        h += y;
        y = 0;
    }

    if (x < 0)
    {
        w += x;
        x = 0;
    }

    u16 line[h] = {0};
    u16 linecolour = RGB8_to_565(colour.r, colour.g, colour.b);

    for(unsigned int i = 0; i < h; i++)
    {
        //if()
        line[i] = linecolour;
    }

    for(unsigned int i = 0; i < w; i++)
    {
        if(x + i > 400)
            return;
        memcpy(&buf[((x + 1 + i) * 240 - y - h) * 2], &line, h*2);
    }

}

void renderRect2(u8 *buf, int x, int y, unsigned int w, unsigned int h, Colour colour)
{
    if (x < 0 || x >= 400 || y < 0 || y >= 240)
        return;

    
    if( y + h > 240)
        h = 240 - y - 1;
    
    for(unsigned int yy = 0; yy < h; yy++)
    {
        for(unsigned int xx = 0; xx < w; xx++)
        {
            
            plotPixel(buf, x + xx, y + yy, colour);
        }
    }
}

void rotateImage(const u16* src, u16* dst, int width, int height, double angle)
{
    //func no work
    int x, y, new_x, new_y;
    double radians = angle * PI / 180.0;
    double cosine = cos(radians);
    double sine = sin(radians);
    int cx = width / 2;   // center x
    int cy = height / 2;  // center y
    
    for (x = 0; x < width; ++x)
    {
        for (y = 0; y < height; ++y)
        {
            // Calculate new coordinates after rotation
            new_x = (int)((x - cx) * cosine - (y - cy) * sine + cx);
            new_y = (int)((x - cx) * sine + (y - cy) * cosine + cy);

            // Nearest neighbor interpolation (find nearest pixel in source image)
            if (new_x >= 0 && new_x < width && new_y >= 0 && new_y < height)
            {
                dst[new_y + new_x * height] = src[y + x * height];
            }
        }
    }
}

void drawImg(u16* buf, u16* img, int x, int y, int width, int height)
{
    if (x >= 400 || y >= 240 || (signed int)width + x <= 0 || (signed int)height + y <= 0)
        return;

    int xOffset = 0;
    int yOffset = 0;

    if (y + height > 240)
        height = 240 - y;

    if ( y < 0)
    {
        height += y;
        y = 0;
    }

    if (x < 0)
    {
        xOffset = abs(x);
    }
    for(int i = height*xOffset; i < width*height; i++)
    {
        const int index = (x + 1 + xOffset) * 240 - y - height + yOffset;
        if(index > BUF_SIZE)
            return;
        buf[index] = img[i];
        if((i+1) % (height) == 0)
        xOffset++;    

        yOffset++;
        if(yOffset == height)
        yOffset = 0;

        
    }
}

void drawImg2(u16* buf, u16* img, int x, int y, int width, int height)
{
    // this func faster but other one should make it easier to implement transparency
    if (x >= 400 || y >= 240 || (signed int)width + x <= 0 || (signed int)height + y <= 0)
        return;

    int xOffset = 0;
    int yOffset = 0;
    int yOffset2 = 0; // this one is for if it goes below the screen.

    if (y + height > 240)
    {
        yOffset2 = y+height-240;
        //height = 240 - y;
        std::cout << yOffset2 << "\n";
        
    }

    if ( y < 0)
    {
        //height += y;
        //y = 0;
        yOffset = y;
    }

    if (x < 0)
    {   
        xOffset = abs(x);
        // width += x;
        // x = 0;
    }
   for(int i = xOffset; i < width; i++)
   {
        if(x + i > 400)
            return;
        memcpy(&buf[((x + 1 + i) * 240 - y - height + yOffset2)], &img[i*height+yOffset2], (height + yOffset -yOffset2)*2);
   }
}

void renderChar(const char text, u8* buf, int x, int y, Colour colour)
{

    char *letter = bitmap[(size_t)text];

    for (int yy = 0; yy < 8; yy++)
    {
        for (int xx = 0; xx < 8; xx++)
        {
            if (letter[yy] & (1 << xx))
            {
                plotPixel(buf, x + xx, y + yy, colour);
            }
        }
    }
}

void renderText(const char* text, u8 *buf, int x, int y, Colour colour)
{
    int len = strnlen(text, 4096);
    for(int i = 0; i < len; i++)
    {
        char *letter = bitmap[(size_t)text[i]];
        for (int yy = 0; yy < 8; yy++)
        {
            for (int xx = 0; xx < 8; xx++)
            {
                if (letter[yy] & (1 << xx))
                {
                    plotPixel(buf, x + xx + (i * 8), y + yy, colour);
                }
            }
        }
    }

}

int main(int argc, char* argv[])
{
    romfsInit();
    gfxInit(GSP_RGB565_OES, GSP_RGB565_OES, false);
    consoleInit(GFX_BOTTOM, NULL);
  //  gfxSetDoubleBuffering(GFX_TOP, false);
    u8 *topscr = (u8*)topscrADR;
    memset(topscr, 255, BUF_SIZE);

    for(int i = 0; i < 400*240; i++)
    {
        ((u16 *)topscr)[i] = 0x1F;
    }

    for(int i = 0; i < 240; i ++)
    {
        plotPixel(topscr, 20, i, {0,255,255});
    }
    for(int i = 0; i < 400; i ++)
    {
        plotPixel(topscr, i, 0, {255,0,255});
    }

    plotPixel(topscr, 399, 239, {0, 0, 0});
    plotPixel(topscr, 0, 0, {0,0,0});
    plotPixel(topscr, 0, 239, {0,0,0});
    plotPixel(topscr, 399, 0, {0, 0, 0});
    plotPixel(topscr, 1, 100, {0,0,0});

    u64 start = svcGetSystemTick();
    renderRect(topscr, 100, 100, 20, 20, {255,255,255});
    u64 stop = svcGetSystemTick();

    u64 duration = stop - start;
    u16 test[] = {0x1234, 0x0000, 0x0000, 0x0000, 0x1234, 0x0000, 0x0000, 0x0000, 0x0000,0x0000, 0x1234,
                  0x0000, 0x0000, 0x0000, 0x0000, 0x1234, 0x0000, 0x0000, 0x0000, 0x0000,0x0000, 0x0000,
                  0x0000, 0x0000, 0x0000, 0x0000, 0x1234, 0x0000, 0x0000, 0x0000, 0x0000,0x0000, 0x0000,
                  0x0000, 0x0000, 0x0000, 0x0000, 0x1234, 0x0000, 0x0000, 0x0000, 0x0000,0x0000, 0x0000,
                  0x0000, 0x0000, 0x0000, 0x0000, 0x1234, 0x0000, 0x0000, 0x0000, 0x0000,0x0000, 0x0000,
                  0x0000, 0x0000, 0x0000, 0x0000, 0x1234, 0x0000, 0x0000, 0x0000, 0x0000,0x0000, 0x0000,
                  0x0000, 0x0000, 0x0000, 0x0000, 0x1234, 0x0000, 0x0000, 0x0000, 0x0000,0x0000, 0x0000,
                  0x0000, 0x0000, 0x0000, 0x0000, 0x1234, 0x0000, 0x0000, 0x0000, 0x0000,0x0000, 0x0000,
                  0x0000, 0x0000, 0x0000, 0x0000, 0x1234, 0x0000, 0x0000, 0x0000, 0x0000,0x0000, 0x0000,
                  0x1234, 0x0000, 0x0000, 0x0000, 0x1234, 0x0000, 0x0000, 0x0000, 0x0000,0x0000, 0x0000};


    renderText(std::to_string(duration).c_str(), topscr, 0, 100, {100,100,100});
    u64 start1 = svcGetSystemTick();
    drawImg2((u16*)topscr, test, 20, 20, 10, 11);
    u64 stop1 = svcGetSystemTick();

    u64 timme = stop1 - start1;

    std::cout << timme << "\n";

    std::cout << std::hex << (int)(char*)topscr;
    int x = 20, y = 20;

    while(aptMainLoop())
    {

        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();
        if(kDown & KEY_START)
            break;

        if(kHeld & KEY_UP)
        {
            y -= 1;
        }
        else if (kHeld & KEY_DOWN)
        {
            y += 1;
        }

        if (kHeld & KEY_LEFT)
        {
            x -= 1;
        }
        else if (kHeld & KEY_RIGHT)
        {
            x += 1;
        }

        clearBuf(topscr, {0, 100, 100});
        renderRect(topscr, x, y, 20, 20, {255, 255, 255});

        drawImg((u16*)topscr, test, x, y, 10, 11);

        gfxFlushBuffers();
        gfxSwapBuffers();

        gspWaitForVBlank();
        topscr = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, nullptr, nullptr);
    }

    gfxExit();
    romfsExit();
    return 0;
}