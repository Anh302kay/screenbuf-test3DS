#include <3ds.h>
#include <citro3d.h>
#include <citro2d.h>
#include <iostream>
#include <string>

#include <chrono>

#include "text.hpp"

#define topscrADR ((void *)0x300BB800)
#define BUF_SIZE ((400 * 240 * 2) * 2)

struct Colour
{
    union 
    {
        struct
        {
            u8 a, b, g, r;
        };
        u32 abgr = 0;
    };
    

};

void plotPixel(u8* buf, int x, int y, Colour colour)
{
    if( x < 0 || x >= 400 || y < 0 || y >= 240)
        return;

    
    u32* buf32 = (u32*)buf;
    //const int pos = (((x + 1) * 240) - y - 1) * 3;
    const int pos = (x + 1) * 240 - y - 1;
    if(pos > BUF_SIZE)
        return;
    
    buf32[pos] = colour.abgr;
}

void clearBuf(u8* buf)
{
    memset(buf, 0, BUF_SIZE);
}

void renderRect(u8* buf, int x, int y, unsigned int w, unsigned int h, Colour colour)
{
    //this func is faster than the other one
    if (x < 0 || x >= 400 || y < 0 || y >= 240)
        return;

    if (y + h > 240)
        h = 240 - y - 1;

    u32 line[h] = {255};

    for(unsigned int i = 0; i < h; i++)
    {
        //if()
        line[i] = colour.abgr;
    }

    for(unsigned int i = 0; i < w; i++)
    {
        if(x + i > 400)
            return;
        memcpy(&buf[((x + 1 + i) * 240 - y - h) * 4], &line, h*4);
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
    gfxInit(GSP_RGBA8_OES, GSP_RGBA8_OES, false); //alpha doesn't seem to do anything
    consoleInit(GFX_BOTTOM, NULL);
    gfxSetDoubleBuffering(GFX_TOP, false);
    u8 *topscr = (u8 *)topscrADR;
    memset(topscr, 0, BUF_SIZE);

    for (int i = 1; i < BUF_SIZE; i+= 4)
    {
        topscr[i] = 255;
    }
    
    for(int i = 0; i < 240; i ++)
    {
        plotPixel(topscr, 20, i, {0, 255,255,0});
    }
    for(int i = 0; i < 400; i ++)
    {
        plotPixel(topscr, i, 0, {0, 255,0,255});
    }

    plotPixel(topscr, 399, 239, {0, 0, 0, 0});
    plotPixel(topscr, 0, 0, {0, 0,0,0});
    plotPixel(topscr, 0, 239, {0, 0,0,0});
    plotPixel(topscr, 399, 0, {0, 0, 0, 0});
    plotPixel(topscr, 1, 100, {0, 0,0,0});

    u64 start = svcGetSystemTick();
    renderRect(topscr, 100, 100, 20, 20, {0, 255,255,255});
    //plotPixel(topscr, 100, 100, {255,255,255,255});
    u64 stop = svcGetSystemTick();

    u64 duration = stop - start;

    renderText(std::to_string(duration).c_str(), topscr, 0, 100, {0, 100,100,100});

    std::cout << std::hex << (int)(char*)topscr;

    while(aptMainLoop())
    {

        hidScanInput();
        u32 kDown = hidKeysDown();
        u32 kHeld = hidKeysHeld();
        if(kDown & KEY_START)
            break;

        gfxFlushBuffers();
        gfxSwapBuffers();

        gspWaitForVBlank();
    }

    gfxExit();
    romfsExit();
    return 0;
}