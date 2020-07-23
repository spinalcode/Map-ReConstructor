
#include <Pokitto.h>
#include <Tilemap.hpp>

#include "font.h"
#include "background.h"
#include "buttonhandling.h"
#include "image.h"

long int myDelay;
long int tempTime;
long int frameCount;
uint8_t fpsCount=0;
long int fpsCounter;
long int lastMillis;


int mapX=0;
int mapY=0;

// print text
void myPrint(char x, char y, const char* text) {
  uint8_t numChars = strlen(text);
  uint8_t x1 = 0;//2+x+28*y;
  for (uint8_t t = 0; t < numChars; t++) {
    uint8_t character = text[t] - 32;
    Pokitto::Display::drawSprite(x+((x1++)*8), y, font88[character]);
  }
}

void setFPS(int fps){
  myDelay = 1000 / fps;
}



/*
     ______                 __                 _______                              
    |   __ \.-----.-----.--|  |.-----.----.   |     __|.----.----.-----.-----.-----.
    |      <|  -__|     |  _  ||  -__|   _|   |__     ||  __|   _|  -__|  -__|     |
    |___|__||_____|__|__|_____||_____|__|     |_______||____|__| |_____|_____|__|__|

*/

uint32_t lineTime=0;

inline void myBGFiller(std::uint8_t* line, std::uint32_t y, bool skip){

    // my Background filer, doesn't take into account scrolling or anything at all
    using PD=Pokitto::Display;

    if(skip) return;

    // set bgcolor different for every line
    Pokitto::Display::palette[0] = hline_pal[hline[(y+(mapY/4))]];
    
    
    int stX = -mapX%bgTileSizeW;
    uint32_t x = stX;
    uint32_t tileIndex = (mapX/bgTileSizeW) + ((y+mapY)/bgTileSizeH) * map[0];
    uint16_t jStart = ((y+mapY) %bgTileSizeH) * bgTileSizeW; // current line in current tile

    uint32_t tileStart;
    uint32_t lineOffset;
    uint32_t lineStart = -stX;

    #define bgTileLine()\
        if(x<0){lineOffset+=lineStart; x=0;}\
        uint32_t isFlipped = map[2+tileIndex]>>15;\
        tileStart = (map[2+tileIndex++]&32767)*tbt;\
        lineOffset = tileStart + jStart;\
        if(isFlipped){\
            lineOffset += bgTileSizeW-1;\
            for(uint32_t b=0; b<bgTileSizeW; b++){\
                line[x++] = tiles[lineOffset--];\
            }\
        }else{\
            for(uint32_t b=0; b<bgTileSizeW; b++){\
                line[x++] = tiles[lineOffset++];\
            }\
        }        

    #define lastTile()\
        uint32_t isFlipped = map[2+tileIndex]>>15;\
        tileStart = (map[2+tileIndex++]&32767)*tbt;\
        lineOffset = tileStart + jStart;\
        if(isFlipped){\
            lineOffset += bgTileSizeW-1;\
            for(uint32_t b=0; b<8; b++){\
                if(x<220){\
                    line[x++] = tiles[lineOffset--];\
                }\
            }\
        }else{\
            for(uint32_t b=0; b<bgTileSizeW; b++){\
                if(x<220){\
                    line[x++] = tiles[lineOffset++];\
                }\
            }\
        }\
        

    #define bgHalfTile()\
        isFlipped = map[2+tileIndex]>>15;\
        tileStart = (map[2+tileIndex++]&32767)*tbt;\
        lineOffset = tileStart + jStart;\
        if(isFlipped){\
            lineOffset += bgTileSizeW-1;\
            for(uint32_t b=0; b<4; b++){\
                if(x<220){\
                    line[x++] = tiles[lineOffset--];\
                }\
            }\
        }else{\
            for(uint32_t b=0; b<4; b++){\
                if(x<220){\
                    line[x++] = tiles[lineOffset++];\
                }\
            }\
        }\

    // half tile for the last column
    #define bgHalfTileLine()\
        tileStart = map[2+tileIndex++]*tbt;\
        lineOffset = tileStart + jStart;\
        line[x++] = tiles[lineOffset++];\
        line[x++] = tiles[lineOffset++];\
        line[x++] = tiles[lineOffset++];\
        line[x++] = tiles[lineOffset++];

    int count = 220/bgTileSizeW;
    for(uint32_t c=0; c<count; c++){
        bgTileLine();
    }
    lastTile();
    bgHalfTile();

}


int main(){
    using PC=Pokitto::Core;
    using PD=Pokitto::Display;
    using PB=Pokitto::Buttons;

    PC::begin();
    PD::load565Palette(palette);
    PD::invisiblecolor = 0;
    PD::persistence = true;
    PD::adjustCharStep = 0;
    PD::adjustLineStep = 0;
    
    //setFPS(30);

    frameCount=0;

    // line filler test
    // 0 = tile layer
    // 1 = sprite layer
    // 2 = next layer
    PD::lineFillers[0] = myBGFiller; // map layer


    while( PC::isRunning() ){
        
        if( !PC::update() ) 
            continue;

        updateButtons();
        PC::sound.updateStream();

        int steps = 1;
        if(_Left[HELD])mapX-=steps;
        if(_Right[HELD])mapX+=steps;
        if(_Up[HELD])mapY-=steps;
        if(_Down[HELD])mapY+=steps;

        if(mapX<0) mapX=0;
        if(mapX>(map[0]*bgTileSizeW)-220-steps) mapX=(map[0]*bgTileSizeW)-220-steps;
        if(mapY<0) mapY=0;
        if(mapY>(map[1]*bgTileSizeH)-176-steps) mapY=(map[1]*bgTileSizeH)-176-steps;
  
        char tempText[10];
        sprintf(tempText,"FPS:%d",fpsCount);
        myPrint(0,0,tempText);

        frameCount++;

        fpsCounter++;
        if(PC::getTime() >= lastMillis+1000){
            lastMillis = PC::getTime();
            fpsCount = fpsCounter;
            fpsCounter = 0;
        }

        //myTilemap.draw(0, 0);

    }
    
    return 0;
}


/*
672x64 map or 857 8x8 tiles, with h-flipping
*/
