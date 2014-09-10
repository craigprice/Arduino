/*
This example dumps the specified sector.

first copy in raw numbers

2nd copy as the ascii values
*/
#include <SDCARD.h>
unsigned char buffer[512] ;  // this 512 bytes read from or written to sd card
unsigned long sector = 8;  // the sector we dump


void setup()			  // run once, when the sketch starts
{
       Serial.begin(9600);			   // initialize serial communication with computer
       
        int error = SDCARD.readblock(sector);  //read into the buffer this sector in sd card
         if (error !=0)
     {
         Serial.print("sd card read error... code =  ");
         Serial.println(error);
     }//end of if we have an error
      else
     { 
       Serial.println("");       
       Serial.println("The sector in raw numbers:");
       int number =0;
       for(int i=0; i<512; i++) //write raw 512 bytes
     {
       number = buffer[i];    //convert to integer       
       if(((i % 32) == 0) & ( i != 0))  
       Serial.println("");   //write 32 then start a new line
       Serial.print(number);
       Serial.print("  ");   //tab to next number
     }//end of write 512 bytes
     Serial.println("");
     Serial.println("");
     Serial.println("The sector in ASCII:");
      for(int i=0; i<512; i++) //write as characters 512 bytes
     {
       number = buffer[i];    //convert to integer       
       if(((i % 32) == 0) & ( i != 0))  
       Serial.println("");   //write 32 then start a new line
       Serial.print(char(number));
       Serial.print("  ");   //tab to next number
     }//end of write 512 bytes
   }//end of good read
}//end of setup

void loop(){}			   // run over and over again

 



