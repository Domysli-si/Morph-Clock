
#ifndef TINYFONT_H
#define TINYFONT_H

#include <Arduino.h>
#include <PxMatrix.h> 

#define TF_COLS 4
#define TF_ROWS 5
typedef struct TFFace 
{
  char fface[TF_ROWS]; //4 cols x 5 rows
};

void TFDrawChar (PxMATRIX* d, char value, char xo, char yo, int col);

void TFDrawText (PxMATRIX* d, String text, char xo, char yo, int col);

#endif
