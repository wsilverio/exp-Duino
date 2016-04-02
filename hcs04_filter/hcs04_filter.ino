// pinos digitais
#define TRIG  8
#define ECHO  7
// pinos analogicos
#define POT   A0

// outas constantes
#define TEMPOAMOSTRAGEM 250 // minimo 50ms

// variaveis para o sensor
int sensorAtual = 0;
long sensorPulso = 0;

// variaveis para o tempo
long tempoAtual = 0;
long tempoAnterior = 0;

// variaveis para o potenciometro
int potAtual = 0;
int distPot = 0;

void setup() {
  // habilita comunicacao serial
  Serial.begin(115200);

  // configura pinos digitais
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  // escreve na serial
  Serial.println("### START ###\n");

}

void loop() {
  // leitura da distancia
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  sensorPulso = pulseIn(ECHO, HIGH);
  sensorAtual = sensorPulso / 58.2; // conversao para CM

  // leitura do potenciometro
  potAtual = analogRead(POT);
  distPot = map(potAtual, 0, 1023, 0, 100); // converte para 0 a 100 cm

  // escreve os valores
  Serial.print("sensor: "); 
  Serial.println(sensorAtual);
  Serial.print("pot: ");    
  Serial.println(distPot);
  Serial.println();

  // espera o tempo de amostragem
  do {
    tempoAtual = millis();
  } while (tempoAtual - tempoAnterior < TEMPOAMOSTRAGEM);

  // atualiza o tempo
  tempoAnterior = tempoAtual;
}
