#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define ch(x) "/|\\"[sgn(x)+1]
#define sgn(x) (x>0?1:x?-1:0)
main(){long a=0,b=0,c=0,d=0,i=0,j=0;char s[21][40]
;memset(s,' ',0x348);while(j++<0x15)s[j][-1]='\0';
srand(time(NULL));while(++i<<15){switch(rand()%7){
  case 0:c&&(c-=sgn(c)*(rand()%(2*c)));
  case 1:c--,c^=-~-~-~-~-~-~-~-8;break;
  case 2:d=18-~!c,c=-sgn(a)+d>>3;break;
  case 3:c=a=(c>0?1:-1)*(d=b=18);break;
  case 4:c=a=sgn(a)*(d=b*=7.0/9);break;
  case 5:sgn(c)*c>2&&(c-=3*sgn(c),d--);
}s[d][19+c]=ch(c);}while(i&21^21)puts(s[31&i++]);}

