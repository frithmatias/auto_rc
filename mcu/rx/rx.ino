#include <SPI.h>  // requerido por RadioHead
#include <RH_ASK.h>

RH_ASK rfReceiver(2400, 4, 5);  // bitRate, rxPin, txPin

// Servo Dirección
//#include <Servo.h>
//Servo steeringServo;



const int POWER_OFF = 0;
const int POWER_LOW = 50;
const int POWER_HIGH = 255;

int position = 90;

#define TURN_RIGHT true
#define TURN_LEFT false
#define GO_FORWARD true
#define GO_REVERSE false
#define BLINK_LEFT 0
#define BLINK_RIGHT 1
#define BLINK_BOTH 2

int reversePin = A1;
int stopPin = A0;
int blinkLeftPin = A2;
int blinkRightPin = A3;  // pin digital

int pinServo = 2;  // pin digital para el servo

// Motor 1
int ENA = 10;
int IN1 = 9;
int IN2 = 8;

// Motor 2
int ENB = 5;
int IN3 = 7;
int IN4 = 6;

int lightsPin = 3;   // pin PWM
int buzzerPin = 11;  // pin PWM

// UltraSonido
int Trig = 12;
int Echo = 13;

// FLAGS
int stSpeed = 0;
bool stForward = true;
int stAngle = 0;

int stLights = 0;  // 0 off 1 low 2 high
bool stLightsSet = false;
bool stFlash = false;
bool stBrakes = false;  // apply brakes
bool stStop = false;    // STOP lights
bool stReverse = false;
bool stBuzzer = false;

bool stBlinkLeft = false;   // activadas o desactivadas
bool stBlinkRight = false;  // activadas o desactivadas
bool stBlinkBoth = false;   // activadas o desactivadas

bool stBlinkStatus = false;  // luz encendida o apagada
int stBlinkSide = 0;

unsigned long anglePreviousMillis = 0;
const long angleInterval = 100;  // intervalo para actualizar la posición al recibir instrucciones para doblar

unsigned long autoCenterPreviousMillis = 0;
const long autoCenterInterval = 100;  // intervalo para enderezar al no recibir instrucciones para doblar

unsigned long flashPreviousMillis = 0;
const long flashInterval = 500;  // intervalo para apagar el guiño


unsigned long blinkPreviousMillis = 0;
const long blinkInterval = 500;  // intervalo de parpadeo en ms

void setup() {

  // RF Communication
  Serial.begin(9600);
  //rfReceiver.begin(1200);  // velocidad típica para 433 MHz
  if (!rfReceiver.init()) {
    Serial.println("RF init failed");
  }

  // Servo
  //steeringServo.attach(pinServo);
  //steeringServo.write(90);  // posición inicial (centro)

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(lightsPin, OUTPUT);
  pinMode(stopPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);

  pinMode(blinkLeftPin, OUTPUT);
  pinMode(blinkRightPin, OUTPUT);
  pinMode(reversePin, OUTPUT);
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);

  initStatus();
  //steeringServo.write(position);
}

void initStatus() {


  setLights(POWER_LOW);
  lightsStopOn();
  lightsReverseOn();
  delay(1000);
  setLights(POWER_OFF);
  unapplyBrakes();
  lightsReverseOff();
}

void loop() {
  uint8_t buf[32];
  uint8_t buflen = sizeof(buf);

  if (rfReceiver.recv(buf, &buflen)) {

    buf[buflen] = '\0';  // terminador

    char message[32];
    strncpy(message, (char *)buf, buflen);
    message[buflen] = '\0';

    Serial.println(message);

    // Parsear CSV
    int values[10];  // cantidad de campos esperados
    int index = 0;
    char *token = strtok(message, ",");
    while (token != NULL && index < 10) {
      values[index++] = atoi(token);
      token = strtok(NULL, ",");
    }

    // Asignar variables
    int speed = values[0];     // 0-255
    bool forward = values[1];  // 0=reverse, 1=forward
    int angle = values[2];     // servo angle

    bool lights = values[3];  // 0/1
    bool flash = values[4];   // 0/1
    bool brake = values[5];   // 0/1
    bool buzzer = values[6];  // 0/1

    bool blinkLeft = values[7];   // 0/1
    bool blinkRight = values[8];  // 0/1
    bool blinkBoth = values[9];   // 0/1

    if (lights) {

      if (!stLightsSet) {

        stLightsSet = true;

        if (stLights == 0) {
          setLights(POWER_LOW);
        }

        else if (stLights == 1) {
          setLights(POWER_HIGH);
        }

        else if (stLights == 2) {
          setLights(POWER_OFF);
        }
      }
    } else {
      if (stLightsSet) {
        stLightsSet = false;
      }
    }

    if (flash) {
      analogWrite(lightsPin, POWER_HIGH);
    } else {

      if (stLights == 0) {
        analogWrite(lightsPin, POWER_OFF);  // 0–255
      }

      else if (stLights == 1) {
        analogWrite(lightsPin, POWER_LOW);  // 0–255
      }

      else if (stLights == 2) {
        analogWrite(lightsPin, POWER_HIGH);  // 0–255
      }
    }

    if (brake) {
      if (!stBrakes) applyBrakes();
    } else {
      if (stBrakes) unapplyBrakes();
    }

    if (buzzer) {
      if (!stBuzzer) buzzerOn();
    } else {
      if (stBuzzer) buzzerOff();
    }

    if (blinkLeft) {
      if (!stBlinkLeft) lightsBlinkingOn(BLINK_LEFT);
    } else {
      if (stBlinkLeft) lightsBlinkingOff();
    }

    if (blinkRight) {
      if (!stBlinkRight) lightsBlinkingOn(BLINK_RIGHT);
    } else {
      if (stBlinkRight) lightsBlinkingOff();
    }

    if (blinkBoth) {
      if (!stBlinkBoth) lightsBlinkingOn(BLINK_BOTH);
    } else {
      if (stBlinkBoth) lightsBlinkingOff();
    }
  }

  lightsBlinkingCheck();
}

