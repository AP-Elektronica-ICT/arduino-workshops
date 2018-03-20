#include <TheThingsNetwork.h>
#include <SoftwareSerial.h>
#include <math.h>

// Set your AppEUI and AppKey
const char *appEui = "70B3D57ED000AC9A";
const char *appKey = "8BD3AB3633D3B1EEBCB6BF5B004B6038";

SoftwareSerial loraSerial(10, 11);
#define debugSerial Serial

// Replace REPLACE_ME with TTN_FP_EU868 or TTN_FP_US915
#define freqPlan TTN_FP_EU868

TheThingsNetwork ttn(loraSerial, debugSerial, freqPlan);

const int RST_PIN = 4;
const int NTC_PIN = A0;

const int VDIVIDER_R1 = 9620;
const int NTC_B_VAL = 4300;
const float T0_KELV = 298.15;

union temperature {
  float value;
  byte val_bytes[sizeof(double)];
};

float adc_to_resistance(int adc_pin) {
  return (VDIVIDER_R1 / (1023 / (double) analogRead(adc_pin) - 1));
}

float resistance_to_temp(float r){
  return NTC_B_VAL / log(r / (VDIVIDER_R1 * exp(-NTC_B_VAL/T0_KELV)));
}


void reset() {
  digitalWrite(RST_PIN, LOW);
  delay(10);
  digitalWrite(RST_PIN, HIGH);
}

void setup()
{
  pinMode(RST_PIN, OUTPUT);
  pinMode(NTC_PIN, INPUT);

  digitalWrite(RST_PIN, HIGH);

  loraSerial.begin(57600);
  debugSerial.begin(9600);

  // Wait a maximum of 10s for Serial Monitor
  while (!debugSerial && millis() < 10000);

  debugSerial.println("-- STATUS");
  reset();
  ttn.showStatus();

  debugSerial.println("-- JOIN");
  ttn.join(appEui, appKey);
}

void loop()
{
  debugSerial.println("-- LOOP");

  // Prepare payload of 1 byte to indicate LED status
  float r = adc_to_resistance(NTC_PIN);
  union temperature degc;
  degc.value = resistance_to_temp(r) - 273.15;  
  debugSerial.println(r);
  debugSerial.println(degc.value);
  
  // Send it off
  ttn.sendBytes(degc.val_bytes, sizeof(degc.val_bytes));

  delay(5000);
}
