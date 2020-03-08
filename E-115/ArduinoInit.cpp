//*************************************************************************************************************
// ArduinoInit.cpp - This file contains C code for "pre-defined" functions that configure and interface with the
// Arduino microcontroller used in the E121 Design I project. This file must be used in conjunction
// with header file ArduinoInit.h. 
// Program written by Prof. Edward Blicharz, Stevens Institute of Technology, Hoboken, NJ
// VERSION 13F-01  (September 14, 2013)  Baseline
// VERSION 13F-02  (October 3, 2013) Adjusted mandatory motor off delays and sychronization between motors
// VERSION 14F-01  (May 22, 2014) New Motor Control Function supporting commands to both motors at once.
//					Eliminated mamdatory motor delays. Students must use delays to wind down EMF on direction change
// VERSION 15S-01  (January 12, 2015) Implement flashing LED on input parameter errors, Modify Motors function to
//					accept both number and ascii Motor # input
// VERSION 15S-02  (April 2, 2015) Corrected bug that prevented motor commands from working in the interrupt function.
//					Modified pause() timing to consider new avr toolchain effective with IDE 1.5.7 Beta
//**************************************************************************************************************

# include "ArduinoInit.h"		

SendOnlySoftwareSerial mySerial (13);  //(Tx Pin)  //Define serial interface to motor controller

char ver[] = "v15S-02"; //FOR DIAGNOSTIC USE- the current version of this library

//***************************************************************************************************************
void flashErrorLED (byte routineID)

// This function is for internal use, not for direct student's use. It will be called only if one of the subfuctions
// used by students is called with an out-of-range input parameter. When called external interrupts are disabled.
// This will stop the motors, turn off any on-board LEDs that may be on, and then enter an endless loop flashing
// the RED LED in a sequence identifying the subfunction that was called with a bad input parameter.
// 1 Flash -	readADC 
// 2 Flashes -	readInput
// 3 Flashes -	outputHigh 
// 4 Flashes -	outputLow
// 5 Flashes -	motors 

{
  commandMotors ('r', 'x', 0);  	//Reset both motors off, 100% full speed
  digitalWrite (10, LOW);
  digitalWrite (11, LOW);
  digitalWrite (12, LOW);
  
  while (1)							//Endless loop, processing dead ends here with input parameter error
  {
	for (byte i=0; i < routineID; i++)
	{
		digitalWrite (10, HIGH);	//Turn on red LED
		pause (150);
		digitalWrite (10, LOW);
		pause (240);
	}
	pause (1000);
  }
}  
  
//***************************************************************************************************************
void tempINT ()

// The default software configuration "enables" the two external bumper interrupts on pins 2 and 3.
// This is a temporary Interrupt Function that must be replaced by students' version using the
// attachInterrupt () statement. tempINT () will confirm the bumper interrupts are operational by flashing
// all three LEDs when an interrupt is activated.  
{
  pause (5);	//allow for switch closure bounce
  while (digitalRead(2)==0 || digitalRead(3)==0) //While a bumper is activated
  {
	digitalWrite (10, HIGH);	//Turn on red LED
	digitalWrite (11, HIGH);	//Turn on yellow LED
	digitalWrite (12, HIGH);	//Turn on green LED
	pause (200);
	digitalWrite (10, LOW);
	digitalWrite (11, LOW);
	digitalWrite (12, LOW);
	pause (200);
  }
  return;
}

//***************************************************************************************************************
void configArduino ()

// This function MUST be called ONCE at the top of the setup() function.
// Configures Arduino hardware to the baseline configuation used for the E121 Design Project
{
  pinMode (13, OUTPUT);				//Serial link TX pin to motor controller
  mySerial.begin (115200);			//Set Baud rate to 115.2K on serial link to motor controller
  pause (5);						//Allow motor controller to start up from POR
  commandMotors ('r', 'x', 0);  	//Reset both motors off, 100% full speed (quickly, they may be running)
  outputLow (10);					//Use Red on-board LED
  outputLow (11);					//Use Yellow on-board LED
  outputLow (12);					//Use Green on-board LED
  attachInterrupt (0, tempINT, LOW);  //Assign temporary interrupt function tempINT() to
  attachInterrupt (1, tempINT, LOW);  //both external bumper interrupts 
  Serial.begin (115200);			//Set Baud rate to 115.2K to IDE terminal window
  interrupts();						//Enable global interrupts
  pause (1000);						
  version();
  return;
} 


//***************************************************************************************************************
void commandMotors (char motor, char direction, int speed)

// This function is for internal use, not for direct student's use. It sends software serial data commands 
// checked for validity by the calling function to the motor controller via Arduino pin 13. External (bumper) 
// interrupts should be disabled by the calling function to allow transmission to complete in its entirety.
{
  pause (2);	//allow time for motor controller to process last command
  mySerial.write ('s');
  mySerial.write (motor);
  if (motor== 'r' || motor== 'R') {mySerial.write ('f'); return;}
  mySerial.write (direction);
  mySerial.write (speed);
  mySerial.write ('f');
  return;
}

//***************************************************************************************************************
void motors (char motor, char direction, int speed)

