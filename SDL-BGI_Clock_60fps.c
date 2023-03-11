//------------------------------------------------------------------------------
// Name:        SDL-BGI_Clock.c
// Purpose:     Example analog clock (Quartz motion)
// Title:       "Clock"
//
// Platform:    Win64, Ubuntu64
//
// Compiler:    GCC V9.x.x, MinGw-64, libc (ISO C99)
// Depends:     SDL2-devel, SDL_bgi-3.0.0,
//
// Author:      Axle
// Created:     05/01/2023
// Updated:     22/02/2023
// Copyright:   (c) Axle 2022
// Licence:     MIT No Attribution (MIT-0)
//------------------------------------------------------------------------------
// NOTES:
// I am uncertain about the licencing status of the DOS code base from which I
// have built this. There appears to be many copies of the same(-ish) code all
// across the internet, so I suspect it has emerged from some previous code
// source from the days of DOS and BGI graphics.h
// If you feel that I have breached copywright on your code, please let me
// know so that I can rectify or place appropriate attributions as required.
//
// Current sources used:
// http://see-programming.blogspot.com/2013/09/c-program-to-implement-analog-clock.html
// http://programmertutor16.blogspot.com/2013/10/analog-clock-in-c-simplified.html
//
// I have made a number of modifications including 60 ticks per second for the
// second hand equaling 3600 ticks per minute, or 360.0 degrees of the clock face.
// The 3600 ticks per minute is monitored with ticks and the delay is adjusted
// acordingly to keep the screen refresh rate synced between the CPU clocks and
// and the 3600 tics of the second hand. this gives an average of 60 FPS.
// I have changed out the graphics.h delay() with the SDL_delay() to reduce the
// CPU active time.
//------------------------------------------------------------------------------
// See: https://sdl-bgi.sourceforge.io/test/sdlbgidemo.c
// https://sdl-bgi.sourceforge.io/test/sdlbgidemo.c
// For more on SDL mode.
// vsync and FPS:
// https://thenumb.at/cpp-course/sdl2/08/08.html
// https://lazyfoo.net/tutorials/SDL/25_capping_frame_rate/index.php

#include <stdio.h>
//#include <conio.h>
//#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <graphics.h>

// Turn off compiler warnings for unused variables between (Windows/Linux etc.)
#define unused(x) (x) = (x)

// Two seperate implimentations of the x. y rendering functions.
// Only 1 pair is used. I have left both sets so you can see what I have done to
// calculate 3600 second hand tics per minute :)
// 3600 (/60 sec) == 60 frames per second for rendering the second hand.
// for 30 frames per second we would make secx/y[1800]
void m60SecCalc(int radius, int midx, int midy, int secx[3600], int secy[3600]);
void mSecCalc(int radius, int midx, int midy, int secx[60], int secy[60]);
//==============================================================================
// http://see-programming.blogspot.com/2013/09/c-program-to-implement-analog-clock.html
void minSecCalc(int radius, int midx, int midy, int secx[60], int secy[60]);
void calcPoints(int radius, int midx, int midy, int x[12], int y[12]);

