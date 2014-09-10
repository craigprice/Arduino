/*
This example will extract the following from the boot record:

bootsector =  251
partitionsectors =  3962629
bytespersector =  512
sectorspercluster =  64
reservedsectors =  1
fatcopies =  2
rootdirnumber =  512
sectorsperFAT =  242
totalfilesectors =  3962629
 
FAT1start =  252
FAT2start =  494
rootdirectorystart =  736
datastartsector =  768
clusterbytes =  32768
maximumdataclusters =  61904
maximum clusters per file spread over all files =  120

The bootsector will also be dumped.

*/

#include <SDCARD.h>

unsigned char buffer[512] ;  // this 512 bytes read from or written to sd card
unsigned long sector = 0;  // the sector we will write or read from

void setup()			  // run once, when the sketch starts
{
 Serial.begin(9600);			   // initialize serial communication with computer
}

void loop()			   // run over and over again
{       struct partition
        {
        long bootsector;  //is at offset 454
        long partitionsectors;  //is at offset 0x457
        }; //partition sector information
        partition* pp = (partition*) & buffer[454]; //pointer to structure & with buffer[]

	 int error = SDCARD.readblock(0);  //look at sector 0 it may be the boot sector
         if (error !=0)
        {
         Serial.print("sd card read error... code =  ");
         Serial.println(error); 
        }//end of if we have an error
        else
        {
          if((buffer[0] == 235) && (buffer[11] == 0) && (buffer[12] == 2) && (buffer[16] == 2))
        {  //in a formated card must have first byte = 235, 512 bytes per sector and 2 FAT's     
         pp -> bootsector = 0;
         pp -> partitionsectors = 0;
        }//sector 0 is the boot sector 
        }//end of read without error       	 
         
        Serial.print("bootsector =  ");
        Serial.println(pp -> bootsector);        
        Serial.print("partitionsectors =  ");
        Serial.println(pp -> partitionsectors);
 
       long bootsector = pp -> bootsector;
       struct bootsectordata
        {
         int bytespersector; //is at offset 11
         char sectorspercluster;  //is at offset 13
         int reservedsectors;  //is at offset 14
         char fatcopies;  //is at offset 16
         int rootdirnumber;  //is at offset 17
         char notneeded1[3];  //we done need these 3
         int sectorsperFAT;  //is at offset 22
         char notneeded2[8];  //we done need these 8
         long totalfilesectors;  //is at offset 32
        };  //boot sector information
       bootsectordata* pb = (bootsectordata*) & buffer[11]; //pointer to structure & with buffer[]
            
         error = SDCARD.readblock(bootsector);  //look at sector 0 it may be the boot sector
         if (error !=0)
        {
         Serial.print("sd card read error... code =  ");
         Serial.println(error); 
        }//end of if we have an error
        else
        {       
        if((buffer[0] == 235) && ((pb -> bytespersector) == 512) && ((pb -> fatcopies) == 2))
       { //in a formated card must have first byte = 235, 512 bytes per sector and 2 FAT's     
        Serial.print("bytespersector =  ");
        Serial.println(pb -> bytespersector);
        int sc = pb -> sectorspercluster;        
        Serial.print("sectorspercluster =  ");
        Serial.println(sc);     
        Serial.print("reservedsectors =  ");
        Serial.println(pb -> reservedsectors);
        sc = pb -> fatcopies;        
        Serial.print("fatcopies =  ");
        Serial.println(sc);       
        Serial.print("rootdirnumber =  ");
        Serial.println(pb -> rootdirnumber);          
        Serial.print("sectorsperFAT =  ");
        Serial.println(pb -> sectorsperFAT);       
        Serial.print("totalfilesectors =  ");
        Serial.println(pb -> totalfilesectors);
       Serial.println(" ");
        long FAT1start = bootsector + (pb -> reservedsectors);
        Serial.print("FAT1start =  ");
        Serial.println(FAT1start);
        long FAT2start = FAT1start + pb -> sectorsperFAT;
        Serial.print("FAT2start =  ");
        Serial.println(FAT2start);
        long rootdirectorystart = bootsector + (pb -> reservedsectors) + (pb -> fatcopies)*(pb -> sectorsperFAT);
        Serial.print("rootdirectorystart =  ");
        Serial.println(rootdirectorystart);
        long datastartsector = rootdirectorystart + ((pb -> rootdirnumber)*32 + (pb -> bytespersector) - 1)/(pb -> bytespersector);
        Serial.print("datastartsector =  ");
        Serial.println(datastartsector);
        unsigned int scc = pb -> sectorspercluster;        
        unsigned long clusterbytes = scc * 512;
        Serial.print("clusterbytes =  ");
        Serial.println(clusterbytes);
        long maximumdataclusters = ((pb -> totalfilesectors) - datastartsector) / scc;
        Serial.print("maximumdataclusters =  ");
        Serial.println(maximumdataclusters);
        long maximumfileclusters = maximumdataclusters / (pb -> rootdirnumber);
        Serial.print("maximum clusters per file spread over all files =  ");
        Serial.println(maximumfileclusters);
       }//end of this card is formatted
      else
      {
        Serial.println(" ");
        Serial.println("This card is not formatted");
      }//end of card is not formatted
       printsector();  //dump the bootsector
     }//end of read without error   
        
    while( 0 == 0);   //wait here
                    
   
}//end of loop

void printsector(void)
{
       Serial.println("");
        
       Serial.println("The boot sector in raw numbers:");
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
     Serial.println("The boot sector in ASCII:");
      for(int i=0; i<512; i++) //write as characters 512 bytes
     {
       number = buffer[i];    //convert to integer       
       if(((i % 32) == 0) & ( i != 0))  
       Serial.println("");   //write 32 then start a new line
       Serial.print(char(number));
       Serial.print("  ");   //tab to next number
     }//end of write 512 bytes
    
}//end of print sector


