#include "lightTrig.h"

float sinLW(int x)
{
  int x2 = x*x;
  int x3 = x2*x;
  int x4 = x2*x2;
  int x5 = x2*x3;
  int x6 = x3*x3;
  return(x - (x3)/(3*2)+(x5)/(5*4*3*2)-(x6*x)/(7*6*5*4*3*2)+
        (x6*x3)/(9*8*7*6*5*4*3*2*1)-(x6*x5)/(11*10*9*8*7*6*5*4*3*2));
}

float cosLW(int x)
{
  int x2 = x*x;
  int x3 = x2*x;
  int x4 = x2*x2;
  int x5 = x2*x3;
  int x6 = x3*x3;
  return (1-(x2)/(2)+(x4)/(4*3*2)-(x6)/(6*5*4*3*2)
         +(x6*x2)/(8*7*6*5*4*3*2)-(x5*x5)/(10*9*8*7*6*5*4*3*2));
}