// This function verifies input paramter ranges and formats a data command for transmission to the motor controller
// Input parameter "motor" can be number or ASCII:
//		'1' or 1 to send command to motor 1
//		'2' or 2 to send command to motor 2
//		'B' or 'b' to send the command to both motors
// Input parameter "direction" must be ASCII:
//		'A' or 'a' to command the motor on and to spin in a direction (dependent on hardware wiring)
//		'B' or 'b' to command the motor on and to spin in the other direction (dependent on hardware wiring)
//		'O' or 'o' (letter O) to command the motor off
//		'X' or 'x' to leave the current motor status as is, i.e., we want to just change motor speed
// Input parameter "speed" must be an unsigned integer number between 25-100 inclusive, or the number 0
//		 25-100 represents % full speed desired
//		 0 (zero) will leave the motor speed unchanged, i.e., we want to just change motor direction.

{
 
  EIMSK=0;
  
  motor = tolower(motor);
  if (motor != '1' && motor != '2' && motor != 1 && motor != 2 && motor != 'b') goto fault;
  if (motor == 1) motor = '1';
  if (motor == 2) motor = '2';
  if ((speed < 25) && (speed != 0)) goto fault;
  if (speed > 100) goto fault;
  direction = tolower(direction);
  if (direction != 'a' && direction !='b' && direction != 'o' && direction != 'x')  goto fault;
  
  if (speed != 0) 
  {
    byte temp1 = speed << 1;   //Covert 25-100% full speed to 8-bit PWM value (approximation)
	byte temp2 = speed >> 1;
	speed = temp1 + temp2 +5;
  }
  
  commandMotors (motor, direction, speed);
  
  EIMSK=3;
  
  return;
  fault:
  flashErrorLED (5);	//halt proceesing, flash RED LED to indicate input parameter error 
  return;
}


//***************************************************************************************************************
unsigned int readADC (byte pin)

// This function reads the analog voltage (A/D converter) on a specified input channel and returns the result.
// Input parameter "channel" must be an integer number between 0-5 inclusive. 
// Output parameter returns an unsigned integer in the range of 00000-50127 (approx) representing a voltage of
// 0-5.0127 volts at the channel. 
{
  EIMSK=0;
  byte i=0;
  unsigned int result=0, ADout=0;
  if (pin>5)
  {
	flashErrorLED (1);	//halt proceesing, flash RED LED to indicate input parameter error
	return 0xFFFF;		//will never return
  }
  ADout = analogRead (pin);
  for (i=1; i<=49; i++)
  {result = result + ADout;}
  EIMSK=3;
  return result;
}

//***************************************************************************************************************
void outputHigh (byte pin)

// This function will configure the specified digital pin as an output and set it's signal level high (5VDC).
// Input parameter "pin" must be an integer between 4-12 inclusive. Note pins 0 and 1 are preallocated to the USB
// serial link, pins 2 ans 3 are preallocated as inputs for the two bumpers and pin 13 is preallocated as thr Tx line
// to the motor controller serial link.
{
  EIMSK=0;
  if (pin <4 || pin >12)
  {
	flashErrorLED (3);	//halt proceesing, flash RED LED to indicate input parameter error
	return;
  }
  pinMode (pin, OUTPUT);
  digitalWrite (pin, HIGH);
  EIMSK=3;
  return;
}

//***************************************************************************************************************
void outputLow (byte pin)

// This function will configure the specified digital pin as an output and set it's signal level low (GND).
// Input parameter "pin" must be an integer between 4-12 inclusive. Note pins 0 and 1 are preallocated to the USB
// serial link, pins 2 ans 3 are preallocated as inputs for the two bumpers and pin 13 is preallocated as the Tx line
// of the serial link to the motor controller.
{
  EIMSK=0;
  if (pin <4 || pin >12)
  {
	flashErrorLED (4);	//halt proceesing, flash RED LED to indicate input parameter error
	return;
  }
  pinMode (pin, OUTPUT);
  digitalWrite (pin, LOW);
  EIMSK=3;
  return;
}

//***************************************************************************************************************
byte readInput (byte pin)

// This function will configure the specified digital pin as an input and return the digital signal level on the 
// pin, either 1 (5VDC) or 0 (GND). Input parameter "pin" must be an integer between 2-9 inclusive. Note pins 0 and 1
// are preallocated to the USB serial link, pins 10, 11, 12 are preallocated as outputs for on-board LEDs and pin 13
// is preallocated as the Tx line of the the serial link to the motor controller.
{
  EIMSK=0;
  if (pin <2 || pin >9)
  {
	flashErrorLED (2);	//halt proceesing flash RED LED to indicate input parameter error
	return 0xFF;		//will never return
  }
  byte status=0;
  pinMode (pin, INPUT);
  status = digitalRead (pin);
  EIMSK=3;
  return status;
}

//***************************************************************************************************************
void pause (unsigned int time)

// Delays program execution for a specifed time in milliseconds (range 1-65535 msec)
// This function may be called from any function including the interrupt function.
// Input parameter "time" must be an unsigned integer in the range of 1-65535
// pause_loop_cnt is defined at complie time according to complier version (GCC_VERSION). See ArduinoInit.h.
{
	while (time !=0)   // 1 msec delay loop
	{
		for (unsigned int count=0; count <= pause_loop_cnt; count++)
		{NOP;NOP;}
	time--;
	}
return;
}

//***************************************************************************************************************
void version ()

//For Diagnostic Use - Sends the Version number of this ArduinoInit.cpp file to the terminal window.
{
	char i=0;         
	while (ver[i] != 0)
	{
		Serial.print (ver[i]);
		i++;
	}
	Serial.println();
	return;
}