//http://programmertutor16.blogspot.com/2013/10/analog-clock-in-c-simplified.html
int main(int argc, char *argv[])
    {
    unused(argc);  // Turns off the compiler warning for unused argc, argv
    unused(argv);  // Turns off the compiler warning for unused argc, argv
    // request auto detection (Old DOS BGI implimentation)
    // Initiate graphics library data structures.
    //int gdriver = DETECT, gmode, err, tmp;
    //int gdriver = DETECT, gmode, err;

    //int gdriver = SDL;  // Use the SDL2 driver (fast)
    //gdriver = SDL_1280x1024; //SDL_1024x768;  //SDL_800x600;  //Set the SDL window size. SDL_FULLSCREEN

    //int j, midx, midy, radius, hr, min, sec;
    int midx, midy, radius, hr, min, sec;

    // Initiate arrays to hold clock graphics data.
    int x[12], y[12], minx[60], miny[60];
    int hrx[12], hry[12];
    int msecx[3600], msecy[3600];
    //char str[256];

    // Initiate Time data structures.
    time_t t1;
    struct tm *data;
    struct timeval tv;
    //struct timezone tz;
    int msec = 0;

    // Set auto ticks update speed and graphics clearing.
    // Current ~= 60 FPS
    // MAX = 11 == 3600/60tps (60FPS)
    int wait = 11;  // 23 == 1200/20fps 25 == 1800/30fps using SDL mode nominal
    int ticks = 0;
    int Last_min = -1;
    //int Last_hr = -1;

    // ==> Load background image (Clockface) to memory.
    // See description of image load routines after main(){}
    char *Cock_face = "purzen_Clock_face_web_blue2.bmp";

    // Get background image to RAM
    // Set the SDL windows options.
    setwinoptions("Load image to RAM", // char *title
                  SDL_WINDOWPOS_CENTERED, // int x
                  SDL_WINDOWPOS_CENTERED, // int y
                  SDL_WINDOW_HIDDEN);  // -1 | SDL_WINDOW_HIDDEN

    // intiiate the graphics driver and window.
    int Win_ID_1 = initwindow(600, 600);
    readimagefile(Cock_face,0,0, getmaxx(), getmaxy());// x, y, W, H
    //refresh();  // DEBUG
    // Get the size of the image to allocate memory.
    unsigned int ImgSize = imagesize(0, 0, getmaxx(), getmaxy());
    // Allocate the memory for the image.
    void *BackGround = malloc(ImgSize);
    // Copy the image (RAW BMP) from the screen into memory.
    getimage (0, 0, getmaxx(), getmaxy(), BackGround);

    // DEBUG (Test if image loaded correctly)
    //getch();  // DEBUG
    //cleardevice();  // DEBUG
    //getch();  // DEBUG
    //putimage (0, 0, BackGround, COPY_PUT);  // DEBUG
    //refresh();  // DEBUG
    //getch();  // DEBUG
    
    // Using a second window instance is not required. I just did it to show
    // the use of multiple windows. SDL_Bgi is limited to creating a maximum
    // of 16 window instances. Even if you closewindow() it will still seams to
    // count toward the maximum allowed (May be a bug). See resetwinoptions()
    // see SDL_bgi.h, SDL_bgi.c:
    //bgi_win[id]
    //bgi_num_windows--;
    //bgi_win[NUM_BGI_WIN];
    closewindow(Win_ID_1);  // Close the window used for loading the image.

    // Set the SDL windows options.
    setwinoptions ("SDL_BGI Clock (600x600) - Any key to quit", // char *title
                   SDL_WINDOWPOS_CENTERED, // int x
                   SDL_WINDOWPOS_CENTERED, // int y
                   -1); // Uint32 flags (See SDL_bgi.c setwinoptions for flags)

                   // only a subset of flag is supported for now
                   // From SDL_bgi.c
                   /*
                    if (flags & SDL_WINDOW_FULLSCREEN         ||
                        flags & SDL_WINDOW_FULLSCREEN_DESKTOP ||
                        flags & SDL_WINDOW_SHOWN              ||
                        flags & SDL_WINDOW_HIDDEN             ||
                        flags & SDL_WINDOW_BORDERLESS         ||
                        flags & SDL_WINDOW_MAXIMIZED          ||
                        flags & SDL_WINDOW_MINIMIZED) */


    int Win_ID_2 = initwindow(600, 600);  // intiiate the graphics driver and window.(1280, 1024)
    // It defaults to fast, so I don't think this is needed.
    sdlbgifast();  // sdlbgiauto(void)

    // Old DOS BGI mode
    // initialize graphic mode (BGI Slow)
    /*
    initgraph(&gdriver, &gmode, "");
    err = graphresult();
    if (err != grOk)
        {
        // error occurred
        printf("Graphics Error: %s",
               grapherrormsg(err));
        return 0;
        }*/

    // mid position in x and y -axis
    midx = getmaxx() / 2;
    midy = getmaxy() / 2;
    radius = 330;

    // Get position to locate numbers in clock
    calcPoints(radius - 30, midx, midy, x, y);

    // Get position for hour needle
    calcPoints(radius - 90, midx, midy, hrx, hry);

    // Get position for minute needle
    minSecCalc(radius - 50, midx, midy, minx, miny);

    // Get position for minute needle. 60 * 6 ticks per second = 360
    m60SecCalc(radius - 70, midx, midy, msecx, msecy);

    //int maxx = getmaxx();
    //int maxy = getmaxy();

    // For clock hands.
    setlinestyle(SOLID_LINE, 1, 3);

    // Main loop to update the clock graphics.
    // kbkit() is for the console emulator, xkbhit() is for the SDL window.
    while (!xkbhit())// && !kbhit())
        {
        // The following is the method for reading an image from disk into the window.
        // To read from disk each loop cycle is expensive and slow, so we use
        // the memory based image instead. This method is OK for the occasional
        // once of loading of an image outside of the main loop.
        //void readimagefile (char *filename, int x1, int y1, int x2, int y2 );
        //Reads a .bmp file and displays it immediately at (x1, y1 ). If (x2, y2 ) are not 0, the
        //bitmap is stretched to fit the rectangle x1, y1 —x2, y2 ; otherwise, the bitmap is clipped
        //as necessary.
        //readimagefile("purzen_Clock_face_web_blue2.bmp", midx -500, midy -500, 1140, 1010);
        //readimagefile(Cock_face, midx -500, midy -500, midx +500, midy +500);
        //BMP, GIF, JPG, ICO, EMF or WMF image?
        
        // Faster method copying the image from RAM.
        putimage (0, 0, BackGround, COPY_PUT);  // COPY_PUT, "XOR_PUT", OR_PUT

        // Draw the clock face (Old draw method)
        //setlinestyle(SOLID_LINE, 1, 3);
        //settextstyle(TRIPLEX_FONT, 0, 3);
/*
        // Draws frame of the clock
        circle(midx, midy, radius);

        // Place the numbers  in clock
        for (j = 0; j < 12; j++)
            {
            if (j == 0)
                {
                sprintf(str, "%d", 12);
                }
            else
                {
                sprintf(str, "%d", j);
                }
            settextjustify(CENTER_TEXT, CENTER_TEXT);
            moveto(x[j], y[j]);
            outtext(str);
            }
*/
        // Get the current time using time() API
        t1 = time(NULL);
        data = localtime(&t1);
        gettimeofday(&tv, NULL);  //gettimeofday(&tv,&tz);

        // Note that the drawing order is important. Drawing starts at the back
        // layer in the Z order progressing up to the most front layer.


        // Draw the hour needle in clock
        // You can alter the colour of the hands with setcolor()
        hr = data->tm_hour % 12;
        //setcolor(LIGHTGRAY);
        line(midx, midy, hrx[hr], hry[hr]);
        //setcolor(WHITE);

        // Draw the minute needle in clock
        min = data->tm_min % 60;
        //setcolor(LIGHTGRAY);
        line(midx, midy, minx[min], miny[min]);
        //setcolor(WHITE);

        // Draw the seconds needle in clock. I am using 6 steps between each
        // second to create a quartz like action.        
        //msec = (int)(tv.tv_usec /166667);  // 0.0 to 0.9
        //sec = (data->tm_sec * 6 +msec) % 360;  //60 sec * 6 ticks/sec

        msec = (int)(tv.tv_usec /16667);  // 0 to 59 seconds (1000000 / 16667)
        sec = (data->tm_sec * 60 +msec) % 3600;  //60 sec * 60 ticks/sec
        //printf("%d\n", sec);  // DEBUG


        setcolor(GREEN);  // LIGHTBLUE
        line(midx, midy, msecx[sec], msecy[sec]);
        setcolor(WHITE);

        //printf("%d|%d\n", msecx[sec], msecy[sec]);  // DEBUG

        // Check the FPS count for each minute past and adjust the loop speed
        // to sync with the clock time. This could be done every second to make
        // the clock a little more accurate. In this example the clock resets
        // the second hand accuracy each one minute period. this means that it
        // may take a few minutes before the second hand becomes accurate.
        // There are many different ways to achieve the clock sync and each has
        // it's pros and cons depending upon your requirements :)

        // Set minute change test.
        if (Last_min == -1)
            {
            Last_min = min;
            }

        // Update tick/wait count at each minute change.
        if (Last_min != min)
            {
            // Track the Last Minute value.
            Last_min = min;

            // Test the number of ticks per minute. We want to sync the refresh
            // rate as close as possible to the 3600 ticks in a minute.
            // This will auto adjust over time.
            if (ticks < 3600 && wait != 1)  // 1800 == 30 ticks/sec|1200 == 20 tps
                {
                wait--;  // Speed up the FPS
                //printf("ticks:%d\n", ticks);  //Debug
                //printf("wait :%d\n", wait);  //Debug
                ticks = 0;  // reset number of ticks per second.
                }
            else if (ticks > 3600 && wait != 999)
                {
                wait++;  // Slow down the FPS
                //printf("ticks:%d\n", ticks);  //Debug
                //printf("wait :%d\n", wait);  //Debug
                //ticks = 0;  // reset number of ticks per second.
                }
            else
                {
                //printf("ticks:%d\n", ticks);  //Debug
                //printf("wait :%d\n", wait);  //Debug
                ticks = 0;  // reset number of ticks per second.
                }
            }
        else
            {
            // Update the tick count untill it reaches 3600, 60 seconds.
            ticks++;
            }


        // We can use double buffering wich is the standard method to create
        // smooth flowing animations without flicker.
        // Use void sdlbgifast (void); Mode + refresh()
        //swapbuffers(); swapbuffers is the same as the 4 lines below.
        // Use swpapbuffers or getvisualpage() etc
        int oldv = getvisualpage();
        int olda = getactivepage();
        setvisualpage(olda);
        setactivepage(oldv);


        // refresh(), event(), x|kbhit() also preforms a refresh!
        refresh();

        // Clears the display interface (background page).
        cleardevice();

        //clearviewport();
        SDL_Delay(wait);  // used to reduce CPU speed.
        // NOTE! SDL_Delay() can interfere with the SDL_Bgi
        // Sleep() vs deley(): Sleep can sometimes interfere with the graphics
        // display and is non standard to graphics.h but has the advantage of
        // keeping the CPU at an idle state. delay is poratable, but will run
        // the CPU core at close to 100% while the application is running.
        // delay wait is part of SDL-BGI
        //delay(wait); //1000 for sec, 167 for 360 ticks/min
        //Sleep(wait);
        // SDL_Delay() Is the native SDL equivelent to Sleep(), sleep()
        // This labrary runs on top of SDL2 so it is OK to use SDL functions
        // "With Care".
        //SDL_Delay(wait);

        }

    //getch();  // Not really required.

    // deallocate memory allocated for graphic screen
    closewindow(Win_ID_2);
    closegraph();

    return 0;
    }  // <== END main()


