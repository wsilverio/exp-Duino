// pino do sensor
#define sensorPin 2

// armazena a frequencia
float freq;
// flag da interrupcao
volatile bool sensorFlag;
// armazenam ultimo "millis"
unsigned long lastSensorTime, lastSerialTime;

void setup() {
  // configura comunicacao serial
  Serial.begin(9600);
  // configura sensor como entrada
  pinMode(sensorPin, INPUT_PULLUP);
  // configura interrupcao (borda de descida)
  attachInterrupt(digitalPinToInterrupt(sensorPin), sensorISR, FALLING);
  // reset das variaveis
  freq = 0;
  sensorFlag = false;
  lastSensorTime = lastSerialTime = millis();
}

void loop() {

  // se houver acionamento do sensor...
  if (sensorFlag) {
    // captura o tempo atual
    unsigned long timeNow = millis();
    // calcula a frequencia
    freq = 1000.0 / float(timeNow - lastSensorTime);
    // atualiza o tempo
    lastSensorTime = timeNow;
    // reseta a flag
    sensorFlag = false;
  }

  // envia a freq a cada 1000ms = 1s
  if (millis() - lastSerialTime >= 1000) {
    Serial.print(F("freq: "));
    Serial.print(freq);
    Serial.println(F("Hz"));
    // atualiza o tempo
    lastSerialTime = millis();
  }
}

void sensorISR() {
  sensorFlag = true;
}
