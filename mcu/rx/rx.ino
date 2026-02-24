#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <Servo.h>
Servo steeringServo;

// 433 MHz
// #include <RH_ASK.h> // RadioHead for 433 Mhz Module
// RH_ASK rfReceiver(2400, 4, 5);  // bitRate, rxPin, txPin


// PWMs
// 03 T2 -> Lights
// 05 T0 -> Motor 1
// 06 T0 -> Motor 2
// 09 T1 -> SERVO
// 10 T1 -> SERVO
// 11 T2 -> MOSI SPI

// Motor 1
int pinServo = 2;   // Servo.h genera la señal con T1 dejan de funcionar 9 y 10
int lightsPin = 3;  // PWM

// Motor 2
int IN1 = 4;
int ENA = 5;  // PWM
int ENB = 6;  // PWM
int IN2 = 7;
int IN3 = 8;
int IN4 = 9;

int buzzerPin = 10;  //PWM

// MOSI 11
// MISO 12
// SCK  13

const byte address[6] = "00001";  // misma dirección que el transmisor
RF24 radio(A0, A1);               // NRF24L01 CE, CSN

int stopPin = A2;
int reversePin = A3;
int blinkLeftPin = A4;
int blinkRightPin = A5;

// UltraSonido (bucar opciones digitales)
int Trig = A6;  // I2C SDA
int Echo = A7;  // I2C SCL

// STATE
int stSpeed = 0;
int stAngle = 90;
bool stReverse = false;
int stLights = 0;  // 0 off 1 low 2 high
bool stLightsSet = false;
bool stFlash = false;
bool stNitro = false;
bool stBrakes = false;  // brakes applied
bool stStop = false;    // lights
bool stBuzzer = false;
bool stBlinkLeft = false;    // activadas o desactivadas
bool stBlinkRight = false;   // activadas o desactivadas
bool stBlinkBoth = false;    // activadas o desactivadas
bool stBlinkStatus = false;  // luz encendida o apagada
int stBlinkSide = 0;

const bool GO_FORWARD = true;
const bool GO_REVERSE = false;
const bool TURN_RIGHT = true;
const bool TURN_LEFT = false;
const int POWER_OFF = 0;
const int POWER_LOW = 150;
const int POWER_HIGH = 255;
const int BLINK_OFF = 0;
const int BLINK_LEFT = 1;
const int BLINK_RIGHT = 2;
const int BLINK_BOTH = 3;


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
  //if (!rfReceiver.init()) {
  //  Serial.println("RF init failed");
  //}

  if (radio.begin()) {
    Serial.println("Hardware encontrado.");
  } else {
    Serial.println("Error de conexión con el módulo RF.");
  }

  radio.openReadingPipe(1, address);  // Configura dirección de recepción

  // radio.setChannel(125);

  // radio.setPALevel(RF24_PA_MIN);
  // radio.setPALevel(RF24_PA_LOW);
  // radio.setPALevel(RF24_PA_HIGH);
  // radio.setPALevel(RF24_PA_MAX);

  // radio.setDataRate(RF24_250KBPS);
  // radio.setDataRate(RF24_1MBPS);
  // radio.setDataRate(RF24_2MBPS);

  radio.startListening();

  // Servo
  steeringServo.attach(pinServo);
  steeringServo.write(90);  // posición inicial (centro)

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
}

void initStatus() {


  setLights(POWER_HIGH);
  lightsStopOn();
  lightsReverseOn();
  buzzerOff();
  delay(1000);
  setLights(POWER_OFF);
  unapplyBrakes();
  lightsReverseOff();
  steeringServo.write(stAngle);
}

void loops() {
  if (radio.available()) {
    char msg[32] = "";
    radio.read(&msg, sizeof(msg));  // lee hasta 32 bytes
    Serial.print("Recibido: ");
    Serial.println(msg);
  }
}

