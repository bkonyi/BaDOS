#include <track_maps.h>

void track_a_sensor_char_init(sensor_map_chars_t* sensor_chars){
      //inialize mem
      int i,j;
      for(i = 0; i < 16; i++) {
          for(j = 0; j < 16; j++) {
              sensor_chars[MAP_DRAW_COORDS(i,j)].x = -1;
              sensor_chars[MAP_DRAW_COORDS(i,j)].y = -1;
              sensor_chars[MAP_DRAW_COORDS(i,j)].original = '\0';
              sensor_chars[MAP_DRAW_COORDS(i,j)].activated = '\0';
          }   
      }
      sensor_chars[MAP_DRAW_COORDS(0,0)].x = 9;
      sensor_chars[MAP_DRAW_COORDS(0,0)].y = 0;
      sensor_chars[MAP_DRAW_COORDS(0,0)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(0,0)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(0,9)].x = 6;
      sensor_chars[MAP_DRAW_COORDS(0,9)].y = 12;
      sensor_chars[MAP_DRAW_COORDS(0,9)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(0,9)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(0,10)].x = 1;
      sensor_chars[MAP_DRAW_COORDS(0,10)].y = 11;
      sensor_chars[MAP_DRAW_COORDS(0,10)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(0,10)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(0,11)].x = 1;
      sensor_chars[MAP_DRAW_COORDS(0,11)].y = 11;
      sensor_chars[MAP_DRAW_COORDS(0,11)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(0,11)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(0,12)].x = 8;
      sensor_chars[MAP_DRAW_COORDS(0,12)].y = 1;
      sensor_chars[MAP_DRAW_COORDS(0,12)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(0,12)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(0,13)].x = 8;
      sensor_chars[MAP_DRAW_COORDS(0,13)].y = 1;
      sensor_chars[MAP_DRAW_COORDS(0,13)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(0,13)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(0,14)].x = 5;
      sensor_chars[MAP_DRAW_COORDS(0,14)].y = 2;
      sensor_chars[MAP_DRAW_COORDS(0,14)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(0,14)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(0,15)].x = 5;
      sensor_chars[MAP_DRAW_COORDS(0,15)].y = 2;
      sensor_chars[MAP_DRAW_COORDS(0,15)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(0,15)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(0,1)].x = 9;
      sensor_chars[MAP_DRAW_COORDS(0,1)].y = 0;
      sensor_chars[MAP_DRAW_COORDS(0,1)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(0,1)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(0,2)].x = 11;
      sensor_chars[MAP_DRAW_COORDS(0,2)].y = 5;
      sensor_chars[MAP_DRAW_COORDS(0,2)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(0,2)].original = '|';
      sensor_chars[MAP_DRAW_COORDS(0,3)].x = 11;
      sensor_chars[MAP_DRAW_COORDS(0,3)].y = 5;
      sensor_chars[MAP_DRAW_COORDS(0,3)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(0,3)].original = '|';
      sensor_chars[MAP_DRAW_COORDS(0,4)].x = 10;
      sensor_chars[MAP_DRAW_COORDS(0,4)].y = 14;
      sensor_chars[MAP_DRAW_COORDS(0,4)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(0,4)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(0,5)].x = 10;
      sensor_chars[MAP_DRAW_COORDS(0,5)].y = 14;
      sensor_chars[MAP_DRAW_COORDS(0,5)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(0,5)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(0,6)].x = 8;
      sensor_chars[MAP_DRAW_COORDS(0,6)].y = 13;
      sensor_chars[MAP_DRAW_COORDS(0,6)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(0,6)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(0,7)].x = 8;
      sensor_chars[MAP_DRAW_COORDS(0,7)].y = 13;
      sensor_chars[MAP_DRAW_COORDS(0,7)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(0,7)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(0,8)].x = 6;
      sensor_chars[MAP_DRAW_COORDS(0,8)].y = 12;
      sensor_chars[MAP_DRAW_COORDS(0,8)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(0,8)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(1,0)].x = 26;
      sensor_chars[MAP_DRAW_COORDS(1,0)].y = 10;
      sensor_chars[MAP_DRAW_COORDS(1,0)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(1,0)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(1,9)].x = 1;
      sensor_chars[MAP_DRAW_COORDS(1,9)].y = 14;
      sensor_chars[MAP_DRAW_COORDS(1,9)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(1,9)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(1,10)].x = 1;
      sensor_chars[MAP_DRAW_COORDS(1,10)].y = 13;
      sensor_chars[MAP_DRAW_COORDS(1,10)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(1,10)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(1,11)].x = 1;
      sensor_chars[MAP_DRAW_COORDS(1,11)].y = 13;
      sensor_chars[MAP_DRAW_COORDS(1,11)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(1,11)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(1,12)].x = 30;
      sensor_chars[MAP_DRAW_COORDS(1,12)].y = 7;
      sensor_chars[MAP_DRAW_COORDS(1,12)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(1,12)].original = '\\';
      sensor_chars[MAP_DRAW_COORDS(1,13)].x = 30;
      sensor_chars[MAP_DRAW_COORDS(1,13)].y = 7;
      sensor_chars[MAP_DRAW_COORDS(1,13)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(1,13)].original = '\\';
      sensor_chars[MAP_DRAW_COORDS(1,14)].x = 11;
      sensor_chars[MAP_DRAW_COORDS(1,14)].y = 7;
      sensor_chars[MAP_DRAW_COORDS(1,14)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(1,14)].original = '|';
      sensor_chars[MAP_DRAW_COORDS(1,15)].x = 11;
      sensor_chars[MAP_DRAW_COORDS(1,15)].y = 7;
      sensor_chars[MAP_DRAW_COORDS(1,15)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(1,15)].original = '|';
      sensor_chars[MAP_DRAW_COORDS(1,1)].x = 26;
      sensor_chars[MAP_DRAW_COORDS(1,1)].y = 10;
      sensor_chars[MAP_DRAW_COORDS(1,1)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(1,1)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(1,2)].x = 24;
      sensor_chars[MAP_DRAW_COORDS(1,2)].y = 9;
      sensor_chars[MAP_DRAW_COORDS(1,2)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(1,2)].original = '/';
      sensor_chars[MAP_DRAW_COORDS(1,3)].x = 24;
      sensor_chars[MAP_DRAW_COORDS(1,3)].y = 9;
      sensor_chars[MAP_DRAW_COORDS(1,3)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(1,3)].original = '/';
      sensor_chars[MAP_DRAW_COORDS(1,6)].x = 1;
      sensor_chars[MAP_DRAW_COORDS(1,6)].y = 12;
      sensor_chars[MAP_DRAW_COORDS(1,6)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(1,6)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(1,7)].x = 1;
      sensor_chars[MAP_DRAW_COORDS(1,7)].y = 12;
      sensor_chars[MAP_DRAW_COORDS(1,7)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(1,7)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(1,8)].x = 1;
      sensor_chars[MAP_DRAW_COORDS(1,8)].y = 14;
      sensor_chars[MAP_DRAW_COORDS(1,8)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(1,8)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(2,0)].x = 28;
      sensor_chars[MAP_DRAW_COORDS(2,0)].y = 7;
      sensor_chars[MAP_DRAW_COORDS(2,0)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(2,0)].original = '/';
      sensor_chars[MAP_DRAW_COORDS(2,9)].x = 19;
      sensor_chars[MAP_DRAW_COORDS(2,9)].y = 10;
      sensor_chars[MAP_DRAW_COORDS(2,9)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(2,9)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(2,10)].x = 29;
      sensor_chars[MAP_DRAW_COORDS(2,10)].y = 2;
      sensor_chars[MAP_DRAW_COORDS(2,10)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(2,10)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(2,11)].x = 29;
      sensor_chars[MAP_DRAW_COORDS(2,11)].y = 2;
      sensor_chars[MAP_DRAW_COORDS(2,11)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(2,11)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(2,12)].x = 18;
      sensor_chars[MAP_DRAW_COORDS(2,12)].y = 0;
      sensor_chars[MAP_DRAW_COORDS(2,12)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(2,12)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(2,13)].x = 18;
      sensor_chars[MAP_DRAW_COORDS(2,13)].y = 0;
      sensor_chars[MAP_DRAW_COORDS(2,13)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(2,13)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(2,14)].x = 24;
      sensor_chars[MAP_DRAW_COORDS(2,14)].y = 12;
      sensor_chars[MAP_DRAW_COORDS(2,14)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(2,14)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(2,15)].x = 24;
      sensor_chars[MAP_DRAW_COORDS(2,15)].y = 12;
      sensor_chars[MAP_DRAW_COORDS(2,15)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(2,15)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(2,1)].x = 28;
      sensor_chars[MAP_DRAW_COORDS(2,1)].y = 7;
      sensor_chars[MAP_DRAW_COORDS(2,1)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(2,1)].original = '/';
      sensor_chars[MAP_DRAW_COORDS(2,2)].x = 40;
      sensor_chars[MAP_DRAW_COORDS(2,2)].y = 14;
      sensor_chars[MAP_DRAW_COORDS(2,2)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(2,2)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(2,3)].x = 40;
      sensor_chars[MAP_DRAW_COORDS(2,3)].y = 14;
      sensor_chars[MAP_DRAW_COORDS(2,3)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(2,3)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(2,4)].x = 17;
      sensor_chars[MAP_DRAW_COORDS(2,4)].y = 12;
      sensor_chars[MAP_DRAW_COORDS(2,4)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(2,4)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(2,5)].x = 17;
      sensor_chars[MAP_DRAW_COORDS(2,5)].y = 12;
      sensor_chars[MAP_DRAW_COORDS(2,5)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(2,5)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(2,6)].x = 18;
      sensor_chars[MAP_DRAW_COORDS(2,6)].y = 14;
      sensor_chars[MAP_DRAW_COORDS(2,6)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(2,6)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(2,7)].x = 18;
      sensor_chars[MAP_DRAW_COORDS(2,7)].y = 14;
      sensor_chars[MAP_DRAW_COORDS(2,7)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(2,7)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(2,8)].x = 19;
      sensor_chars[MAP_DRAW_COORDS(2,8)].y = 10;
      sensor_chars[MAP_DRAW_COORDS(2,8)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(2,8)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(3,0)].x = 30;
      sensor_chars[MAP_DRAW_COORDS(3,0)].y = 5;
      sensor_chars[MAP_DRAW_COORDS(3,0)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(3,0)].original = '/';
      sensor_chars[MAP_DRAW_COORDS(3,9)].x = 45;
      sensor_chars[MAP_DRAW_COORDS(3,9)].y = 11;
      sensor_chars[MAP_DRAW_COORDS(3,9)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(3,9)].original = '/';
      sensor_chars[MAP_DRAW_COORDS(3,10)].x = 35;
      sensor_chars[MAP_DRAW_COORDS(3,10)].y = 12;
      sensor_chars[MAP_DRAW_COORDS(3,10)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(3,10)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(3,11)].x = 35;
      sensor_chars[MAP_DRAW_COORDS(3,11)].y = 12;
      sensor_chars[MAP_DRAW_COORDS(3,11)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(3,11)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(3,12)].x = 32;
      sensor_chars[MAP_DRAW_COORDS(3,12)].y = 10;
      sensor_chars[MAP_DRAW_COORDS(3,12)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(3,12)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(3,13)].x = 32;
      sensor_chars[MAP_DRAW_COORDS(3,13)].y = 10;
      sensor_chars[MAP_DRAW_COORDS(3,13)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(3,13)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(3,14)].x = 34;
      sensor_chars[MAP_DRAW_COORDS(3,14)].y = 9;
      sensor_chars[MAP_DRAW_COORDS(3,14)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(3,14)].original = '\\';
      sensor_chars[MAP_DRAW_COORDS(3,15)].x = 34;
      sensor_chars[MAP_DRAW_COORDS(3,15)].y = 9;
      sensor_chars[MAP_DRAW_COORDS(3,15)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(3,15)].original = '\\';
      sensor_chars[MAP_DRAW_COORDS(3,1)].x = 30;
      sensor_chars[MAP_DRAW_COORDS(3,1)].y = 5;
      sensor_chars[MAP_DRAW_COORDS(3,1)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(3,1)].original = '/';
      sensor_chars[MAP_DRAW_COORDS(3,2)].x = 32;
      sensor_chars[MAP_DRAW_COORDS(3,2)].y = 2;
      sensor_chars[MAP_DRAW_COORDS(3,2)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(3,2)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(3,3)].x = 32;
      sensor_chars[MAP_DRAW_COORDS(3,3)].y = 2;
      sensor_chars[MAP_DRAW_COORDS(3,3)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(3,3)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(3,4)].x = 46;
      sensor_chars[MAP_DRAW_COORDS(3,4)].y = 4;
      sensor_chars[MAP_DRAW_COORDS(3,4)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(3,4)].original = '\\';
      sensor_chars[MAP_DRAW_COORDS(1,4)].x = 26;
      sensor_chars[MAP_DRAW_COORDS(1,4)].y = 2;
      sensor_chars[MAP_DRAW_COORDS(1,4)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(1,4)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(3,5)].x = 46;
      sensor_chars[MAP_DRAW_COORDS(3,5)].y = 4;
      sensor_chars[MAP_DRAW_COORDS(3,5)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(3,5)].original = '\\';
      sensor_chars[MAP_DRAW_COORDS(1,5)].x = 26;
      sensor_chars[MAP_DRAW_COORDS(1,5)].y = 2;
      sensor_chars[MAP_DRAW_COORDS(1,5)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(1,5)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(3,6)].x = 45;
      sensor_chars[MAP_DRAW_COORDS(3,6)].y = 1;
      sensor_chars[MAP_DRAW_COORDS(3,6)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(3,6)].original = '\\';
      sensor_chars[MAP_DRAW_COORDS(3,7)].x = 45;
      sensor_chars[MAP_DRAW_COORDS(3,7)].y = 1;
      sensor_chars[MAP_DRAW_COORDS(3,7)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(3,7)].original = '\\';
      sensor_chars[MAP_DRAW_COORDS(3,8)].x = 45;
      sensor_chars[MAP_DRAW_COORDS(3,8)].y = 11;
      sensor_chars[MAP_DRAW_COORDS(3,8)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(3,8)].original = '/';
      sensor_chars[MAP_DRAW_COORDS(4,0)].x = 28;
      sensor_chars[MAP_DRAW_COORDS(4,0)].y = 5;
      sensor_chars[MAP_DRAW_COORDS(4,0)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(4,0)].original = '\\';
      sensor_chars[MAP_DRAW_COORDS(4,9)].x = 44;
      sensor_chars[MAP_DRAW_COORDS(4,9)].y = 9;
      sensor_chars[MAP_DRAW_COORDS(4,9)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(4,9)].original = '/';
      sensor_chars[MAP_DRAW_COORDS(4,10)].x = 42;
      sensor_chars[MAP_DRAW_COORDS(4,10)].y = 12;
      sensor_chars[MAP_DRAW_COORDS(4,10)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(4,10)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(4,11)].x = 42;
      sensor_chars[MAP_DRAW_COORDS(4,11)].y = 12;
      sensor_chars[MAP_DRAW_COORDS(4,11)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(4,11)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(4,12)].x = 39;
      sensor_chars[MAP_DRAW_COORDS(4,12)].y = 10;
      sensor_chars[MAP_DRAW_COORDS(4,12)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(4,12)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(4,13)].x = 39;
      sensor_chars[MAP_DRAW_COORDS(4,13)].y = 10;
      sensor_chars[MAP_DRAW_COORDS(4,13)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(4,13)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(4,1)].x = 28;
      sensor_chars[MAP_DRAW_COORDS(4,1)].y = 5;
      sensor_chars[MAP_DRAW_COORDS(4,1)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(4,1)].original = '\\';
      sensor_chars[MAP_DRAW_COORDS(4,4)].x = 39;
      sensor_chars[MAP_DRAW_COORDS(4,4)].y = 2;
      sensor_chars[MAP_DRAW_COORDS(4,4)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(4,4)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(4,5)].x = 39;
      sensor_chars[MAP_DRAW_COORDS(4,5)].y = 2;
      sensor_chars[MAP_DRAW_COORDS(4,5)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(4,5)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(4,6)].x = 40;
      sensor_chars[MAP_DRAW_COORDS(4,6)].y = 0;
      sensor_chars[MAP_DRAW_COORDS(4,6)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(4,6)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(4,7)].x = 40;
      sensor_chars[MAP_DRAW_COORDS(4,7)].y = 0;
      sensor_chars[MAP_DRAW_COORDS(4,7)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(4,7)].original = '=';
      sensor_chars[MAP_DRAW_COORDS(4,8)].x = 44;
      sensor_chars[MAP_DRAW_COORDS(4,8)].y = 9;
      sensor_chars[MAP_DRAW_COORDS(4,8)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(4,8)].original = '/';
}

