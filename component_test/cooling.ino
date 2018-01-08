include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>

// Compressor pin
int cmpPin = 22;
// fan thrower pin
int fanThrwPin = 23;
// define humidity-temp sensor type
#define dhtTYPE DHT22
// Humidity-temp data pint 
int dhtPin = 24; 
// define water temp data pin
int watTempPin = 2; 
// define lower and upper bound Temperatures
float waterUB = 20.0, waterLB = 16.0;
float airUB = 23.0, airLB = 19.0;

// Launch a dht instance
DHT dht(dhtPin, dhtTYPE);
// launch OneWire instance
OneWire watSensor(watTempPin);
DallasTemperature sensors(&watSensor);

void setup() {
  // intialize serial monitor
  Serial.begin(9600);
  Serial.println("********Starting cooling test********");
  // define mode
 
  pinMode(cmpPin, OUTPUT);
  pinMode(fanThrwPin, OUTPUT);  
  sensors.begin();
  // mark some delay here
}

void loop(){
  // read humidity-sensor data
  float h = dht.readHumidity();
  float airT = dht.readTemperature();
  // read water temp data
  sensors.requestTemperatures();
  float waterT = sensors.getTempCByIndex(0);

  delay(2000);

//  Serial.println("Air Temperature: %4.2f | Humidity : %4.2f | Water Temperature: %4.2f ", airT, h, waterT );
  Serial.print("Air Temperature: ");
  Serial.print(airT);
  Serial.print("\t");
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print("%");
  Serial.print("\t");
  Serial.print("Water Temperature: ");
  Serial.print(waterT);
  Serial.print("\t");
  Serial.println();
  
  if(waterT > waterUB){
    // turn compressor on
    digitalWrite(cmpPin, LOW);   
    
    if(airT > airUB){
      // turn fan thrower on
      digitalWrite(fanThrwPin, LOW);
    }
    if(airT < airLB){
      // turn fan thrower off
      digitalWrite(fanThrwPin, HIGH);
    }

  }

  if(waterT < waterLB){
    // turn compressor off
    digitalWrite(cmpPin, HIGH);

  }

  delay(1000);

}

