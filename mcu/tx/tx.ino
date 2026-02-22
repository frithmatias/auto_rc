#include <RH_ASK.h>
#include <SPI.h>  // requerido por RadioHead

//RH_ASK rfDriver; // usa pines digitales por defecto: TX en pin 12
RH_ASK rfDriver(2400, 10, 11);  // bitRate, rxPin, txPin
char* systemState = "";

int speedPin = A2;  // Y axis 0-
int anglePin = A3;  //X axis
int lightsPin = 2;
int flashPin = 3;
int brakePin = 4;
int buzzerPin = 5;
int blinkLeftPin = 7;
int blinkRightPin = 8;
int blinkBothPin = 9;

int speed = 255;     // 0-255
int forward = 1;     // 0=reverse, 1=forward
int angle = 45;      // servo angle

int lights = 150;    // 0-255
int flash = 0;       // 0=off, 1=on
int brake = 0;       // 0=off, 1=on
int buzzer = 0;        // 0=off, 1=on

int blinkLeft = 0;   // 0=off, 1=on
int blinkRight = 0;  // 0=off, 1=on
int blinkBoth = 0;   // 0=off, 1=on



void setup() {

  pinMode(speedPin, INPUT);
  pinMode(anglePin, INPUT);
  
  pinMode(lightsPin, INPUT_PULLUP);
  pinMode(flashPin, INPUT_PULLUP);
  pinMode(brakePin, INPUT_PULLUP);
  pinMode(buzzerPin, INPUT_PULLUP);
  
  pinMode(blinkLeftPin, INPUT_PULLUP);
  pinMode(blinkRightPin, INPUT_PULLUP);
  pinMode(blinkBothPin, INPUT_PULLUP);

  Serial.begin(9600);
  if (!rfDriver.init()) {
    Serial.println("RF init failed");
  }
}

void loop() {

  speed = 255;
  forward = 1;
  angle = 135;
  
  lights = !digitalRead(lightsPin);
  flash = !digitalRead(flashPin);
  brake = !digitalRead(brakePin);
  buzzer = !digitalRead(buzzerPin);

  blinkLeft = !digitalRead(blinkLeftPin);
  blinkRight = !digitalRead(blinkRightPin);
  blinkBoth = !digitalRead(blinkBothPin);

  String state =
    
    String(speed) + "," + 
    String(forward) + "," + 
    String(angle) + "," + 
    
    String(lights) + "," + 
    String(flash) + "," + 
    String(brake) + "," + 
    String(buzzer) + "," +
    
    String(blinkLeft) + "," + 
    String(blinkRight) + "," + 
    String(blinkBoth);

    sendMessage(state.c_str());

}

void sendMessage(const char* state) {
  rfDriver.send((uint8_t*)state, strlen(state));
  rfDriver.waitPacketSent();
  Serial.print("Enviado: ");
  Serial.println(state);
}
