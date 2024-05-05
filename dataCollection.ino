#include "WiFi.h"
#include "DHT.h"

#define DHT_PIN 5
#define DHTTYPE DHT11
#define SOUND_PIN 18


const char* ssid = "waifai";
const char* password = "holaa123";
const uint16_t port = 8090;
const char * host = "192.168.73.36";

WiFiClient client;
DHT dht(DHT_PIN, DHTTYPE); // constructor to declare our sensor

int timer;

void setup() {
  pinMode(SOUND_PIN, INPUT);
  dht.begin();
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.print("WiFi connected with IP: ");
  Serial.println(WiFi.localIP());

  timer = 0;
}

void loop() {
  //Detects whether the connection was successful
  
  if (client.connect(host, port))
    Serial.println("Connection successful");
  else
    Serial.println("Conection failed");
    
  char buff[64];
  float temp = dht.readTemperature();
  delay(1000);
  float humidity = dht.readHumidity();

  sprintf(buff, "%d,%.2f,%.2f", timer, temp, humidity);
  client.print(buff);
  client.print(" ");
  Serial.print(buff);
  timer = timer + 1;
  delay(1000);
}

//It returns the voltage read by the pin
float readAdc(int pin) {
  int tempSensorValue = analogRead(A0);
  return float(tempSensorValue)*(5./4095.);
}

void printTemp(float Vtemp) {
    Serial.print(-88.23*Vtemp+86.17);
    Serial.print(" ºC  -  ");
    Serial.print(Vtemp);
    Serial.println(" V");
}