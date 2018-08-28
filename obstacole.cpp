#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include"generare.h"
#include<fstream>

void genereaza_obstacole(SDL_Rect wall[],int &k)
{
    k=0;
    int h,w,x,y;
    std::ifstream f("obstacole.in");
    while(f>>h>>w>>x>>y)
    {
        wall[k].h=h;
        wall[k].w=w;
        wall[k].x=x;
        wall[k].y=y;
        k++;
    }
    f.close();
}
