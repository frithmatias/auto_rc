
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
int Luces = 3;  // pin PWM

// Bocina (PWM)
int Bocina = 11;  // pin PWM

// Balizas (digital)
int Balizas = 4;  // pin digital
bool balizasOn = false; // activadas o desactivadas
bool balizasStatus = false; // luz encendida o apagada
unsigned long previousMillis = 0;
const long interval = 500; // intervalo de parpadeo en ms

void setup() {

  Serial.begin(9600);

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

  // Luces, bocina y balizas
  pinMode(Luces, OUTPUT);
  pinMode(Bocina, OUTPUT);
  pinMode(Balizas, OUTPUT);
}

void loop() {

  long distancia = medirDistancia();

  if (distancia <= 50) {
    // Si hay obstáculo cerca, frena
    Frenar();
  }

  checkBalizas();
  //Acelerar();
  //Desacelerar();
  //Frenar();
  //Atras();
}

void checkBalizas() {
  if (balizasOn) {
    unsigned long currentMillis = millis();

    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;

      // Cambiar estado
      balizasStatus = !balizasStatus;
      digitalWrite(Balizas, balizasStatus ? HIGH : LOW);
    }
  } else {
    // Si balizas están apagadas, aseguramos que el pin quede en LOW
    digitalWrite(Balizas, LOW);
    balizasStatus = false;
  }
}


void EncenderLuces(int brillo) {
  analogWrite(Luces, brillo);  // 0–255
}

void ApagarLuces() {
  analogWrite(Luces, 0);
}

void ActivarBocina(int intensidad) {
  analogWrite(Bocina, intensidad);  // 0–255
}

void ApagarBocina() {
  analogWrite(Bocina, 0);
}

void ToggleBalizas() {
  balizasOn = !balizasOn;
}

  void Acelerar(int velocidad) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  analogWrite(ENA, velocidad);
  analogWrite(ENB, velocidad);
}

void Reversa(int velocidad) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, velocidad);
  analogWrite(ENB, velocidad);
}

void Desacelerar() {

  // Motor 1 hacia adelante
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  // Motor 2 hacia adelante
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  for (int velocidad = 255; velocidad >= 0; velocidad--) {
    analogWrite(ENA, velocidad);
    analogWrite(ENB, velocidad);
    delay(20);  // ajustá este delay para controlar la rapidez de la aceleración
  }
}

void Frenar() {

  // Motor 1
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);

  // Motor 2
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 0);
}

long medirDistancia() {
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);

  long duracion = pulseIn(Echo, HIGH);
  long distancia = duracion * 0.034 / 2;  // en cm
  return distancia;
}
