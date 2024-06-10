#define BLYNK_TEMPLATE_ID "TMPL22ub2cKVi"
#define BLYNK_TEMPLATE_NAME "Weather Station"
#define BLYNK_AUTH_TOKEN "jqRVB2LImeiBvl5pU35jE01oM6k4muNO"
#define BLYNK_PRINT Serial

#include <ESP8266WiFi.h> 
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "ASP";  // type your wifi name
char pass[] = "asp100111";  // type your wifi password

BlynkTimer timer;
Servo servo;

#define DHTPIN 5 //Connect Out pin to D5 in NODE MCU
DHT dht(DHTPIN, DHT11);

const int AirValue = 12;   //you need to replace this value with Value_1
const int WaterValue = 0;  //you need to replace this value with Value_2
const int SensorPin = A0;
int soilMoistureValue = 0;
int soilmoisturepercent = 0;

BLYNK_WRITE(V2) { // V2 is for the LED
  digitalWrite(D0, param.asInt());
}

void sendSensor()
{
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
 
  Serial.print("Air Temperature: ");
  Serial.println(t);
  Serial.print("Air Humidity: ");
  Serial.println(h);
  
  servo.attach(D4);

  Blynk.virtualWrite(V0, t); // V0 is for Temperature
  Blynk.virtualWrite(V1, h); // V1 is for Humidity

  soilMoistureValue = analogRead(SensorPin);  //put Sensor insert into soil
  Serial.println(soilMoistureValue);
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  Blynk.virtualWrite(V4, soilmoisturepercent); // V4 is for de soil moisture
  if (soilmoisturepercent > 100)
  {
    Serial.println("100 %");
    delay(1500);
  }
  else if (soilmoisturepercent < 0)
  {
    Serial.println("0 %");
    delay(1500);
  }
  else if (soilmoisturepercent >= 0 && soilmoisturepercent <= 100)
  {
    Serial.print(soilmoisturepercent);
    Serial.println("%");
    delay(1500);
  }
  if (soilmoisturepercent >= 0 && soilmoisturepercent <= 30)
  {
    Serial.println("needs water, send notification");
    //send notification
    Serial.println("Motor is ON");
    delay(1000);
  }
  else if (soilmoisturepercent > 30 && soilmoisturepercent <= 100)
  {
    Serial.println("Soil Moisture level looks good...");
    Serial.println("Motor is OFF");
    delay(1000);
  }
}

void setup()
{   
  Serial.begin(115200);
  delay(100);
  Blynk.begin(auth, ssid, pass);
  
  pinMode(D0, OUTPUT); //Set the LED pin as an output pin

  dht.begin();
  timer.setInterval(100L, sendSensor);
}

void loop()
{
  Blynk.run();
  timer.run();
}

BLYNK_WRITE(V3) // V3 is for servo
{
  servo.write(param.asInt());
}