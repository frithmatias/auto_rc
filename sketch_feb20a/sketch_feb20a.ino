#include <Servo.h>

// Servo Dirección
Servo steeringServo;
int pinServo = 2; // pin digital para el servo

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
int lights = 3; // pin PWM

// Bocina (PWM)
int horn = 11; // pin PWM

// Balizas (digital)
int hazard = 4;            // pin digital
bool hazardOn = false;     // activadas o desactivadas
bool hazardStatus = false; // luz encendida o apagada
unsigned long previousMillis = 0;
const long interval = 500; // intervalo de parpadeo en ms

// luz de stop trasera
int stop = A0;
int reverse = A1; // luz de reversa

void setup()
{

  Serial.begin(9600);

  // Servo
  steeringServo.attach(pinServo);
  steeringServo.write(90); // posición inicial (centro)

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

  // Balizas
  pinMode(hazard, OUTPUT);

  // Luz Stop
  pinMode(stop, OUTPUT);

  // Luz Reversa
  pinMode(reverse, OUTPUT);

  // Bocina
  pinMode(horn, OUTPUT);
}

void loop()
{

  long distancia = checkDistance();

  if (distancia <= 50)
  {
    // Si hay obstáculo cerca, frena
    brake();
  }

  lightsHazardCheck();
  // Acelerar();
  // Desacelerar();
  // Frenar();
  // Atras();
}

void speedTo(bool forward, int speed)
{

  if (forward)
  {
    lightsStopOff();
    lightsReverseOff();
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }
  else
  {
    lightsReverseOn();
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }

  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void slowDown()
{

  for (int velocidad = 255; velocidad >= 0; velocidad--)
  {
    analogWrite(ENA, velocidad);
    analogWrite(ENB, velocidad);
    delay(20); // ajustá este delay para controlar la rapidez de la aceleración
  }
}

void brake()
{

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

long checkDistance()
{
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);

  long duracion = pulseIn(Echo, HIGH);
  long distancia = duracion * 0.034 / 2; // en cm
  return distancia;
}

void turnTo(int angle)
{
  // ajustá el ángulo según tu montaje
  steeringServo.write(angle);
}

void turnCenter()
{
  steeringServo.write(90);
}

void lightsFrontOn(int power)
{
  analogWrite(lights, power); // 0–255
}

void lightsFrontOff()
{
  analogWrite(lights, 0);
}

void lightsStopOn()
{
  digitalWrite(stop, HIGH);
}

void lightsStopOff()
{
  digitalWrite(stop, LOW);
}

void lightsReverseOn()
{
  digitalWrite(reverse, HIGH);
}

void lightsReverseOff()
{
  digitalWrite(reverse, LOW);
}

void lightsHazardCheck()
{
  if (hazardOn)
  {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval)
    {
      previousMillis = currentMillis;

      // Cambiar estado
      hazardStatus = !hazardStatus;
      digitalWrite(hazard, hazardStatus ? HIGH : LOW);
    }
  }
  else
  {
    // Si balizas están apagadas, aseguramos que el pin quede en LOW
    digitalWrite(hazard, LOW);
    hazardStatus = false;
  }
}

void lightsHazardToggle()
{
  hazardOn = !hazardOn;
}

void hornOn(int volume)
{
  analogWrite(horn, volume); // 0–255
}

void hornOff()
{
  analogWrite(horn, 0);
}