// ==> Basic routine for loading BMP images to memory <==
//unsigned imagesize (int left, int top, int right, int bottom );
//Returns the size in bytes of the memory area required to store a bit image. This value
//must be allocated in a buffer before copying the image with getimage().

//getimage (int left, int top, int right, int bottom, void *bitmap );
//Copies a bit image of the specified region into the memory pointed by bitmap, previ-
//ously allocated with malloc (imagesize())(C)

//void putimage (int left, int top, void *bitmap, int op );
//Puts the bit image pointed to by bitmap onto the screen, with the upper left corner of
//the image placed at (left, top ). op specifies the drawing mode (COPY PUT, etc).

// ==> read and load an imagge file (BMP) directly from disc.
//void readimagefile (char *filename, int x1, int y1, int x2, int y2 );
//Reads a .bmp file and displays it immediately at (x1, y1 ). If (x2, y2 ) are not 0, the
//bitmap is stretched to fit the rectangle x1, y1 —x2, y2 ; otherwise, the bitmap is clipped
//as necessary.

//void readimagefile(
//    const char* title=NULL,     
//    int left=0, int right=0, int right=INT_MAX, int bottom=INT_MAX
//    );
//BMP, GIF, JPG, ICO, EMF or WMF image
// <== END Basic image load routines