void speedTo(bool forward, int speed) {

  stBrakes = false;
  if (forward) {
    unapplyBrakes();
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

void applyBrakes() {
  stBrakes = true;
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

void turnTo(boolean side) {

  autoCenterPreviousMillis = millis();

  if (millis() - anglePreviousMillis >= angleInterval) {
    anglePreviousMillis = millis();
    if (side == TURN_RIGHT) {
      // Giro a la derecha
      if (stAngle < 135) {
        stAngle += 10;
        //steeringServo.write(angle);
      }
    } else if (side == TURN_LEFT) {
      if (stAngle > 45) {
        stAngle -= 10;
        //steeringServo.write(angle);
      }
    }
  }
}

void autoCenterCheck() {
  unsigned long currentMillis = millis();

  // Si pasó más de 1 segundo sin comandos de giro
  if (currentMillis - autoCenterPreviousMillis >= autoCenterInterval) {
    autoCenterPreviousMillis = currentMillis;

    if (position > 90) {
      position -= 10;  // se acerca al centro desde la derecha
      if (position < 90) position = 90;
      //steeringServo.write(position);
    } else if (position < 90) {
      position += 10;  // se acerca al centro desde la izquierda
      if (position > 90) position = 90;
      //steeringServo.write(position);
    }
  }
}

void turnCenter() {
  position = 90;
  //steeringServo.write(90);
}

void setLights(int power) {

  switch (power) {
    case POWER_OFF:
      stLights = 0;
      break;
    case POWER_LOW:
      stLights = 1;
      break;
    case POWER_HIGH:
      stLights = 2;
      break;
  }
  Serial.print("SET STATUS LIGHTS: ");
  Serial.println(stLights);

  analogWrite(lightsPin, power);  // 0–255
}

void lightsFlashOn() {
  stFlash = true;
  analogWrite(lightsPin, 255);  // enciende la luz a máxima potencia
}

void lightsFlashCheck() {
  if (stFlash) {
    unsigned long currentMillis = millis();

    if (currentMillis - flashPreviousMillis >= flashInterval) {
      flashPreviousMillis = currentMillis;

      if (stLights == 1) {
        setLights(POWER_LOW);
      } else {
        setLights(POWER_OFF);
      }
    }
  }
}

void lightsFlashOff() {
  stFlash = false;
  analogWrite(lightsPin, LOW);
}

void lightsStopOn() {
  stStop = true;
  digitalWrite(stopPin, HIGH);
}

void unapplyBrakes() {
  stBrakes = false;
  stStop = false;
  digitalWrite(stopPin, LOW);
}

void lightsReverseOn() {
  stReverse = true;
  digitalWrite(reversePin, HIGH);
}

void lightsReverseOff() {
  stReverse = false;
  digitalWrite(reversePin, LOW);
}

void lightsBlinkingCheck() {
  if (stBlinkLeft || stBlinkRight || stBlinkBoth) {
    unsigned long currentMillis = millis();

    if (currentMillis - blinkPreviousMillis >= blinkInterval) {
      blinkPreviousMillis = currentMillis;

      // Cambiar estado
      stBlinkStatus = !stBlinkStatus;

      switch (stBlinkSide) {
        default:
        case 0:
          digitalWrite(blinkRightPin, LOW);
          digitalWrite(blinkLeftPin, stBlinkStatus ? HIGH : LOW);
          break;
        case 1:
          digitalWrite(blinkLeftPin, LOW);
          digitalWrite(blinkRightPin, stBlinkStatus ? HIGH : LOW);
          break;
        case 2:
          digitalWrite(blinkLeftPin, stBlinkStatus ? HIGH : LOW);
          digitalWrite(blinkRightPin, stBlinkStatus ? HIGH : LOW);
          break;
      }
    }
  } else {
    digitalWrite(blinkLeftPin, LOW);
    digitalWrite(blinkRightPin, LOW);
  }
}

void lightsBlinkingOn(int side) {
  stBlinkSide = side;
  switch (side) {
    case 0:
      stBlinkLeft = true;
      break;
    case 1:
      stBlinkRight = true;
      break;
    case 2:
      stBlinkBoth = true;
      break;
  }
}

void lightsBlinkingOff() {
  stBlinkLeft = false;
  stBlinkRight = false;
  stBlinkBoth = false;
}

void buzzerOn() {
  stBuzzer = true;
  //   for (int tone = 0; tone <= 255; tone++) {
  //   analogWrite(buzzerPin, tone);
  //   delay(200); // ajustá el delay para controlar la velocidad del barrido
  // }
  analogWrite(buzzerPin, 30);
}

void buzzerOff() {
  stBuzzer = false;
  analogWrite(buzzerPin, 0);
}
