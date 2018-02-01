//
//  GPS-FGPMMOPA6H.c
//  CU-in-Space-2018-Avionics-Software
//
//  GPS
//

#include "GPS-FGPMMOPA6H.h"



//Convert gsp time from the format "hhmmss.sss" to an integer number of milliseconds
//Uses a cumulative sum approach, on each iteration the contribution of the previous digits
//is 10 times as much as the current except the jump from hrs to mins and mins to secs which occur at i=2, 4
/* 
int time = 0;
for (EACH digit IN STRING){
  if(digit == '.'){continue;} //skip decimal place
  time *= 10;
  if(i == 2 || i == 4){
      time *= 6; //jump from hrs->mins or mins->secs, is 60 times larger instead of 10 (already did ten, only need extra 6)
  }
  time += (digit - '0'); //!! pretty sure this is how to do conversion from ascii to number but not sure
}
return time;
*/
