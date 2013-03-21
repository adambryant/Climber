//#include <Arduino.h>
#include <PinChangeInt.h>

#define TOP_LIMIT     5
#define BOTTOM_LIMIT  6
#define LED_OUT       13
#define LED2_OUT      12

volatile int top_state = LOW;
volatile int bottom_state = LOW;

uint8_t latest_interrupted_pin;
uint8_t interrupt_count[20]={0}; // 20 possible arduino pins

void quicfunc() 
{
  latest_interrupted_pin=PCintPort::arduinoPin;
  interrupt_count[latest_interrupted_pin]++;
}

void setup() 
{
  // put your setup code here, to run once:
  pinMode( TOP_LIMIT, INPUT );
  pinMode( BOTTOM_LIMIT, INPUT );
  pinMode( LED_OUT, OUTPUT );
  pinMode( LED2_OUT, OUTPUT );

  // Turn on internal pullups  
  digitalWrite(TOP_LIMIT, HIGH);
  digitalWrite(BOTTOM_LIMIT, HIGH );
  
  PCintPort::attachInterrupt(TOP_LIMIT, &quicfunc, RISING);
  PCintPort::attachInterrupt(BOTTOM_LIMIT, &quicfunc, FALLING);
  
  attachInterrupt( 1, limitSwitchInt, RISING );
}

void loop() 
{
  // put your main code here, to run repeatedly: 
  if (latest_interrupted_pin == TOP_LIMIT)
    digitalWrite(LED_OUT, LOW);
  else if (latest_interrupted_pin == BOTTOM_LIMIT)
    digitalWrite(LED_OUT, HIGH);
}

void limitSwitchInt()
{
  top_state = !top_state;
  digitalWrite( LED2_OUT, top_state );
}