// A modification of minSecCalc() to acheive 360 x,y points for the second
// hand. (seconds * 6) + (millisecons / 166667)
// 50 * 6 + 10 <- as weird as that seams we start at 0 to 359 in the array.
void m60SecCalc(int radius, int midx, int midy, int secx[3600], int secy[3600])
    {
    //int i, j = 0, x, y;
    int i, j = 0;
    //char str[32];

    /* 90 position(min/sec - 12 to 3) in first quadrant of clock  */
    secx[j] = midx, secy[j++] = midy - radius;
    for (i = 901; i < 1800; i = i + 1)  // +6
        {
        secx[j] = midx - (radius * cos((i * 3.14) / 1800));
        secy[j++] = midy - (radius * sin((i * 3.14) / 1800));
        }

    /* 90 positions(min or sec - 3 to 6) in second quadrant of clock */
    secx[j] = midx + radius, secy[j++] = midy;
    for (i = 1801; i < 2700; i = i + 1)  // +6
        {
        secx[j] = midx - (radius * cos((i * 3.14) / 1800));
        secy[j++] = midy - (radius * sin((i * 3.14) / 1800));
        }

    /* 90 positions(min or sec - 6 to 9) in third quadrant of clock */
    secx[j] = midx, secy[j++] = midy + radius;
    for (i = 2701; i < 3600; i = i + 1)  // +6
        {
        secx[j] = midx - (radius * cos((i * 3.14) / 1800));
        secy[j++] = midy - (radius * sin((i * 3.14) / 1800));
        }

    /* 90 positions(min or sec - 9 to 12) in fourth quadrant of clock */
    secx[j] = midx - radius, secy[j++] = midy;
    for (i = 1; i < 900; i = i + 1)  // +6
        {
        secx[j] = midx - (radius * cos((i * 3.14) / 1800));
        secy[j++] = midy - (radius * sin((i * 3.14) / 1800));
        }

    }


