uint8_t dataHeader = 0xff;
uint8_t nothing = 0xfe;
uint8_t unclickable = 0x00;
uint8_t numberOfData;
uint8_t data;
int flagella = 20;
int motor1APin = 17;     // H-bridge leg 1
int motor2APin = 23;     // H-bridge leg 2

enum dir {
  FORWARD,
  BACKWARD,
  STOP
};

void setDirection(dir newDir);

void setup() {
  Serial.begin(19200);
  while (!Serial) {
    // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("OK. Connected.");
  pinMode(flagella, OUTPUT);
  pinMode(motor1APin, OUTPUT);
  pinMode(motor2APin, OUTPUT);
  pinMode(flagella, OUTPUT); 
  setDirection(FORWARD);
  Serial.println(unclickable);
}


void setDirection(dir newDir) {
  if (newDir == FORWARD) {
    Serial.println("Forward");
    digitalWrite(motor1APin, LOW);
    digitalWrite(motor2APin, HIGH);
  } else if(newDir == BACKWARD) {
    Serial.println("Backward");
    digitalWrite(motor1APin, HIGH);
    digitalWrite(motor2APin, LOW);
  } else {
    digitalWrite(motor1APin, HIGH);
    digitalWrite(motor2APin, HIGH);
  }
}

void loop() {
    if (Serial.available() > 1) {
        uint8_t recievedByte = Serial.read();
        if ((uint8_t)recievedByte  == dataHeader) { // read first byte and check if it is the beginning of the stream
            delay(10);
            data = Serial.read();
//            Serial.println(data);
//            Serial.println("----");
        }
    }
//    Serial.println(data);
    if (data == unclickable || data == nothing) {
      setDirection(STOP);
    } else {
      setDirection(FORWARD);
      int rate = map(data, 1, 120, 50, 200);
      analogWrite(flagella, rate);
      delay(50);
    }
    
}
