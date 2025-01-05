// Load libraries
#include "BluetoothSerial.h"


// Check if Bluetooth configs are enabled
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

// Bluetooth Serial object
BluetoothSerial SerialBT;

//motor control variables
const int cw = 1;
const int ccw = 2;
int motor_dir = 1;
const int halfSeq = 1;
const int fullSeq = 2;
int motorSequence = fullSeq;
int motorSpeed = 4;
int maxSeq = 8;

//flag commands for stepper motor control
bool flagTurn = false;
bool flagIncreaseSpeed = false;
bool flagDecreaseSpeed = false;


// ports used to control the stepper motor
// if your motor rotate to the opposite direction, 
// change the order as {4, 5, 6, 7};
int port[4] = {2, 4, 5, 18};//{4, 5, 6, 7};

// half step sequence of stepper motor control
int halfseq[8][4] = {
  {  LOW, HIGH, HIGH,  LOW},
  {  LOW,  LOW, HIGH,  LOW},
  {  LOW,  LOW, HIGH, HIGH},
  {  LOW,  LOW,  LOW, HIGH},
  { HIGH,  LOW,  LOW, HIGH},
  { HIGH,  LOW,  LOW,  LOW},
  { HIGH, HIGH,  LOW,  LOW},
  {  LOW, HIGH,  LOW,  LOW}
};

// full step sequence of stepper motor control
int fullseq[4][4] = {
  {  LOW, HIGH, HIGH,  LOW},
  {  LOW,  LOW, HIGH, HIGH},
  { HIGH,  LOW,  LOW, HIGH},
  { HIGH, HIGH,  LOW,  LOW}
};

// Handle received and sent messages
String message = "";
char incomingChar;

// Timer: auxiliar variables
unsigned long previousMillis = 0;    // Stores last time stepper command was sent
long stepperStepInterval = 2;         // interval between each step

void setup() {
  Serial.begin(115200);
  
  // Bluetooth device name
  SerialBT.begin("BT_TurnTable");

  //setup ULN2003 driver pins for the stepper
  pinMode(port[0], OUTPUT);
  pinMode(port[1], OUTPUT);
  pinMode(port[2], OUTPUT);
  pinMode(port[3], OUTPUT);
}

void loop() {
  // Read received BT messages (commands from phone)
  if (SerialBT.available()){
    char incomingChar = SerialBT.read();
    if (incomingChar != '\n'){
      message += String(incomingChar);
    }
    else{// end of message when \n arrives
      //echo back the command to phone
      SerialBT.print("Received message: "); SerialBT.println(message);
      message = "";
    }
    Serial.write(incomingChar);  
    // Check received message and control output accordingly
    checkMessage(message);    
  }
   
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > motorSpeed)
  {
    previousMillis = currentMillis;
    if(flagTurn)
      rotate(motor_dir, motorSequence);
  }
}


void checkMessage(String message)
{
  if (message =="turn"){
    flagTurn = true;
  }
  else if (message =="stop"){
    flagTurn = false;
    cutCoilsPower();
  }
  else if (message =="slower"){
    flagDecreaseSpeed = true;
  }
  else if (message =="faster"){
    flagIncreaseSpeed = true;
  }
  else if (message =="dir cw"){
    motor_dir = cw;
    Serial.print("motor_dir: "); Serial.println(motor_dir);
  }
  else if (message =="dir ccw"){
    motor_dir = ccw;
    Serial.print("motor_dir: "); Serial.println(motor_dir);
  }  
  else if (message =="seq half"){
    motorSequence = halfSeq;
    Serial.print("motorSequence: "); Serial.println(motorSequence);
  }
  else if (message =="seq full"){
    motorSequence = fullSeq;
    Serial.print("motorSequence: "); Serial.println(motorSequence);
  }  

  if(flagIncreaseSpeed){
    motorSpeed-=1;
    if(motorSpeed < 1) motorSpeed  = 1;
    flagIncreaseSpeed = false;
    Serial.print("motorSpeed: "); Serial.println(motorSpeed);
  }
  if(flagDecreaseSpeed){
    motorSpeed+=1;
    flagDecreaseSpeed = false;
    Serial.print("motorSpeed: "); Serial.println(motorSpeed);
  }    
}

void cutCoilsPower()
{
  // power cut
  for(int i = 0; i < 4; i++) {
    digitalWrite(port[i], LOW);
  }
}

void rotate(int dir, int sequence) {
  static int phase = 0;
  if(sequence == halfSeq)
  {
    maxSeq = 8;
    //drive the pins
    for(int i = 0; i < 4; i++) {
      digitalWrite(port[i], halfseq[phase][i]);
    }    
  }
  else//full step
  {
    maxSeq = 4;
    //drive the pins
    for(int i = 0; i < 4; i++) {
      digitalWrite(port[i], fullseq[phase][i]);
    }    
  }

  if(dir == cw)//clockwise - cw
  {
    phase++;
    if(phase >= maxSeq)
      phase = 0;
  }
  else if (dir == ccw)//counter clockwise - ccw
  {
    phase--;    
    if(phase < 0)
      phase = maxSeq-1;
  }
  //Serial.println(phase);
}