// A modification of minSecCalc() to acheive 360 x,y points for the second
// hand. (seconds * 6) + (millisecons / 166667)
// 50 * 6 + 10 <- as weird as that seams we start at 0 to 359 in the array.
void mSecCalc(int radius, int midx, int midy, int secx[360], int secy[360])
    {
    //int i, j = 0, x, y;
    int i, j = 0;
    //char str[32];

    /* 90 position(min/sec - 12 to 3) in first quadrant of clock  */
    secx[j] = midx, secy[j++] = midy - radius;
    for (i = 91; i < 180; i = i + 1)  // +6
        {
        secx[j] = midx - (radius * cos((i * 3.14) / 180));
        secy[j++] = midy - (radius * sin((i * 3.14) / 180));
        }

    /* 90 positions(min or sec - 3 to 6) in second quadrant of clock */
    secx[j] = midx + radius, secy[j++] = midy;
    for (i = 181; i < 270; i = i + 1)  // +6
        {
        secx[j] = midx - (radius * cos((i * 3.14) / 180));
        secy[j++] = midy - (radius * sin((i * 3.14) / 180));
        }

    /* 90 positions(min or sec - 6 to 9) in third quadrant of clock */
    secx[j] = midx, secy[j++] = midy + radius;
    for (i = 271; i < 360; i = i + 1)  // +6
        {
        secx[j] = midx - (radius * cos((i * 3.14) / 180));
        secy[j++] = midy - (radius * sin((i * 3.14) / 180));
        }

    /* 90 positions(min or sec - 9 to 12) in fourth quadrant of clock */
    secx[j] = midx - radius, secy[j++] = midy;
    for (i = 1; i < 90; i = i + 1)  // +6
        {
        secx[j] = midx - (radius * cos((i * 3.14) / 180));
        secy[j++] = midy - (radius * sin((i * 3.14) / 180));
        }

    }



