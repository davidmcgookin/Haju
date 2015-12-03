/**
 * Haju Source Code
 * Version 1.0
 * David McGookin & Dariella Escobar
 * david.mcgookin@aalto.fi
 * 
 * (c) Aalto University 2015
 * 
 * This software is licensed under the GNU GPL v3 license
 * http://www.gnu.org/copyleft/gpl.html
 * 
 * USAGE
 * This code should be uploaded onto the Arduino.
 * Commands are sent over a serial connection in the form
 * <CMD><BIN>X
 * Where <CMD> is the command code (currently d to dispense a smell and r to rotate the fragrance bin without dispensing a smell)
 * <BIN> is the fragrance bin <CMD> applies to. Bins are numbered in accending order clockwise. Starting at 1 (the largest bin)
 * X is the terminator for the command
 * E.g. d3X (will dispense the scent in bin 3.
 * 
 */


#include <Bounce2.h>


//Codes used to start and stop the rotational servo. Note these are codes not angles
#define ROTATIONAL_SERVO_START 80
#define ROTATIONAL_SERVO_STOP 90

//max command length
#define CMD_LENGTH 3
//the terminator command
#define CMD_TERMINATOR 'x'


#include <Servo.h>    // include Servo library
#include <Bounce2.h>
#include <stdlib.h>

Servo piezoServo;     // create servo object to control small servo 
Servo rotationalServo;       // create servo object to control big servo 

//holds the last time we checked for the limitSwitch. Doing this to avoid calling delay. 
unsigned long lastMilliSecCheck;
int piezoPinOut =3;
int limitSwitchPinIn = 9;

int commandCode;

//The last fragrance bin that we passed by
int lastFragranceBin;

//the number of the bin we are searching for. 
int targetBin;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  piezoServo.attach(10);
  rotationalServo.attach(11);
  pinMode(piezoPinOut, OUTPUT);
  pinMode(limitSwitchPinIn, INPUT);


  //set the piezoServo
  piezoServo.write(0);


  //setup the logging to know what bin we are at.
  lastFragranceBin=1;// set to be water initially

}

void loop() {
  // put your main code here, to run repeatedly:
  //int state =  false;
  //char inputString[2];
  char cmd;
  int binIndex;

   if (readCommand(&cmd, &binIndex)) { // If data is available to read, get the command char
    Serial.println("HAVE CMD");
    switch(cmd){


    case 't': //test routine
      digitalWrite(piezoPinOut, HIGH);
      delay (5000);
      digitalWrite(piezoPinOut, LOW);
      break;
    case 'r':
      //reset the smell device
      lastFragranceBin = binIndex;
      locateFragranceBin(1);
      break;


    case 'd':
      if(binIndex > 1 && binIndex < 8){ // Scents are numberd. 0 is the water 1..5 are available for scents
         //targetBin = commandCode;
        locateFragranceBin(binIndex); //go to the bin with the desired smell
     
        Serial.println("DISPENSING VAPOUR");
        delay(3000);
        dispenseVapour();
        locateFragranceBin(1); //go to the water bin
      } 
      break;

    default:
      Serial.println("Invalid Command Provided");
      break;
  }
}
}

  

/* Causes the system to release the vapour of the currently selected reservoir
 
 */
 void dispenseVapour(){
     piezoServo.write(90);            // small servo motor rotates down
      digitalWrite (piezoPinOut, HIGH);  // turn piezo ON
       delay (5000);                   // spray out water to clean piezo
                        
       digitalWrite(piezoPinOut, LOW);    // turn piezo OFF
        piezoServo.write(0);           // small servo motor rotates back up
       delay (1000);                   // wait one second
  
  }


void locateFragranceBin(int binID){
   //start the servo running
        rotationalServo.write(80);
       delay (200);  
      do{
        bool switchActivated  = checkSwitchState();
      
       if(switchActivated){
          updateLastFragranceBin();
          Serial.println("SWITCH ACTIVATED");
        Serial.print("Last Seen Fragrance Bin was: ");
        Serial.println(lastFragranceBin);
       }
        }while(binID != lastFragranceBin);


  rotationalServo.write(90);
  
}


  void updateLastFragranceBin(){
       lastFragranceBin = lastFragranceBin+1;
       if(lastFragranceBin == 8){ //We have 7 bins numbered 0..6. Reset if we get to 7.
          lastFragranceBin = 1;
        }
       }
  
bool checkSwitchState(){
  bool currentSwitchState = digitalRead(limitSwitchPinIn);
  Serial.print(currentSwitchState);
  
  unsigned long  currentTime = millis();
  unsigned long deltaSinceLastActive = currentTime - lastMilliSecCheck;
  
  if (currentSwitchState == false && deltaSinceLastActive > 300){ //we put a check on the returning of true to avoid the switch staying active between calls
                                                                  //600 is hacked but seems to be a good number
                                                                  //the false here is the way we wired the circuit. It is true when open and false when closed. But returns true when activated and false otherwise!
                                                                    
      Serial.println(deltaSinceLastActive);
      lastMilliSecCheck = currentTime;
      return true;
  }else{
    return false;
  }
  }


//reads a command from the serial port. Returns false if there is no valid command or true if there is.
//Iff true, cmd and fragranceBin are filled in
 bool readCommand(char *cmd, int*fragranceBin){
  bool haveValidCommand = false;
  int charsRead = 0;
  bool haveCommandTerminator = false;
  char commandStr[CMD_LENGTH];
 
  for(int i = 0; i < CMD_LENGTH; i++){
   commandStr[i] = '?';
  }
 
  if(Serial.available()>=3){ //there are 3 characters in a command
    while(!haveCommandTerminator && charsRead < CMD_LENGTH){
      commandStr[charsRead] = char(Serial.read());
      //Serial.print(commandStr);
      if(commandStr[charsRead] == CMD_TERMINATOR){
      
        //We read the terminator command
        haveCommandTerminator = true;
      }
      charsRead++;
        
    }

  if(haveCommandTerminator && charsRead == CMD_LENGTH){ // We have a valid command
    haveValidCommand = true;
    *cmd = commandStr[0];
    *fragranceBin = commandStr[1] - '0';
  }
  }
  return haveValidCommand; 
  
 }


