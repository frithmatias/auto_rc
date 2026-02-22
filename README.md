# 🚗 Proyecto Auto RC con Arduino Pro Mini

Este proyecto consiste en el desarrollo de un **auto RC autónomo** utilizando un **Arduino Pro Mini**, control de motores, sensores ultrasónicos y distintos periféricos como luces, balizas y bocina.

---

## ⚙️ Hardware utilizado
- Arduino Pro Mini
- Puente H Driver L298N (DC Motor Driver)
- Motores DC JGA25-370 (x2)
- Sensor ultrasónico HC-SR04
- Luces delanteras (LEDs 1Watt con PWM, LM2596 + IRF520)
- Luz de stop trasera (LED digital)
- Balizas (LED digital)
- Bocina (PWM)
- Baterías 18650 x6 2S3P
- Transceptor RF para TX/RX en 433 Mhz
- Chasis RC 1/10 en 3D con suspensión de doble horquilla

---

## 📡 Protocolo de comunicación
El transmisor envía el estado completo del vehículo en cada paquete en formato CSV:
speed,forward,angle,lights,flash,brake,buzzer,blinkLeft,blinkRight,blinkBoth


Ejemplo
255,1,135,0,0,0,0,1,0,0


Este esquema asegura que el receptor siempre tenga el estado completo, incluso si se pierde un paquete intermedio.
## 📌 Asignación de pines

- D2 → Servo dirección (Servo)
- D3 → Luces delanteras (PWM)
- D5 → Motor 2 ENB (PWM)
- D6 → Motor 2 IN4 (Digital)
- D7 → Motor 2 IN3 (Digital)
- D8 → Motor 1 IN2 (Digital)
- D9 → Motor 1 IN1 (Digital)
- D10 → Motor 1 ENA (PWM)
- D11 → Bocina (PWM)
- D12 → Ultrasonido Trig (Digital)
- D13 → Ultrasonido Echo (Digital)
- A0 → Luz de stop trasera (Digital)
- A1 → Luz de reversa (Digital)
- A2 → Blinker izquierda (Digital)
- A3 → Blinker derecha (Digital)
- A4 → RF RX (Digital)
- A5 → RF TX (sin uso)
- A6–A7 → disponibles (Digital/Analógico)

---

## 🧩 Funcionalidades actuales
- Control de dos motores DC (adelante, atrás, frenar, desacelerar).
- Detección de obstáculos con sensor ultrasónico.
- Luces delanteras con control de brillo (PWM).
- Bocina con control de intensidad (PWM).
- Balizas con parpadeo no bloqueante usando millis().
- Luz de stop trasera que se activa al frenar.
- Protocolo CSV para sincronización TX/RX.


---

## 🚀 Próximos pasos
- Implementar suspensión y dirección servo completa.
- Mejorar lógica autónoma (ej. retroceder y girar al detectar obstáculos).
- Documentar esquemas eléctricos y diagramas de conexión.
- Añadir fotos del prototipo y STL de piezas impresas en 3D.
- Publicar ejemplos de transmisor y receptor en el repo.


---

## 📖 Cómo usar
1. Clonar el repositorio:
   ```bash
   git clone https://github.com/frithmatias/auto_rc.git