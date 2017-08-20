//This code tests the remote unit which has motion detection and ambient light detector.
//The motion detection will be used to put the table to sleep when the room is empty.
//The ambient light detector will be used to adjust the LED intensity based on the brightness of the room.

//Circuit:
//5V->2 photoresistors in series->Analog 0 -> 10K potentiometer-> ground

int ledPin = 13; // LED is attached to digital pin 13
byte ambLightPin = A2;
//variables to keep track of the timing of recent interrupts
unsigned long interrupt_time = 0;  
unsigned long last_interrupt_time = 0;

int x = 0; // variable to be updated by the interrupt
int y = 0; // variable to be updated by the interrupt

int timer1_counter;

void setup() 
{
  //Timed interrupt method 1 begin
  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle and call the "Compare A" function below
  //OCR0A = 0xFF;
  //TIMSK0 |= _BV(OCIE0A);
  //Timed interrupt method 1 end

  //Timed interrupt method 2 begin
  // initialize timer1 
  noInterrupts();           // disable all interrupts
  TCCR1A = 0;
  TCCR1B = 0;
  //Timed interrupt method 2 end

  // Set timer1_counter to the correct value for our interrupt interval
  //timer1_counter = 64911;   // preload timer 65536-16MHz/256/100Hz
  //timer1_counter = 64286;   // preload timer 65536-16MHz/256/50Hz
  timer1_counter = 0;   // preload timer 65536-16MHz/256/2Hz
  
  TCNT1 = timer1_counter;   // preload timer
  TCCR1B |= (1 << CS12);    // 256 prescaler 
  TIMSK1 |= (1 << TOIE1);   // enable timer overflow interrupt
    
  Serial.begin(9600);
  pinMode(3,INPUT);
  attachInterrupt(1, motion_ISR, FALLING);  //Interrupt on pin 3
  pinMode(2,INPUT);
  attachInterrupt(0, mode_btn_ISR, LOW);  //Interrupt on pin 2

  interrupts();             // enable all interrupts
}

void loop() 
{
  digitalWrite(ledPin, LOW);
  
  int ambLightValue = analogRead(ambLightPin);
  // print out the value you read:
  Serial.print(ambLightValue);
  Serial.print("    ");  
  Serial.print(x, DEC); //print x to serial monitor
  Serial.print("    ");  
  Serial.println(y, DEC); //print y to serial monitor
  delay(50);        // delay in between reads
}

//Timed interrupt method 2 begin
ISR(TIMER1_OVF_vect)        // interrupt service routine 
{
  TCNT1 = timer1_counter;   // preload timer
  y++;
}
//Timed interrupt method 2 end

//Timed interrupt method 1 begin
// Interrupt is called once a millisecond, 
//SIGNAL(TIMER0_COMPA_vect) 
//{
//  y++;
//}
//Timed interrupt method 1 end

// Interrupt service routine for motion interrupt
void motion_ISR() 
{
  //interrupt_time = millis();
  //check to see if increment() was called in the last 250 milliseconds
  //if (interrupt_time - last_interrupt_time > 1000)
  //{
    x++;
  //  digitalWrite(ledPin, HIGH);
  //  last_interrupt_time = interrupt_time;
  //}
}

// Interrupt service routine for motion interrupt
void mode_btn_ISR() 
{
  interrupt_time = millis();
  //check to see if increment() was called in the last 250 milliseconds
  if (interrupt_time - last_interrupt_time > 200)
  {
    y++;
    last_interrupt_time = interrupt_time;
  }
}
