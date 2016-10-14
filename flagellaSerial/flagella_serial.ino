uint8_t dataHeader = 0xff;
uint8_t nothing = 0xfe;
uint8_t unclickable = 0x00;
uint8_t numberOfData;
uint8_t data;
int flagella = 20;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("OK. Connected.");
  pinMode(flagella, OUTPUT);
  analogWriteResolution(8);
}

void loop() {
    if (Serial.available() > 1) {
        uint8_t recievedByte = Serial.read();
        if ((uint8_t)recievedByte  == dataHeader) { // read first byte and check if it is the beginning of the stream
            delay(10);
            data = Serial.read();
            Serial.println(data);
            Serial.println("----");
        }
    }
    if (data != nothing && data != unclickable) {
      int rate = map(data, 1, 100, 1000, 0);
      Serial.print("rate: ");
      Serial.println(rate);
      digitalWrite(flagella, 1);
      delay(70);
      digitalWrite(flagella, 0);
      delay(rate);
    }
}
