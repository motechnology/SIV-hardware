#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "DHT.h"

#define DHTPIN1 D4
#define DHTPIN2 D5
#define DHTTYPE DHT11
#define BOMBA1PIN D8
#define BOMBA2PIN D7
#define LED_VERDE D1
#define LED_VERMELHO D2

const char* ssid = "FELIPE";
const char* password = "penaareia";
const char* host = "projeto-siv.herokuapp.com";
const char* urlAtivo = "/ativo";
const char* urlInformacoes = "/informacao";
const char* urlRegador = "/bomba";
const uint16_t httpsPort = 443;
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
  pinMode(BOMBA1PIN, OUTPUT);
  pinMode(BOMBA2PIN, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  inicializa();
}

void inicializa(){
  digitalWrite(LED_VERDE, HIGH);
  digitalWrite(LED_VERMELHO, HIGH);
  delay(500);
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, LOW);
  delay(500);
  digitalWrite(LED_VERDE, HIGH);
  digitalWrite(LED_VERMELHO, HIGH);
  delay(500);
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, LOW);
  delay(500);
}

void liga(){
  digitalWrite(LED_VERDE, HIGH);
  digitalWrite(LED_VERMELHO, LOW);
}

void off(){
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, HIGH);
}

void loop() {
  
  verificaConexoes(); 
  
  String line = getApi(urlAtivo);
  if(line == "") return;
  
  int vaso1 = (int)line[11];
  int vaso2 = (int)line[23];
  
  if(vaso1 == 49){
    getUmidadeTemperatura(dht1, 1);
  }

  if(vaso2 == 49){
    getUmidadeTemperatura(dht2, 2);
  }

  setFingerprintHost();
  Serial.println();
  Serial.println("Verificando temporização bomba");
  line = getApi(urlRegador);
  if(line == "") return;
  
  int posA = line.indexOf(':');
  int posB = line.indexOf(',');
  int tempoA = atoi(line.substring(posA+1, posB).c_str());
 // int tempoA = 10;
  if(tempoA != 0) ativaBomba(1, tempoA);
  
  int posC = line.indexOf(':', posA + 1);
  int posD = line.indexOf(',', posB + 1);
  int tempoB = atoi(line.substring(posC+1, posD).c_str());
 // int tempoB = 5;
  if(tempoB != 0) ativaBomba(2, tempoB);

  delay(5000);
}

void ativaBomba(int bomba, int tempo){
  if(bomba == 1){
    Serial.print("Bomba 1 ativa por: ");
    Serial.println(tempo);
    digitalWrite(BOMBA1PIN, HIGH);
    delay(tempo*1000);
    digitalWrite(BOMBA1PIN, LOW);
    Serial.println("Desativando bomba 1...");
  }
  if(bomba == 2){
    Serial.print("Bomba 2 ativa por: ");
    Serial.println(tempo);
    digitalWrite(BOMBA2PIN, HIGH);
    delay(tempo*1000);
    digitalWrite(BOMBA2PIN, LOW);
    Serial.println("Desativando bomba 2...");
  }
}

void conectaWifi(){
  Serial.println();
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

String getApi(String url){
  client.print(String("GET ") 
                + url 
                + " HTTP/1.1\r\n" 
                + "Host: " + host + 
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

void setFingerprintHost(){
  Serial.println();
  client.setFingerprint(fingerprint);
  if (client.connect(host, httpsPort)) {
    Serial.print("Conectado ao host:");
    Serial.println(host);
  } else {
    Serial.println("Problemas na conexão, tentando reconectar...");
    setFingerprintHost();
  }
}

void verificaConexoes(){
  if(WiFi.status() != WL_CONNECTED){
    conectaWifi();
    off();
  }

  if (!client.connect(host, httpsPort)) {
    Serial.println("Problemas na conexão, tentando reconectar...");
    setFingerprintHost();
    off();
  }
  liga();
}

void getUmidadeTemperatura(DHT dht, int vaso){
  Serial.print("Post vaso ");
  Serial.println(vaso);
  float temperatura = dht.readTemperature();
  float umidade = dht.readHumidity();
  postInformacao(vaso, temperatura, umidade);
}

void postInformacao(int idVaso, int temperatura, int umidade){
  int recebido = 0;
  String msg = "{\"idVaso\":\"" + (String)idVaso + "\""
                + ",\"t\":\"" + (String)temperatura 
                + "\",\"u\":\"" + (String)umidade 
                +"\"}";
  Serial.print("");
  Serial.print("Postando objeto :");
  Serial.println(msg);
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
