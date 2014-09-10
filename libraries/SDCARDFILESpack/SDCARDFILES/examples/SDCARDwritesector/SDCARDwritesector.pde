/*
This example writes to the specified sector.

Set fillsector to set the written data
*/

#include <SDCARD.h>
unsigned char buffer[512] ;  // this 512 bytes read from or written to sd card

unsigned long sector = 8;  // the sector we will write or read from
unsigned char fillsector = 0;  //the data to write to a sector

void setup()			  // run once, when the sketch starts
{
  Serial.begin(9600);			   // initialize serial communication with computer 
  Serial.println("To write to this sector enter y");	   
}//end of setup

void loop()   // run over and over again
{
  
   if (Serial.available() > 0)   // do nothing if have not received a byte by serial
   {
    int inByte = Serial.read();
    if (inByte == 'y')  // send a "y" to start the write sector process
    {
 
        int i = 0;  //general purpose counter
        for(i=0;i<512;i++)
	buffer[i]=fillsector;      //fill the buffer with a number between 0 and 255
	
	 int error = SDCARD.writeblock(sector);  //write the buffer to this sector on the sd card	
	 if (error !=0)
     {
         Serial.print("sd card write error... code =  ");
         Serial.println(error);	 
     }//end of if we have an error
     else
        Serial.println(" ");
        Serial.println("sector write complete");
    }//end of byte is "y"
   }//end of no serial available	 
 
}//end of loop			  

 



