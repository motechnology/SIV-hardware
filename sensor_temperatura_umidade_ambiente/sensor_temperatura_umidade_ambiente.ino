#include "DHT.h"

#define DHTPIN D4     // Digital pin connected to the DHT sensor

#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  Serial.println(F("DHTxx test!"));
  dht.begin();
}

void loop() {
 
  delay(2000);
  
  float h = dht.readHumidity(); //lendo a umidade
  float t = dht.readTemperature(); //lendo a temperatura

  // validando os dados capturados
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  Serial.print(F("Umidade: "));
  Serial.println(h);
  Serial.print(F("temperatura: "));
  Serial.print(t);
  Serial.println(F("°C "));
  
}
