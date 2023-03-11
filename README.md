# SDL_BGI Clock with Qurtz like action

In This version of the Clock I am spliting the line drawing for the second hand into n number of frames per second. This gives the second hand a somewhat smooth "Quarts Motion".  
  
The rendering timing was a little bit tricky to get correct and is done by testing the loops per second against the timer and adjusting the delay() period to synchronize with each whole second.  
There **are** other ways to achieve this, but have tried to keep as much as I can within the SDL_Bgi graphics.h library without introducing additional APIs :)


You will need [SDL2](https://github.com/libsdl-org/SDL) development version and [SDL_Bgi V3](https://sdl-bgi.sourceforge.io/).  
Please read my booklet ["A Beginners Guide To SDL_bgi.pdf"](https://github.com/Axle-Ozz-i-sofT/A-BEGINNERS-GUIDE-TO-PROGRAMMING/tree/main/Supplimental/A%20Beginners_Guide_To_SDL_bgi) for how to set up the library as well as setting up the project in Dev-C++ and Code::Blocks.

At the moment I have only done some C language demos but I am looking at creating some SDL_Bgi demos in both FreeBASIC and Python3 as well.

Axle  
