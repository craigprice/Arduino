/*
This example creates 512 files DATA0000.TXT - DATA0511

This fills the root directory

Each file is 1 sector long. 
*/

#include <SDCARDFILES.h>

unsigned char buffer[512] ;  // this 512 bytes read from or written to sd card

void setup() 
{
Serial.begin(9600); 
delay(5000);

for(int k = 0; k < 512; k++) //use all the root directory
{
   int error = SDCARDFILES.createfile();
   if (error !=0)
     {
         Serial.print("CREATE sd card  error... code =  ");
         Serial.println(error);	 
     }//end of if we have an error
   else
   { 
 
   for(int i = 0; i < 512; i++)
        buffer[i] = i % 10 + 48;  //fill with 0123456789 
     int error = SDCARDFILES.fileappend();  //append one sector to created file
      if (error !=0)
     {
         Serial.print("APPEND sd card  error... code =  ");
         Serial.println(error);	 
     }//end of if we have an error
      Serial.print("completed  ");
      Serial.print(k + 1);
      Serial.println("  files");     
   }//end of no error
}//end of create k files

}//end setup

void loop(){}
