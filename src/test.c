#include <stdio.h>

static int Getdec(char hexchar)
{
   if ((hexchar >= '0') && (hexchar <= '9')) return hexchar - '0';
   if ((hexchar >= 'A') && (hexchar <= 'F')) return hexchar - 'A' + 10;
   if ((hexchar >= 'a') && (hexchar <= 'f')) return hexchar - 'a' + 10;

   return -1; // Wrong character

}

static void HexColorToFloat(char* HexColor, float* FloatColor)
{
   char* HexColorPtr = HexColor;

   int i = 0;
   for (i = 0; i < 4; i++)
   {
     int IntColor = (Getdec(HexColorPtr[0]) * 16) +
                     Getdec(HexColorPtr[1]);

     FloatColor[i] = (float) IntColor / 255.0;
     HexColorPtr += 2;
   }

}

int main(int argc, char* argv[])
{
   float FloatColor[4];

   HexColorToFloat("FFffff44", FloatColor);

   
   printf("r=%4.2f, g=%4.2f, b=%4.2f, a=%2.4f\n", FloatColor[0], FloatColor[1], FloatColor[2], FloatColor[3]);

   return 0;

} 
