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
/*still commented out because I don't want to mess up any compiler stuff if this is invalid.
int parse_gps_time(char* stuff){
    int time = 0;
    for (int i = 0; i<10; i++){
      if(i==6){continue;} //if(digit == '.'){continue;} //skip decimal place
      time *= 10;
      if(i == 2 || i == 4){
          time *= 6; //jump from hrs->mins or mins->secs, is 60 times larger instead of 10 (already did ten, only need extra 6)
      }
      time += (stuff[i] - '0'); 
    }
    return time;
}
*/