void track_b_sensor_char_init(sensor_map_chars_t* sensor_chars){
      int i,j;
      for(i = 0; i < 16; i++) {
          for(j = 0; j < 16; j++) {
              sensor_chars[MAP_DRAW_COORDS(i,j)].x = -1;
              sensor_chars[MAP_DRAW_COORDS(i,j)].y = -1;
              sensor_chars[MAP_DRAW_COORDS(i,j)].original = '\0';
              sensor_chars[MAP_DRAW_COORDS(i,j)].activated = '\0';
          }   
      }

      sensor_chars[MAP_DRAW_COORDS(0,0)].x = 41;
      sensor_chars[MAP_DRAW_COORDS(0,0)].y = 14;
      sensor_chars[MAP_DRAW_COORDS(0,0)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(0,0)].original = '='; //A1
      sensor_chars[MAP_DRAW_COORDS(0,9)].x = 44;
      sensor_chars[MAP_DRAW_COORDS(0,9)].y = 2;
      sensor_chars[MAP_DRAW_COORDS(0,9)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(0,9)].original = '='; //A10
      sensor_chars[MAP_DRAW_COORDS(0,12)].x = 43;
      sensor_chars[MAP_DRAW_COORDS(0,12)].y = 13;
      sensor_chars[MAP_DRAW_COORDS(0,12)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(0,12)].original = '='; //A13
      sensor_chars[MAP_DRAW_COORDS(0,13)].x = 43;
      sensor_chars[MAP_DRAW_COORDS(0,13)].y = 13;
      sensor_chars[MAP_DRAW_COORDS(0,13)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(0,13)].original = '='; //A14
      sensor_chars[MAP_DRAW_COORDS(0,1)].x = 41;
      sensor_chars[MAP_DRAW_COORDS(0,1)].y = 14;
      sensor_chars[MAP_DRAW_COORDS(0,1)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(0,1)].original = '='; //A2
      sensor_chars[MAP_DRAW_COORDS(0,2)].x = 36;
      sensor_chars[MAP_DRAW_COORDS(0,2)].y = 9;
      sensor_chars[MAP_DRAW_COORDS(0,2)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(0,2)].original = '|'; //A3
      sensor_chars[MAP_DRAW_COORDS(0,3)].x = 36;
      sensor_chars[MAP_DRAW_COORDS(0,3)].y = 9;
      sensor_chars[MAP_DRAW_COORDS(0,3)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(0,3)].original = '|'; //A4
      sensor_chars[MAP_DRAW_COORDS(0,4)].x = 40;
      sensor_chars[MAP_DRAW_COORDS(0,4)].y = 0;
      sensor_chars[MAP_DRAW_COORDS(0,4)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(0,4)].original = '='; //A5
      sensor_chars[MAP_DRAW_COORDS(0,5)].x = 40;
      sensor_chars[MAP_DRAW_COORDS(0,5)].y = 0;
      sensor_chars[MAP_DRAW_COORDS(0,5)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(0,5)].original = '='; //A6
      sensor_chars[MAP_DRAW_COORDS(0,6)].x = 42;
      sensor_chars[MAP_DRAW_COORDS(0,6)].y = 1;
      sensor_chars[MAP_DRAW_COORDS(0,6)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(0,6)].original = '='; //A7
      sensor_chars[MAP_DRAW_COORDS(0,7)].x = 42;
      sensor_chars[MAP_DRAW_COORDS(0,7)].y = 1;
      sensor_chars[MAP_DRAW_COORDS(0,7)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(0,7)].original = '='; //A8
      sensor_chars[MAP_DRAW_COORDS(0,8)].x = 44;
      sensor_chars[MAP_DRAW_COORDS(0,8)].y = 2;
      sensor_chars[MAP_DRAW_COORDS(0,8)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(0,8)].original = '='; //A9
      sensor_chars[MAP_DRAW_COORDS(1,0)].x = 21;
      sensor_chars[MAP_DRAW_COORDS(1,0)].y = 4;
      sensor_chars[MAP_DRAW_COORDS(1,0)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(1,0)].original = '='; //B1
      sensor_chars[MAP_DRAW_COORDS(1,9)].x = 48;
      sensor_chars[MAP_DRAW_COORDS(1,9)].y = 0;
      sensor_chars[MAP_DRAW_COORDS(1,9)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(1,9)].original = '='; //B10
      sensor_chars[MAP_DRAW_COORDS(1,10)].x = 49;
      sensor_chars[MAP_DRAW_COORDS(1,10)].y = 1;
      sensor_chars[MAP_DRAW_COORDS(1,10)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(1,10)].original = '='; //B11
      sensor_chars[MAP_DRAW_COORDS(1,11)].x = 49;
      sensor_chars[MAP_DRAW_COORDS(1,11)].y = 1;
      sensor_chars[MAP_DRAW_COORDS(1,11)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(1,11)].original = '='; //B12
      sensor_chars[MAP_DRAW_COORDS(1,12)].x = 17;
      sensor_chars[MAP_DRAW_COORDS(1,12)].y = 7;
      sensor_chars[MAP_DRAW_COORDS(1,12)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(1,12)].original = '\\'; //B13
      sensor_chars[MAP_DRAW_COORDS(1,13)].x = 17;
      sensor_chars[MAP_DRAW_COORDS(1,13)].y = 7;
      sensor_chars[MAP_DRAW_COORDS(1,13)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(1,13)].original = '\\'; //B14
      sensor_chars[MAP_DRAW_COORDS(1,14)].x = 36;
      sensor_chars[MAP_DRAW_COORDS(1,14)].y = 7;
      sensor_chars[MAP_DRAW_COORDS(1,14)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(1,14)].original = '|'; //B15
      sensor_chars[MAP_DRAW_COORDS(1,15)].x = 36;
      sensor_chars[MAP_DRAW_COORDS(1,15)].y = 7;
      sensor_chars[MAP_DRAW_COORDS(1,15)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(1,15)].original = '|'; //B16
      sensor_chars[MAP_DRAW_COORDS(1,1)].x = 21;
      sensor_chars[MAP_DRAW_COORDS(1,1)].y = 4;
      sensor_chars[MAP_DRAW_COORDS(1,1)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(1,1)].original = '='; //B2
      sensor_chars[MAP_DRAW_COORDS(1,2)].x = 23;
      sensor_chars[MAP_DRAW_COORDS(1,2)].y = 5;
      sensor_chars[MAP_DRAW_COORDS(1,2)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(1,2)].original = '/'; //B3
      sensor_chars[MAP_DRAW_COORDS(1,3)].x = 23;
      sensor_chars[MAP_DRAW_COORDS(1,3)].y = 5;
      sensor_chars[MAP_DRAW_COORDS(1,3)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(1,3)].original = '/'; //B4
      sensor_chars[MAP_DRAW_COORDS(1,4)].x = 21;
      sensor_chars[MAP_DRAW_COORDS(1,4)].y = 12;
      sensor_chars[MAP_DRAW_COORDS(1,4)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(1,4)].original = '='; //B5
      sensor_chars[MAP_DRAW_COORDS(1,5)].x = 21;
      sensor_chars[MAP_DRAW_COORDS(1,5)].y = 12;
      sensor_chars[MAP_DRAW_COORDS(1,5)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(1,5)].original = '='; //B6
      sensor_chars[MAP_DRAW_COORDS(1,6)].x = 49;
      sensor_chars[MAP_DRAW_COORDS(1,6)].y = 2;
      sensor_chars[MAP_DRAW_COORDS(1,6)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(1,6)].original = '='; //B7
      sensor_chars[MAP_DRAW_COORDS(1,7)].x = 49;
      sensor_chars[MAP_DRAW_COORDS(1,7)].y = 2;
      sensor_chars[MAP_DRAW_COORDS(1,7)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(1,7)].original = '='; //B8
      sensor_chars[MAP_DRAW_COORDS(1,8)].x = 48;
      sensor_chars[MAP_DRAW_COORDS(1,8)].y = 0;
      sensor_chars[MAP_DRAW_COORDS(1,8)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(1,8)].original = '='; //B9
      sensor_chars[MAP_DRAW_COORDS(2,0)].x = 19;
      sensor_chars[MAP_DRAW_COORDS(2,0)].y = 7;
      sensor_chars[MAP_DRAW_COORDS(2,0)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(2,0)].original = '/'; //C1
      sensor_chars[MAP_DRAW_COORDS(2,9)].x = 29;
      sensor_chars[MAP_DRAW_COORDS(2,9)].y = 4;
      sensor_chars[MAP_DRAW_COORDS(2,9)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(2,9)].original = '='; //C10
      sensor_chars[MAP_DRAW_COORDS(2,10)].x = 28;
      sensor_chars[MAP_DRAW_COORDS(2,10)].y = 12;
      sensor_chars[MAP_DRAW_COORDS(2,10)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(2,10)].original = '='; //C11
      sensor_chars[MAP_DRAW_COORDS(2,11)].x = 28;
      sensor_chars[MAP_DRAW_COORDS(2,11)].y = 12;
      sensor_chars[MAP_DRAW_COORDS(2,11)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(2,11)].original = '='; //C12
      sensor_chars[MAP_DRAW_COORDS(2,12)].x = 29;
      sensor_chars[MAP_DRAW_COORDS(2,12)].y = 14;
      sensor_chars[MAP_DRAW_COORDS(2,12)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(2,12)].original = '='; //C13
      sensor_chars[MAP_DRAW_COORDS(2,13)].x = 29;
      sensor_chars[MAP_DRAW_COORDS(2,13)].y = 14;
      sensor_chars[MAP_DRAW_COORDS(2,13)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(2,13)].original = '='; //C14
      sensor_chars[MAP_DRAW_COORDS(2,14)].x = 23;
      sensor_chars[MAP_DRAW_COORDS(2,14)].y = 2;
      sensor_chars[MAP_DRAW_COORDS(2,14)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(2,14)].original = '='; //C15
      sensor_chars[MAP_DRAW_COORDS(2,15)].x = 23;
      sensor_chars[MAP_DRAW_COORDS(2,15)].y = 2;
      sensor_chars[MAP_DRAW_COORDS(2,15)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(2,15)].original = '='; //C16
      sensor_chars[MAP_DRAW_COORDS(2,1)].x = 19;
      sensor_chars[MAP_DRAW_COORDS(2,1)].y = 7;
      sensor_chars[MAP_DRAW_COORDS(2,1)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(2,1)].original = '/'; //C2
      sensor_chars[MAP_DRAW_COORDS(2,2)].x = 5;
      sensor_chars[MAP_DRAW_COORDS(2,2)].y = 0;
      sensor_chars[MAP_DRAW_COORDS(2,2)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(2,2)].original = '='; //C3
      sensor_chars[MAP_DRAW_COORDS(2,3)].x = 5;
      sensor_chars[MAP_DRAW_COORDS(2,3)].y = 0;
      sensor_chars[MAP_DRAW_COORDS(2,3)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(2,3)].original = '='; //C4
      sensor_chars[MAP_DRAW_COORDS(2,4)].x = 30;
      sensor_chars[MAP_DRAW_COORDS(2,4)].y = 2;
      sensor_chars[MAP_DRAW_COORDS(2,4)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(2,4)].original = '='; //C5
      sensor_chars[MAP_DRAW_COORDS(2,5)].x = 30;
      sensor_chars[MAP_DRAW_COORDS(2,5)].y = 2;
      sensor_chars[MAP_DRAW_COORDS(2,5)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(2,5)].original = '='; //C6
      sensor_chars[MAP_DRAW_COORDS(2,6)].x = 28;
      sensor_chars[MAP_DRAW_COORDS(2,6)].y = 0;
      sensor_chars[MAP_DRAW_COORDS(2,6)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(2,6)].original = '='; //C7
      sensor_chars[MAP_DRAW_COORDS(2,7)].x = 28;
      sensor_chars[MAP_DRAW_COORDS(2,7)].y = 0;
      sensor_chars[MAP_DRAW_COORDS(2,7)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(2,7)].original = '='; //C8
      sensor_chars[MAP_DRAW_COORDS(2,8)].x = 29;
      sensor_chars[MAP_DRAW_COORDS(2,8)].y = 4;
      sensor_chars[MAP_DRAW_COORDS(2,8)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(2,8)].original = '='; //C9
      sensor_chars[MAP_DRAW_COORDS(3,9)].x = 2;
      sensor_chars[MAP_DRAW_COORDS(3,9)].y = 3;
      sensor_chars[MAP_DRAW_COORDS(3,9)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(3,9)].original = '/'; //D10
      sensor_chars[MAP_DRAW_COORDS(3,10)].x = 12;
      sensor_chars[MAP_DRAW_COORDS(3,10)].y = 2;
      sensor_chars[MAP_DRAW_COORDS(3,10)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(3,10)].original = '='; //D11
      sensor_chars[MAP_DRAW_COORDS(3,11)].x = 12;
      sensor_chars[MAP_DRAW_COORDS(3,11)].y = 2;
      sensor_chars[MAP_DRAW_COORDS(3,11)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(3,11)].original = '='; //D12
      sensor_chars[MAP_DRAW_COORDS(3,12)].x = 15;
      sensor_chars[MAP_DRAW_COORDS(3,12)].y = 4;
      sensor_chars[MAP_DRAW_COORDS(3,12)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(3,12)].original = '='; //D13
      sensor_chars[MAP_DRAW_COORDS(3,13)].x = 15;
      sensor_chars[MAP_DRAW_COORDS(3,13)].y = 4;
      sensor_chars[MAP_DRAW_COORDS(3,13)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(3,13)].original = '='; //D14
      sensor_chars[MAP_DRAW_COORDS(3,14)].x = 13;
      sensor_chars[MAP_DRAW_COORDS(3,14)].y = 5;
      sensor_chars[MAP_DRAW_COORDS(3,14)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(3,14)].original = '\\'; //D15
      sensor_chars[MAP_DRAW_COORDS(3,15)].x = 13;
      sensor_chars[MAP_DRAW_COORDS(3,15)].y = 5;
      sensor_chars[MAP_DRAW_COORDS(3,15)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(3,15)].original = '\\'; //D16
      sensor_chars[MAP_DRAW_COORDS(3,2)].x = 15;
      sensor_chars[MAP_DRAW_COORDS(3,2)].y = 12;
      sensor_chars[MAP_DRAW_COORDS(3,2)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(3,2)].original = '='; //D3
      sensor_chars[MAP_DRAW_COORDS(3,3)].x = 15;
      sensor_chars[MAP_DRAW_COORDS(3,3)].y = 12;
      sensor_chars[MAP_DRAW_COORDS(3,3)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(3,3)].original = '='; //D4
      sensor_chars[MAP_DRAW_COORDS(3,4)].x = 3;
      sensor_chars[MAP_DRAW_COORDS(3,4)].y = 11;
      sensor_chars[MAP_DRAW_COORDS(3,4)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(3,4)].original = '\\'; //D5
      sensor_chars[MAP_DRAW_COORDS(3,5)].x = 3;
      sensor_chars[MAP_DRAW_COORDS(3,5)].y = 11;
      sensor_chars[MAP_DRAW_COORDS(3,5)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(3,5)].original = '\\'; //D6
      sensor_chars[MAP_DRAW_COORDS(3,6)].x = 3;
      sensor_chars[MAP_DRAW_COORDS(3,6)].y = 13;
      sensor_chars[MAP_DRAW_COORDS(3,6)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(3,6)].original = '\\'; //D7
      sensor_chars[MAP_DRAW_COORDS(3,7)].x = 3;
      sensor_chars[MAP_DRAW_COORDS(3,7)].y = 13;
      sensor_chars[MAP_DRAW_COORDS(3,7)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(3,7)].original = '\\'; //D8
      sensor_chars[MAP_DRAW_COORDS(3,8)].x = 2;
      sensor_chars[MAP_DRAW_COORDS(3,8)].y = 3;
      sensor_chars[MAP_DRAW_COORDS(3,8)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(3,8)].original = '/'; //D9
      sensor_chars[MAP_DRAW_COORDS(4,9)].x = 1;
      sensor_chars[MAP_DRAW_COORDS(4,9)].y = 6;
      sensor_chars[MAP_DRAW_COORDS(4,9)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(4,9)].original = '/'; //E10
      sensor_chars[MAP_DRAW_COORDS(4,10)].x = 5;
      sensor_chars[MAP_DRAW_COORDS(4,10)].y = 2;
      sensor_chars[MAP_DRAW_COORDS(4,10)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(4,10)].original = '='; //E11
      sensor_chars[MAP_DRAW_COORDS(4,11)].x = 5;
      sensor_chars[MAP_DRAW_COORDS(4,11)].y = 2;
      sensor_chars[MAP_DRAW_COORDS(4,11)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(4,11)].original = '='; //E12
      sensor_chars[MAP_DRAW_COORDS(4,12)].x = 8;
      sensor_chars[MAP_DRAW_COORDS(4,12)].y = 4;
      sensor_chars[MAP_DRAW_COORDS(4,12)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(4,12)].original = '='; //E13
      sensor_chars[MAP_DRAW_COORDS(4,13)].x = 8;
      sensor_chars[MAP_DRAW_COORDS(4,13)].y = 4;
      sensor_chars[MAP_DRAW_COORDS(4,13)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(4,13)].original = '='; //E14
      sensor_chars[MAP_DRAW_COORDS(4,14)].x = 23;
      sensor_chars[MAP_DRAW_COORDS(4,14)].y = 11;
      sensor_chars[MAP_DRAW_COORDS(4,14)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(4,14)].original = '\\'; //E15
      sensor_chars[MAP_DRAW_COORDS(4,15)].x = 23;
      sensor_chars[MAP_DRAW_COORDS(4,15)].y = 11;
      sensor_chars[MAP_DRAW_COORDS(4,15)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(4,15)].original = '\\'; //E16
      sensor_chars[MAP_DRAW_COORDS(4,2)].x = 13;
      sensor_chars[MAP_DRAW_COORDS(4,2)].y = 11;
      sensor_chars[MAP_DRAW_COORDS(4,2)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(4,2)].original = '/'; //E3
      sensor_chars[MAP_DRAW_COORDS(4,3)].x = 13;
      sensor_chars[MAP_DRAW_COORDS(4,3)].y = 11;
      sensor_chars[MAP_DRAW_COORDS(4,3)].activated = '^';
      sensor_chars[MAP_DRAW_COORDS(4,3)].original = '/'; //E4
      sensor_chars[MAP_DRAW_COORDS(4,4)].x = 8;
      sensor_chars[MAP_DRAW_COORDS(4,4)].y = 12;
      sensor_chars[MAP_DRAW_COORDS(4,4)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(4,4)].original = '='; //E5
      sensor_chars[MAP_DRAW_COORDS(4,5)].x = 8;
      sensor_chars[MAP_DRAW_COORDS(4,5)].y = 12;
      sensor_chars[MAP_DRAW_COORDS(4,5)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(4,5)].original = '='; //E6
      sensor_chars[MAP_DRAW_COORDS(4,6)].x = 8;
      sensor_chars[MAP_DRAW_COORDS(4,6)].y = 14;
      sensor_chars[MAP_DRAW_COORDS(4,6)].activated = '<';
      sensor_chars[MAP_DRAW_COORDS(4,6)].original = '='; //E7
      sensor_chars[MAP_DRAW_COORDS(4,7)].x = 8;
      sensor_chars[MAP_DRAW_COORDS(4,7)].y = 14;
      sensor_chars[MAP_DRAW_COORDS(4,7)].activated = '>';
      sensor_chars[MAP_DRAW_COORDS(4,7)].original = '='; //E8
      sensor_chars[MAP_DRAW_COORDS(4,8)].x = 1;
      sensor_chars[MAP_DRAW_COORDS(4,8)].y = 6;
      sensor_chars[MAP_DRAW_COORDS(4,8)].activated = 'v';
      sensor_chars[MAP_DRAW_COORDS(4,8)].original = '/'; //E9    
}

