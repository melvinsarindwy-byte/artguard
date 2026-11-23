// ArtGuard – A1: Medição rápida (ESP32-C6)
// Como usar: abrir o Serial Monitor (115200), colocar o objeto à distância
// pretendida e carregar em ENTER (Send). O programa faz 5 leituras e mostra a média.
// Ligações iguais ao teste_sensores_c6: Trig 18, Echo 19, LDR 2.

const int TRIG_PIN = 18;
const int ECHO_PIN = 19;
const int LDR_PIN  = 2;
const int N = 5;
int numeroTeste = 0;

float lerDistanciaCm() {
  digitalWrite(TRIG_PIN, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long d = pulseIn(ECHO_PIN, HIGH, 30000);
  if (d == 0) return -1;
  return d * 0.0343 / 2.0;
}

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  delay(1000);
  Serial.println("ArtGuard - Medicao rapida. Carregue ENTER para medir.");
  Serial.println("Teste,Dist1,Dist2,Dist3,Dist4,Dist5,DistMedia,Luz1,Luz2,Luz3,Luz4,Luz5,LuzMedia");
}

void loop() {
  if (Serial.available()) {
    while (Serial.available()) Serial.read();   // limpar entrada
    numeroTeste++;
    float somaD = 0; int validos = 0; long somaL = 0;
    float dist[N]; int luz[N];
    for (int i = 0; i < N; i++) {
      dist[i] = lerDistanciaCm();
      luz[i] = analogRead(LDR_PIN);
      if (dist[i] > 0) { somaD += dist[i]; validos++; }
      somaL += luz[i];
      delay(200);
    }
    Serial.print(numeroTeste);
    for (int i = 0; i < N; i++) { Serial.print(","); Serial.print(dist[i], 1); }
    Serial.print(",");
    if (validos > 0) Serial.print(somaD / validos, 1); else Serial.print("---");
    for (int i = 0; i < N; i++) { Serial.print(","); Serial.print(luz[i]); }
    Serial.print(",");
    Serial.println(somaL / N);
  }
}
