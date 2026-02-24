// #include <RH_ASK.h>
#include <SPI.h>  // requerido por RadioHead
#include <nRF24L01.h>
#include <RF24.h>

// RH_ASK rfDriver; // usa pines digitales por defecto: TX en pin 12
// RH_ASK rfDriver(2400, 10, 11);  // bitRate, pinRx, pinTx

const byte address[6] = "00001";  // Dirección de comunicación
RF24 radio(A0, A1);               // CE, CSN
int pinSpeed = A2;                // Y axis 0-
int pinAngle = A3;                //X axis

int pinLights = 2;
int pinFlash = 3;
int pinNitro = 4;
int pinBrake = 5;
int pinBuzzer = 6;
int pinBlinkLeft = 7;
int pinBlinkRight = 8;

int speed = 255;  // 0-255
int forward = 1;  // 0=reverse, 1=forward
int angle = 45;   // servo angle

int lights = 150;  // 0-255
int flash = 0;     // 0=off, 1=on
int nitro = 0;
int brake = 0;     // 0=off, 1=on
int buzzer = 0;    // 0=off, 1=on

int blinkLeft = 0;   // 0=off, 1=on
int blinkRight = 0;  // 0=off, 1=on



void setup() {

  Serial.begin(9600);

  if (radio.begin()) {
    Serial.println("Hardware encontrado.");
  } else {
    Serial.println("Hardware no encontrado. Revisa cables y alimentación.");
  }

  radio.openWritingPipe(address);  // Configura dirección de transmisión
  // radio.setChannel(125);

  // radio.setPALevel(RF24_PA_MIN);
  // radio.setPALevel(RF24_PA_LOW);
  // radio.setPALevel(RF24_PA_HIGH);
  // radio.setPALevel(RF24_PA_MAX);

  // radio.setDataRate(RF24_250KBPS);
  // radio.setDataRate(RF24_1MBPS);
  // radio.setDataRate(RF24_2MBPS);
  radio.stopListening();

  pinMode(pinLights, INPUT_PULLUP);
  pinMode(pinFlash, INPUT_PULLUP);
  pinMode(pinNitro, INPUT_PULLUP);
  pinMode(pinBrake, INPUT_PULLUP);
  pinMode(pinBuzzer, INPUT_PULLUP);
  pinMode(pinBlinkLeft, INPUT_PULLUP);
  pinMode(pinBlinkRight, INPUT_PULLUP);

  // if (!rfDriver.init()) {
  //   Serial.println("RF init failed");
  // }
}

void loop() {

delay(500);
  int speed = analogRead(pinSpeed);  // 0–1023
  int angle = analogRead(pinAngle);  // 0–1023

  lights = !digitalRead(pinLights);
  flash = !digitalRead(pinFlash);
  nitro = !digitalRead(pinNitro);
  brake = !digitalRead(pinBrake);
  buzzer = !digitalRead(pinBuzzer);

  blinkLeft = !digitalRead(pinBlinkLeft);
  blinkRight = !digitalRead(pinBlinkRight);
  
  char state[50];  // buffer de salida, ajustá tamaño según necesidad

  sprintf(state, "%d,%d,%d,%d,%d,%d,%d,%d,%d",
          speed,        //1023
          angle,        //1023
          lights,       // 0-1
          flash,        // 0-1
          nitro,        // 0-1
          brake,        // 0-1
          buzzer,       // 0-1
          blinkLeft,    // 0-1
          blinkRight);  // 0-1

  // Ahora 'state' contiene la cadena formateada
  sendMessage(state);
}

void sendMessage(const char* state) {
  // rfDriver.send((uint8_t*)state, strlen(state));
  // rfDriver.waitPacketSent();
  // Serial.print("Enviado: ");
  // Serial.println(state);
  bool ok = radio.write(state, strlen(state) + 1);

  if (ok) {
    Serial.print("Mensaje enviado: ");
    Serial.println(state);
  } else {
    Serial.println("Error al enviar");
  }
}
