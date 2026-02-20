#include <Servo.h>
#include <SPI.h>  // requerido por RadioHead
#include <SoftwareSerial.h>
SoftwareSerial rfReceiver(4, 5);  // RX, TX (solo usás RX en el receptor)

// Servo Dirección
Servo steeringServo;
int pinServo = 2;  // pin digital para el servo

// Motor 1
int ENA = 10;
int IN1 = 9;
int IN2 = 8;

// Motor 2
int ENB = 5;
int IN3 = 7;
int IN4 = 6;

// UltraSonido
int Trig = 12;
int Echo = 13;

// Luces delanteras (PWM)
int lights = 3;         // pin PWM
bool lightsOn = false;  // activadas o desactivadas
const int LIGHTS_DEFAULT_POWER = 150;
unsigned long flashPreviousMillis = 0;
const long flashInterval = 100;  // intervalo para apagar el guiño


// int lightsHigh = 4; // pin reservado para flash

// Bocina (PWM)
int horn = 11;  // pin PWM

// luz de stop trasera
int stop = A0;
int reverse = A1;  // luz de reversa

// Balizas (digital)

int blink_left = A2;
int blink_right = A3;  // pin digital

#define BLINK_BOTH 0
#define BLINK_LEFT 1
#define BLINK_RIGHT 2

bool blinkOn = false;      // activadas o desactivadas
bool blinkStatus = false;  // luz encendida o apagada
int blink_side = 0;
unsigned long blinkPreviousMillis = 0;
const long blinkInterval = 500;  // intervalo de parpadeo en ms

void setup() {

  // RF Communication
  Serial.begin(9600);
  rfReceiver.begin(2400);  // velocidad típica para 433 MHz

  // Servo
  steeringServo.attach(pinServo);
  steeringServo.write(90);  // posición inicial (centro)

  // Motor 1
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  // Motor 2
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Ultrasonido
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);

  // Luces
  pinMode(lights, OUTPUT);

  // Giro Izquierda
  pinMode(blink_left, OUTPUT);

  // Giro Derecha
  pinMode(blink_right, OUTPUT);

  // Bocina
  pinMode(horn, OUTPUT);

  // Luz Stop
  pinMode(stop, OUTPUT);

  // Luz Reversa
  pinMode(reverse, OUTPUT);
}

void loop() {

  if (rfReceiver.available()) {
    String command = rfReceiver.readStringUntil('\n');
    Serial.println("Recibido: " + command);

    if (command == "FWD") speedTo(true, 200);
    else if (command == "REV") speedTo(false, 200);
    else if (command == "FLASH") lightsFlash();
    else if (command == "BLINK_LEFT") lightsBlinkingToggle(BLINK_LEFT);
    else if (command == "BLINK_RIGHT") lightsBlinkingToggle(BLINK_RIGHT);
    else if (command == "BLINK_BOTH") lightsBlinkingToggle(BLINK_BOTH);
    else if (command == "HORN") hornOn(255);
    else if (command == "HORN_OFF") hornOff();
  }



  long distance = checkDistance();

  if (distance <= 50) {
    // Si hay obstáculo cerca, frena
    brake();
  }

  lightsBlinkingCheck();
  lightsFlashCheck();
  // Acelerar();
  // Desacelerar();
  // Frenar();
  // Atras();
}

void speedTo(bool forward, int speed) {

  if (forward) {
    lightsStopOff();
    lightsReverseOff();
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  } else {
    lightsReverseOn();
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }

  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void slowDown() {

  for (int speed = 255; speed >= 0; speed--) {
    analogWrite(ENA, speed);
    analogWrite(ENB, speed);
    delay(20);  // ajustá este delay para controlar la rapidez de la aceleración
  }
}

void brake() {

  // Luz de stop
  lightsStopOn();

  // Motor 1
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);

  // Motor 2
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 0);
}

long checkDistance() {
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);

  long duration = pulseIn(Echo, HIGH);
  long distance = duration * 0.034 / 2;  // en cm
  return distance;
}

void turnTo(int angle) {
  // ajustá el ángulo según tu montaje
  steeringServo.write(angle);
}

void turnCenter() {
  steeringServo.write(90);
}

void lightsFrontOn(int power = LIGHTS_DEFAULT_POWER) {
  lightsOn = true;
  analogWrite(lights, power);  // 0–255
}

void lightsFrontOff() {
  lightsOn = false;
  analogWrite(lights, 0);
}


void lightsFlashCheck() {
  unsigned long currentMillis = millis();
  if (currentMillis - flashPreviousMillis >= flashInterval) {
    if (lightsOn) {
      lightsFrontOn();  // apaga el guiño y deja el brillo en 150 por defecto
    }

    else {
      lightsFrontOff();
    }
  }
}

void lightsFlash() {
  flashPreviousMillis = millis();  // registra el momento del guiño
  analogWrite(lights, 255);        // enciende la luz a máxima potencia
}

void lightsStopOn() {
  digitalWrite(stop, HIGH);
}

void lightsStopOff() {
  digitalWrite(stop, LOW);
}

void lightsReverseOn() {
  digitalWrite(reverse, HIGH);
}

void lightsReverseOff() {
  digitalWrite(reverse, LOW);
}

void lightsBlinkingCheck() {
  if (blinkOn) {
    unsigned long currentMillis = millis();

    if (currentMillis - blinkPreviousMillis >= blinkInterval) {
      blinkPreviousMillis = currentMillis;

      // Cambiar estado
      blinkStatus = !blinkStatus;

      switch (blink_side) {
        default:
        case 0:
          digitalWrite(blink_left, blinkStatus ? HIGH : LOW);
          digitalWrite(blink_right, blinkStatus ? HIGH : LOW);
          break;
        case 1:
          digitalWrite(blink_left, blinkStatus ? HIGH : LOW);
          break;
        case 2:
          digitalWrite(blink_right, blinkStatus ? HIGH : LOW);
          break;
      }
    }
  }

  else {
    // Si balizas están apagadas, aseguramos que el pin quede en LOW
    digitalWrite(blink_left, LOW);
    digitalWrite(blink_right, LOW);
    blinkStatus = false;
  }
}

void lightsBlinkingToggle(int side) {
  blink_side = side;
  blinkOn = !blinkOn;
}

void hornOn(int volume) {
  analogWrite(horn, volume);  // 0–255
}

void hornOff() {
  analogWrite(horn, 0);
}
