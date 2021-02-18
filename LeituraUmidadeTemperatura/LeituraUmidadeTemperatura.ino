#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "DHT.h"

#define DHTPIN1 D4
#define DHTPIN2 D5
#define DHTTYPE DHT11

const char* ssid = "FELIPE";
const char* password = "penaareia";
const char* host = "https://projeto-siv.herokuapp.com";
const char* urlAtivo = "/ativo";
const char* urlInformacoes = "/informacao";
const int httpsPort = 443;
const char fingerprint[] PROGMEM = "94 FC F6 23 6C 37 D5 E7 92 78 3C 0B 5F AD 0C E4 9E FD 9E A8";

WiFiClientSecure client;
DHT dht1(DHTPIN1, DHTTYPE);
DHT dht2(DHTPIN2, DHTTYPE);

void setup() {
  Serial.begin(115200);
  conectaWifi();
  setFingerprintHost();
  dht1.begin();
  dht2.begin();
}

void conectaWifi(){
  Serial.print("connectando em ");
  Serial.println(ssid);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.print("WiFi connectado no ip ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void setFingerprintHost(){
  client.setFingerprint(fingerprint);
  if (!client.connect(host, httpsPort)) {
    Serial.println("Problemas na conexão, tentando reconectar...");
    setFingerprintHost();
  } else {
    Serial.print("Conectado ao host:");
    Serial.println(host);
  }
}

String getVasoAtivo(){
  
  client.print(String("GET ") 
                + urlAtivo 
                + " HTTP/1.1\r\n" 
                + "Host: " + host 
                + "\r\n" + "User-Agent: ESP8266\r\n" 
                + "Connection: close\r\n\r\n");
  Serial.println();             
  Serial.println("request GET enviado");
  Serial.println();
  
  while (client.connected()) {
    String linha = client.readStringUntil('\n');
    if(linha.startsWith("{")){
      return linha;
    }
  }
}

void verificaConexoes(){
  if(WiFi.status() != WL_CONNECTED){
    conectaWifi();
  }

  if (!client.connect(host, httpsPort)) {
    Serial.println("Problemas na conexão, tentando reconectar...");
    setFingerprintHost();
  }
}

void loop() {
  
  verificaConexoes(); 
  
  String line = getVasoAtivo();
  int vaso1 = (int)line[11];
  int vaso2 = (int)line[23];
  
  if(vaso1 == 49){
    Serial.println("Post vaso 1");
    float temperatura1 = dht1.readTemperature(); //lendo a temperatura
    Serial.print("Temperatura1:");
    Serial.println(temperatura1);
   // postInformacao(1, temperatura1, 100);
  }

  if(vaso2 == 49){
    Serial.println("Post vaso 2");
    float temperatura2 = dht2.readTemperature(); //lendo a temperatura
    Serial.print("Temperatura2:");
    Serial.println(temperatura2);
   // postInformacao(2, temperatura2, 200);
  }


  //dar um get no endpoint do regador
  
  delay(5000);
}

void postInformacao(int idVaso, int temperatura, int umidade){
  int recebido = 0;
  String msg = "{\"idVaso\":\"" + (String)idVaso + "\""
                + ",\"t\":\"" + (String)temperatura 
                + "\",\"u\":\"" + (String)umidade 
                +"\"}";
  
  client.connect(host, httpsPort);
  if (!client.connect(host, httpsPort)) {
    Serial.println("Connection failed!");
    return;
  }
  
  client.println(String("POST ") + urlInformacoes + " HTTP/1.1");
  client.println( "Host: " + (String)host);
  client.println("User-Agent: ESP8266/1.0");
  client.println("Connection: close");
  client.println("Content-Type: application/json;");
  client.print("Content-Length: ");
  client.println(msg.length());
  client.println();
  client.println(msg);
 
  Serial.println();
  
  while (client.connected()) {
    String linha = client.readStringUntil('\n');
    if(linha.startsWith("\"Objeto cadastrado!\"")){
      recebido = 1;
    }
  }
  
  if(recebido){
    Serial.println("Objeto cadastrado com sucesso.");
  } else {
    Serial.println("Problemas ao cadastrar.");
  }
  
}