//==============================================================================
// http://see-programming.blogspot.com/2013/09/c-program-to-implement-analog-clock.html
/*
   * Calculates position for minute and second needle movement
   * Each quadrant has 90 degrees.  So, we need to split each
   * quadrant into 15 parts(6 degree each) to get the minute
   * and second needle movement
*/
void minSecCalc(int radius, int midx, int midy, int secx[60], int secy[60])
    {
    //int i, j = 0, x, y;
    int i, j = 0;
    //char str[32];

    /* 15 position(min/sec - 12 to 3) in first quadrant of clock  */
    secx[j] = midx, secy[j++] = midy - radius;

    for (i = 96; i < 180; i = i + 6)
        {
        secx[j] = midx - (radius * cos((i * 3.14) / 180));
        secy[j++] = midy - (radius * sin((i * 3.14) / 180));
        }

    /* 15 positions(min or sec - 3 to 6) in second quadrant of clock */
    secx[j] = midx + radius, secy[j++] = midy;
    for (i = 186; i < 270; i = i + 6)
        {
        secx[j] = midx - (radius * cos((i * 3.14) / 180));
        secy[j++] = midy - (radius * sin((i * 3.14) / 180));
        }

    /* 15 positions(min or sec - 6 to 9) in third quadrant of clock */
    secx[j] = midx, secy[j++] = midy + radius;
    for (i = 276; i < 360; i = i + 6)
        {
        secx[j] = midx - (radius * cos((i * 3.14) / 180));
        secy[j++] = midy - (radius * sin((i * 3.14) / 180));
        }

    /* 15 positions(min or sec - 9 to 12) in fourth quadrant of clock */
    secx[j] = midx - radius, secy[j++] = midy;
    for (i = 6; i < 90; i = i + 6)
        {
        secx[j] = midx - (radius * cos((i * 3.14) / 180));
        secy[j++] = midy - (radius * sin((i * 3.14) / 180));
        }

    return;
    }

/*
   * find the points at 0, 30, 60,.., 360 degrees
   * on the given circle.  x value correponds to
   * radius * cos(angle) and y value corresponds
   * to radius * sin(angle).  Numbers in the clock
   * are written using the above manipulated x and
   * y values.  And the hour needle movement
   * is based on this
*/

void calcPoints(int radius, int midx, int midy, int x[12], int y[12])
    {
    int x1, y1;

    /* 90, 270, 0, 180 degrees */
    x[0] = midx, y[0] = midy - radius;
    x[6] = midx, y[6] = midy + radius;
    x[3] = midx + radius, y[3] = midy;
    x[9] = midx - radius, y[9] = midy;

    /* 30, 150, 210, 330 degrees */
    x1 = (int) ((radius / 2) * sqrt(3));
    y1 = (radius / 2);
    x[2] = midx + x1, y[2] = midy - y1;
    x[4] = midx + x1, y[4] = midy + y1;
    x[8] = midx - x1, y[8] = midy + y1;
    x[10] = midx - x1, y[10] = midy - y1;

    /* 60, 120, 210, 300 degrees */
    x1 = radius / 2;
    y1 = (int) ((radius / 2)  * sqrt(3));
    x[1] = midx + x1, y[1] = midy - y1;
    x[5] = midx + x1, y[5] = midy + y1;
    x[7] = midx - x1, y[7] = midy + y1;
    x[11] = midx - x1, y[11] = midy - y1;

    return;
    }
//==============================================================================


