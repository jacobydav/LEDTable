/*
 * Test the TLC5947 board.
*/
#include "src\Adafruit_TLC5947\Adafruit_TLC5947.h"


int k;
int numLEDs = 45;  //Total number of leds

// How many boards do you have chained?
#define NUM_TLC5974 2

#define data_pin   11
#define clock_pin  10
#define latch_pin   9
#define oe  -1  // set to -1 to not use the enable pin (its optional)

Adafruit_TLC5947 tlc = Adafruit_TLC5947(NUM_TLC5974, clock_pin, data_pin, latch_pin);

//This is a representation of the layout of the LEDs viewed from above with the control panel above Row 1.
//There are 7 rows, Rows 1,3,5,7 have 6 LEDs
//Rows 2,4,6 have 7 LEDs
byte ledIndArray[]={  29,30,28,27,31,32, //the index for each led
                      25,26,24,42,36,38,37,
                      39,40,41,5,4,3,
                      8,1,6,7,2,45,11,
                      43,44,9,46,10,47,
                      20,23,14,17,21,18,19,
                      16,22,0,12,13,15};

void setup()
{
  Serial.begin(9600);
  
  k=0;
  
  tlc.begin();
  if (oe >= 0) {
    pinMode(oe, OUTPUT);
    digitalWrite(oe, LOW);
  }

  //Set the LEDs to zero
  for(int j=0; j < 48; j++)
  {
      tlc.setPWM(j,0);
  }

  tlc.write();
}

void loop()
{
  int direction = 1;
  long randVal = 0;
  int maxVal = 16;
  int minVal = 0;
  //for (int channel = 0; channel < NUM_TLCS * 16; channel += direction) {
  
  
    /* Tlc.clear() sets all the grayscale values to zero, but does not send
       them to the TLCs.  To actually send the data, call Tlc.update() */
    //Tlc.clear();

    /* Tlc.set(channel (0-15), value (0-4095)) sets the grayscale value for
       one channel (15 is OUT15 on the first TLC, if multiple TLCs are daisy-
       chained, then channel = 16 would be OUT0 of the second TLC, etc.).

       value goes from off (0) to always on (4095).

       Like Tlc.clear(), this function only sets up the data, Tlc.update()
       will send the data. */
      //randVal=random(minVal,maxVal);
      //for(int j=0; j < randVal; j++)
      //{
      //  Tlc.set(j, 3000);
      //} 
      
      //*********test 2 begin: one light at a time***********
      //In order of TLC5497 index      
//      for(int j=0; j < 48; j++)
//      {
//        if(j==k)        
//          tlc.setPWM(j,4000);
//        else
//          tlc.setPWM(j,0);
//      }
//
//      tlc.write();
//      delay(5000);
//
//      k--;
//      if(k==0)
//        k=47;
           
     //**************test 2 end******************
     
     //***********test 3 begin all leds on constant*******************
//    for(int j=0; j < 24; j++)
//    {
//       tlc.setPWM(j,4000);
//    }   
//    tlc.write();

    //delay(1500);
    //**************test 3 end******************
    
    //*********test 4 begin: one light at a time in physical order***********
      //Row by row in order of the physical layout on the table      
      for(int i=0; i <numLEDs; i++)
      {
         for(int j=0; j < 48; j++)
          {            
              tlc.setPWM(j,0);
          }

          tlc.setPWM(ledIndArray[i],4000);

         tlc.write();
       

        delay(500); 
      }
     //**************test 4 end******************   
    //*********test 5 begin: advance to next LED on serial input***********
     // if 1 is sent, increment LED index, if 2 is sent, decrement LED index
//        if (Serial.available() > 0) 
//        {
//                // read the incoming byte:
//                byte incomingByte = Serial.read();
//            if(incomingByte==1)
//            {
//              if(k<47)
//                k++;
//              Serial.print("k=");
//              Serial.println(k);
//            }
//            else if(incomingByte==2)
//            {
//              if(k>0)
//                k--;
//              Serial.print("k=");
//              Serial.println(k);
//            }
//            //Set the LED
//            for(int j=0; j < 48; j++)
//            {
//              if(j==k)        
//                tlc.setPWM(j,4000);
//              else
//                tlc.setPWM(j,0);
//            }
//      
//            tlc.write();
//        }
    /* Tlc.update() sends the data to the TLCs.  This is when the LEDs will
       actually change. */
    
}

