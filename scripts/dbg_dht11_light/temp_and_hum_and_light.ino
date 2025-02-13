#include <stDHT.h>
DHT sens(DHT11); // Указать датчик DHT11, DHT21 или DHT22. (несколько датчиков вписывать не нужно)
// Подключать можно только одинаковые датчики, то есть нельзя использовать одновременно DHT11 и DHT22

void setup() 
{
  Serial.begin(9600);
  pinMode(2, INPUT);
  digitalWrite(2, HIGH);
  pinMode(3, OUTPUT);//red
  pinMode(4, OUTPUT);//green
  pinMode(5, OUTPUT);//blue
}

void loop() 
{
  int t = sens.readTemperature(2); // чтение датчика на пине 2
  int h = sens.readHumidity(2);    // чтение датчика на пине 2
  if (h>=55){
      digitalWrite(4, HIGH);
      digitalWrite(5, LOW);
      digitalWrite(3, LOW);
  } else if (h<=50){
      digitalWrite(3, HIGH);
      digitalWrite(4, LOW);
      digitalWrite(5, LOW);
  } else{
      digitalWrite(5, HIGH);
      digitalWrite(4, LOW);
      digitalWrite(3, LOW);
  }
  if (t >= 26){
    digitalWrite(6, HIGH);
  } else digitalWrite(6, LOW);
  
  delay(1000);
  Serial.print("Hum: ");
  Serial.print(h);
  Serial.print(" %");
  Serial.print(" Temp: ");
  Serial.print(t);
  Serial.println(" C ");

}
