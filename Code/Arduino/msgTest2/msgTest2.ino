/*This is the same as msgTest1 except it displays information
about what leds would be turned on for each frequency band.*/

int analogPin = 0; // read from multiplexer using analog input 0
int strobePin = 7; // strobe is attached to digital pin 7
int resetPin = 5; // reset is attached to digital pin 5
int spectrumValue[7]; // to hold a2d values

int recentMax[]={15,15,15,15,15,15,15}; //hold the recent max value returned by MSG
int recentMin[]={600,600,600,600,600,600,600}; //hold the recent min value returned by MSG

int bandLedNum[]={7,7,5,7,7,5,3};

int bandMinGap[]={20,20,20,20,20,20,30};  //Min gap between recentMin and recentMax

int chanHeight=0;  //holds the number of LEDs to activate
     int ledOffset=0;   //holds the current offset for the led index
     int chanHeightTotal=0; //holds the total number of LEDs to activate
                            //this is to protect against all the lights coming on
                            //because of static or equipment pop   

void setup() 
{
  Serial.begin(9600);
  pinMode(analogPin, INPUT);
  pinMode(strobePin, OUTPUT);
  pinMode(resetPin, OUTPUT);
  analogReference(DEFAULT);

  digitalWrite(resetPin, LOW);
  digitalWrite(strobePin, HIGH);

  Serial.println("MSGEQ7 test by J Skoba");
}

void loop() 
{
  digitalWrite(resetPin, HIGH);
  digitalWrite(resetPin, LOW);

  for (int i = 0; i < 7; i++)
  {
    digitalWrite(strobePin, LOW);
    delayMicroseconds(30); // to allow the output to settle
    spectrumValue[i] = analogRead(analogPin);
    
    digitalWrite(strobePin, HIGH);
  }
  
  //for each frequency band
  for (int i = 0; i < 7; i++)
  {
    // comment out/remove the serial stuff to go faster
    // - its just here for show
    if (spectrumValue[i] < 10)
    {
      Serial.print("    ");
      Serial.print(spectrumValue[i]);
    }
    else if (spectrumValue[i] < 100 )
    {
      Serial.print("   ");
      Serial.print(spectrumValue[i]);
    }
    else
    {
      Serial.print("  ");
      Serial.print(spectrumValue[i]);
    }
    
    //keep track of the max for scaling
      if(spectrumValue[i]>recentMax[i])
      {
        recentMax[i]=spectrumValue[i];
      }
      //keep track of the min for scaling
      if(spectrumValue[i]<recentMin[i])
      {
        recentMin[i]=spectrumValue[i];
      }
      
  }
  for (int i = 0; i < 7; i++)
  {    
          //calculate the percentage of the max
          //and multiply it times the number of leds that represent the current band
          //Check for a minimum difference between the recentMin and recentMax. This is because
          //when the audio is quiet, the recentMax and recentMin will be very close. This will
          //cause alot of fluctuations.
          if(recentMax[i]-recentMin[i]>bandMinGap[i])
          {
           chanHeight=(int)((float((spectrumValue[i]-recentMin[i]))/((float)recentMax[i]-recentMin[i]))*bandLedNum[i]);
          }
          else
          {
            chanHeight=(int)((float((spectrumValue[i]-recentMin[i]))/((float)bandMinGap[i]))*bandLedNum[i]);
          }
         //chanHeight=(int)((float((spectrumValue[i]-(recentMin[i]/2)))/(recentMax[i]-recentMin[i]))*bandLedNum[i]);
         chanHeightTotal=chanHeightTotal+chanHeight;
                  
         Serial.print("  ");
         Serial.print(chanHeight);
         
         //Shrink the gap between max and min to adjust to changes in overall volume levels
         if(recentMin[i]<1000)
           recentMin[i]++;
         if(recentMax[i]>0 && recentMax[i]>recentMin[i])
           recentMax[i]--;
  }
//  //Show recent min
  for (int i = 0; i < 7; i++)
  {
    Serial.print("  ");
    Serial.print(recentMin[i]);
  }
//  //Show recent max
  for (int i = 0; i < 7; i++)
  {
    Serial.print("  ");
    Serial.print(recentMax[i]);
  }
      
  Serial.println();
}

