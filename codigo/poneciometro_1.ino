#include <Arduino.h>

const int pinPot = 34;    
const int pinBtn = 18;    
const int pinLED = 2;     

volatile int estado = 0;
volatile int parpadeosCount = 0;
volatile bool ledStatus = false;

hw_timer_t *timer = NULL;

void IRAM_ATTR isrBoton() {
  static uint32_t last_time = 0;
  uint32_t now = millis();
  if (now - last_time > 350) { 
    estado++;
    if (estado > 2) estado = 0;
    
    if (estado == 2) {
      parpadeosCount = 0;
      ledStatus = false; // Empezamos apagado para que el primer cambio sea a encendido
    }
    last_time = now;
  }
}

void IRAM_ATTR onTimer() {
  if (estado == 2) {
    ledStatus = !ledStatus;
    digitalWrite(pinLED, ledStatus);
    
    // Contamos cada vez que el LED se APAGA
    if (ledStatus == false) {
      parpadeosCount++;
      // Ajuste: Cambiamos a 5 para asegurar que el quinto ciclo termine
      if (parpadeosCount >= 5) {
        estado = 0;
        parpadeosCount = 0;
      }
    }
  }
}

void setup() {
  Serial.begin(115200); 
  pinMode(pinLED, OUTPUT);
  pinMode(pinBtn, INPUT_PULLUP);
  
  attachInterrupt(digitalPinToInterrupt(pinBtn), isrBoton, FALLING);

  timer = timerBegin(1000000); 
  timerAttachInterrupt(timer, &onTimer);
  timerAlarm(timer, 1000000, true, 0); 

  Serial.println("--- SISTEMA LISTO (5 PARPADEOS REALES) ---");
}

void loop() {
  static int estadoAnterior = -1;

  if (estado != estadoAnterior) {
    Serial.printf("\n>>> ESTADO ACTUAL: %d\n", estado);
    estadoAnterior = estado;
    
    // Forzamos el modo del pin para limpiar el PWM del estado 1
    pinMode(pinLED, OUTPUT); 
    if(estado == 0) digitalWrite(pinLED, LOW);
  }

  switch (estado) {
    case 0:
      digitalWrite(pinLED, LOW);
      break;

    case 1:
      analogWrite(pinLED, map(analogRead(pinPot), 0, 4095, 0, 255));
      break;

    case 2:
      // El parpadeo lo hace la ISR onTimer
      break;
  }
}
