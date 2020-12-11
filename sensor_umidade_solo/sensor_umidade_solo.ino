
int umidade;
 
void setup()
{
 Serial.begin(115200);
 Serial.println("www.usinainfo.com.br");
 pinMode(13, OUTPUT);
}
void loop()
{
 umidade = analogRead(A0);
 int Porcento = map(umidade, 1023, 0, 0, 100);
 
 Serial.print(Porcento);
 Serial.println("%");
 if(Porcento <=70)
 {
 Serial.println("Irrigando...");
 digitalWrite(13, HIGH);
 }
 
 else
 {
 digitalWrite(13, LOW);
 }
 delay(1000);
}