void loop() {

  //uint8_t buf[32];
  //uint8_t buflen = sizeof(buf);

  //if (rfReceiver.recv(buf, &buflen)) {
  if (radio.available()) {

    //buf[buflen] = '\0';  // terminador
    //char message[32];
    //strncpy(message, (char *)buf, buflen);
    //message[buflen] = '\0';
    char msg[32] = "";
    radio.read(&msg, sizeof(msg));  // lee hasta 32 bytes
    Serial.print("Recibido: ");
    Serial.println(msg);

    // Parsear CSV
    int values[9];  // cantidad de campos esperados
    int index = 0;
    char *token = strtok(msg, ",");
    while (token != NULL && index < 9) {
      values[index++] = atoi(token);
      token = strtok(NULL, ",");
    }


    int speed = values[0];        // 0-255
    int direction = values[1];    // servo angle
    bool lights = values[2];      // 0/1
    bool flash = values[3];       // 0/1
    bool nitro = values[4];       // 0/1
    bool brake = values[5];       // 0/1
    bool buzzer = values[6];      // 0/1
    bool blinkLeft = values[7];   // 0/1
    bool blinkRight = values[8];  // 0/1

    // State Managment

    if (direction) {
      int angle = map(direction, 0, 1023, 10, 170);
      turnTo(angle);
    }

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


    if (nitro) {
      if (!stNitro) applyNitro();
    } else {
      if (stNitro) unapplyNitro();
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

    if (!nitro) {

      if (blinkLeft && !blinkRight) {

        if (!stBlinkLeft) {
          setBlink(BLINK_LEFT);
        } else {
          setBlink(BLINK_OFF);
        }
      }

      if (!blinkLeft && blinkRight) {
        if (!stBlinkRight) {
          setBlink(BLINK_RIGHT);
        } else {
          setBlink(BLINK_OFF);
        }
      }

      if (blinkLeft && blinkRight) {
        if (!stBlinkBoth) {
          setBlink(BLINK_BOTH);
        } else {
          setBlink(BLINK_OFF);
        }
      }
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
  autoCenterPreviousMillis = millis();
  if (angle > 88 && angle < 92) {
    stAngle = 90;  // centro fijo
  } else {
    stAngle = angle;
  }
  steeringServo.write(stAngle);
}

void autoCenterCheck() {
  unsigned long currentMillis = millis();

  // Si pasó más de 1 segundo sin comandos de giro
  if (currentMillis - autoCenterPreviousMillis >= autoCenterInterval) {

    autoCenterPreviousMillis = currentMillis;

    if (stAngle > 90) {
      stAngle -= 10;  // se acerca al centro desde la derecha
    } else if (stAngle < 90) {
      stAngle += 10;  // se acerca al centro desde la izquierda
    }

    if (stAngle >= 85 && stAngle < 95) {
      stAngle = 90;  // centro fijo
    }

    steeringServo.write(stAngle);
  }
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

void lightsStopOn() {
  stStop = true;
  digitalWrite(stopPin, HIGH);
}

void lightsStopOff() {
  stStop = false;
  digitalWrite(stopPin, LOW);
}

void applyNitro() {
  stNitro = true;
  Serial.println("APLLY NITRO");
  setBlink(BLINK_BOTH);
}

void unapplyNitro() {
  stNitro = false;
  setBlink(BLINK_OFF);
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

void unapplyBrakes() {
  stBrakes = false;
  lightsStopOff();
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
      buzzerTic();

      switch (stBlinkSide) {
        default:
        case 0:
          digitalWrite(blinkRightPin, LOW);
          digitalWrite(blinkLeftPin, LOW);
          break;
        case 1:
          digitalWrite(blinkRightPin, LOW);
          digitalWrite(blinkLeftPin, stBlinkStatus ? HIGH : LOW);
          break;
        case 2:
          digitalWrite(blinkLeftPin, LOW);
          digitalWrite(blinkRightPin, stBlinkStatus ? HIGH : LOW);
          break;
        case 3:
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

void setBlink(int side) {
  Serial.print("SETBLINK: ");
  Serial.println(side);

  stBlinkSide = side;
  switch (side) {
    case 0:
      stBlinkLeft = false;
      stBlinkRight = false;
      stBlinkBoth = false;
      break;
    case 1:
      stBlinkLeft = true;
      break;
    case 2:
      stBlinkRight = true;
      break;
    case 3:
      stBlinkBoth = true;
      break;
  }
}


void buzzerTic() {
  analogWrite(buzzerPin, 255);
  delay(2);
  analogWrite(buzzerPin, 0);
}

void buzzerOn() {
  stBuzzer = true;
  //   for (int tone = 0; tone <= 255; tone++) {
  //   analogWrite(buzzerPin, tone);
  //   delay(200); // ajustá el delay para controlar la velocidad del barrido
  // }
  analogWrite(buzzerPin, 255);  // Servo quita PWM en Pin 9 y 10
}

void buzzerOff() {
  stBuzzer = false;
  analogWrite(buzzerPin, 0);
}
