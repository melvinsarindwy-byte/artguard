// ArtGuard – A1: Teste dos sensores HC-SR04 e LDR (versão ESP32-C6-DevKitC-1)
// Placa no Arduino IDE: "ESP32C6 Dev Module" (core esp32 by Espressif >= 3.0)
// Ligar o cabo USB à porta marcada "UART".
//
// Ligações:
//   HC-SR04: VCC -> 5V | GND -> GND | Trig -> GPIO18
//            Echo -> divisor de tensão (1 kΩ + 2 kΩ) -> GPIO19
//            (!! o Echo dá 5V e o ESP32-C6 só aguenta 3.3V)
//   LDR:     uma perna -> 3V3 | outra perna -> GPIO2 (ADC1_CH2)
//            resistência 10 kΩ entre GPIO2 e GND

const int TRIG_PIN = 18;
const int ECHO_PIN = 19;
const int LDR_PIN  = 2;

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  analogReadResolution(12);           // 0..4095
  analogSetAttenuation(ADC_11db);     // gama completa 0..3.3V
  delay(1000);
  Serial.println("ArtGuard - Teste dos sensores (ESP32-C6)");
}

// Devolve a distância em cm, ou -1 se não houver eco
float lerDistanciaCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duracao = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30 ms (~5 m)
  if (duracao == 0) return -1;
  return duracao * 0.0343 / 2.0;
}

void loop() {
  float distancia = lerDistanciaCm();
  int luz = analogRead(LDR_PIN);      // 0 (escuro) a 4095 (muita luz)

  Serial.print("Distancia: ");
  if (distancia < 0) Serial.print("---");
  else Serial.print(distancia, 1);
  Serial.print(" cm | Luz: ");
  Serial.println(luz);

  delay(500);
}
