// ArtGuard – A2: Control logic and alerts (ESP32-C6-DevKitC-1)
// Board: "ESP32C6 Dev Module" (esp32 by Espressif >= 3.0) – USB cable on the "UART" port
//
// Wiring (A1, unchanged):
//   HC-SR04: VCC -> 3V3 | GND -> GND | Trig -> GPIO18 | Echo -> 1 kOhm -> GPIO19
//   LDR:     3V3 -> LDR -> GPIO2 -> 10 kOhm -> GND
// Wiring (A2, step 2 – optional for now, the code works without them):
//   Green LED  -> GPIO0  (through a 220–1k resistor to GND)
//   Yellow LED -> GPIO1  (through a 220–1k resistor to GND)
//   Red LED    -> GPIO3  (through a 220–1k resistor to GND)
//   Buzzer (+) -> GPIO21 | Buzzer (-) -> GND
// The on-board RGB LED (GPIO8) also shows the state: green / yellow / red.

// ---------- Pins ----------
const int TRIG_PIN   = 18;
const int ECHO_PIN   = 19;
const int LDR_PIN    = 2;
const int LED_GREEN  = 0;
const int LED_YELLOW = 1;
const int LED_RED    = 3;
const int BUZZER_PIN = 21;

// ---------- Thresholds (based on the A1 test log) ----------
const float DIST_WARNING = 30.0;   // cm  -> closer than this = WARNING
const float DIST_ALERT   = 15.0;   // cm  -> closer than this = ALERT
const int   LIGHT_WARNING = 2000;  // ADC -> above this = WARNING
const int   LIGHT_ALERT   = 2700;  // ADC -> above this = ALERT

enum State { SAFE = 0, WARNING = 1, ALERT = 2 };
const char* STATE_NAMES[] = { "SAFE", "WARNING", "ALERT" };

// ---------- Sensors ----------
float readDistanceOnce() {
  digitalWrite(TRIG_PIN, LOW);  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long d = pulseIn(ECHO_PIN, HIGH, 30000);
  if (d == 0) return -1;
  return d * 0.0343 / 2.0;
}

// Median of 3 valid readings: removes occasional spikes (e.g. 194.9 cm in A1)
float readDistance() {
  float v[3]; int n = 0;
  for (int i = 0; i < 5 && n < 3; i++) {
    float d = readDistanceOnce();
    if (d > 0) v[n++] = d;
    delay(15);
  }
  if (n == 0) return -1;            // no echo: nothing in range
  if (n < 3) return v[0];
  // sort 3 values
  if (v[0] > v[1]) { float t = v[0]; v[0] = v[1]; v[1] = t; }
  if (v[1] > v[2]) { float t = v[1]; v[1] = v[2]; v[2] = t; }
  if (v[0] > v[1]) { float t = v[0]; v[0] = v[1]; v[1] = t; }
  return v[1];
}

int readLight() {
  long sum = 0;
  for (int i = 0; i < 5; i++) { sum += analogRead(LDR_PIN); delay(2); }
  return sum / 5;
}

// ---------- Logic ----------
State distanceState(float d) {
  if (d < 0) return SAFE;            // no object detected
  if (d < DIST_ALERT) return ALERT;
  if (d < DIST_WARNING) return WARNING;
  return SAFE;
}

State lightState(int l) {
  if (l > LIGHT_ALERT) return ALERT;
  if (l > LIGHT_WARNING) return WARNING;
  return SAFE;
}

// ---------- Outputs ----------
void showState(State s) {
  digitalWrite(LED_GREEN,  s == SAFE);
  digitalWrite(LED_YELLOW, s == WARNING);
  digitalWrite(LED_RED,    s == ALERT);
#ifdef RGB_BUILTIN
  if (s == SAFE)         rgbLedWrite(RGB_BUILTIN, 0, 40, 0);
  else if (s == WARNING) rgbLedWrite(RGB_BUILTIN, 40, 25, 0);
  else                   rgbLedWrite(RGB_BUILTIN, 60, 0, 0);
#endif
}

// Non-blocking buzzer: WARNING = short beep every second, ALERT = fast beeping
void updateBuzzer(State s) {
  unsigned long t = millis();
  bool on = false;
  if (s == WARNING) on = (t % 1000) < 100;
  else if (s == ALERT) on = (t % 250) < 125;
  digitalWrite(BUZZER_PIN, on);
}

// ---------- Main ----------
State state = SAFE;
unsigned long lastRead = 0;

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  delay(1000);
  Serial.println("ArtGuard - A2 (state logic)");
}

void loop() {
  if (millis() - lastRead >= 300) {
    lastRead = millis();
    float d = readDistance();
    int l = readLight();
    State sd = distanceState(d);
    State sl = lightState(l);
    state = (sd > sl) ? sd : sl;     // the most critical one wins
    showState(state);

    Serial.print("Distance: ");
    if (d < 0) Serial.print("---"); else Serial.print(d, 1);
    Serial.print(" cm | Light: ");
    Serial.print(l);
    Serial.print(" | State: ");
    Serial.println(STATE_NAMES[state]);
  }
  updateBuzzer(state);
}
