#define NO_PORTB_PINCHANGES
#define NO_PORTC_PINCHANGES
#include <PinChangeInt.h>
#include <FlexiTimer2.h>
#include <Servo.h>

#define TOP_LIMIT     5
#define BOTTOM_LIMIT  6
#define LED_OUT       13
#define LED2_OUT      12
#define LED3_OUT      11
#define BODY_SERVO    8
#define SHUTTLE_MOTOR 9
//#define M1  A0
//#define M2  A1

volatile int top_state = LOW;
volatile int bottom_state = LOW;

unsigned char desiredAngle = 90;
volatile unsigned char servoAngle = 90;
char servoDir = 0;

unsigned char latest_interrupted_pin;

volatile int encoderCount = 0;

Servo bodyServo;
Servo shuttleMotor;

void pcInt2() 
{
  latest_interrupted_pin=PCintPort::arduinoPin;
//  interrupt_count[latest_interrupted_pin]++;
}

void tmr2Int()
{
  servoAngle += servoDir;
}

void encoderInt()
{
  encoderCount++;
//  top_state = !top_state;
//  digitalWrite( LED2_OUT, top_state );
}

void setup() 
{
  // put your setup code here, to run once:
  pinMode( TOP_LIMIT, INPUT );
  pinMode( BOTTOM_LIMIT, INPUT );
  pinMode( LED_OUT, OUTPUT );
  pinMode( LED2_OUT, OUTPUT );
  pinMode( LED3_OUT, OUTPUT );
//  pinMode( M1, OUTPUT );
//  pinMode( M2, OUTPUT );

  // Turn on internal pullups  
  digitalWrite(TOP_LIMIT, HIGH);
  digitalWrite(BOTTOM_LIMIT, HIGH );
  
  PCintPort::attachInterrupt(TOP_LIMIT, &pcInt2, RISING);
  PCintPort::attachInterrupt(BOTTOM_LIMIT, &pcInt2, FALLING);
  
  attachInterrupt( 1, encoderInt, RISING );
  
  FlexiTimer2::set(20,tmr2Int);
  FlexiTimer2::start();
  
  bodyServo.attach(BODY_SERVO);
  shuttleMotor.attach(SHUTTLE_MOTOR);
  
  Serial.begin(9600);
}

void loop() 
{
  // put your main code here, to run repeatedly: 
  if (servoAngle > desiredAngle)
    servoDir = -1;
  else if (servoAngle < desiredAngle)
    servoDir = 1;
  else
    servoDir = 0;
    
  bodyServo.write(servoAngle);
  
  // Check for incoming serial data
  if (Serial.available() > 0)
  {
    handleSerial();
  }
}

void handleSerial()
{
  String COMMANDS = "AUBD";
  
  char c = Serial.read();
  if ( c != '\r' && c != '\n' & c != ' ' )
  {
    Serial.print("Received: ");
    Serial.print(c);
    
    if ( COMMANDS.indexOf(c) > -1 )
    {
      int value = Serial.parseInt();
      Serial.print("   int: ");
      Serial.println(value);
    }
    else
      Serial.println();
  }
  
}
  /*
  if (latest_interrupted_pin == TOP_LIMIT)
    digitalWrite(LED_OUT, LOW);
  else if (latest_interrupted_pin == BOTTOM_LIMIT)
    digitalWrite(LED_OUT, HIGH);
  */  
  /*
  bodyServo.write(30);
  delay(2000);
  bodyServo.write(150);
  delay(2000);
  */

/*  
  if (servoAngle == desiredAngle)
  {
    if (desiredAngle == 30)
    {
      desiredAngle = 150;
      shuttleMotor.write(95);
      digitalWrite(M1, LOW);
      digitalWrite(M2, LOW);
      digitalWrite(M1, HIGH);
    }
    else
    {
      desiredAngle = 30;
      shuttleMotor.write(75);
      digitalWrite(M1, LOW);
      digitalWrite(M2, LOW);
      digitalWrite(M2, HIGH);
    }
  }
*/  

