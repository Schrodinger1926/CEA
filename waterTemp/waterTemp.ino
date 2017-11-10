//Include libraries
#include <DallasTemperature.h>
#include <DHT.h>

#define RELAY_ON 0
#define RELAY_OFF 1

#define DHTPIN 32
#define DHTTYPE DHT22
#define Relay_Cooler  27
#define Relay_AirThrower 28
#define Relay_Fans 30
DHT dht(DHTPIN, DHTTYPE);

unsigned long Defrost_Time = 15*60*(1000L); // L is by covention, issue: https://github.com/arduino/Arduino/issues/3590
unsigned long Air_Flush_Time = 60*60*(1000L);
unsigned long  Compressor_lock_Time = 5*60*(1000L);

int readTime = 5*60; // intentionally left as it is, works on counter basis not on delay basis


// Variable Set points
float T_low = 24.0;
float T_high = 26.0;
float H_low = 50.0;
float H_high = 70.0;


String collectData()
{
  String data;
  while(Serial.available() > 0)
  {
          char c = Serial.read();
          data += c;
  }
  return data;
}


float * parseData(String data)
{
  static float res[10];
  // fill in the data in each element of the float array
  // data --> string type
  int i = 0, j = 0;
  String temp;
  
  while(i < data.length())
  {
    char c = data.charAt(i);
    String s;
    while(c != ' ')
    {
      s += c;
      i++;
      // only access the next char if index is with String length
      if (i < data.length())
      {
        c = data.charAt(i);
      }
      else
      {
        c = ' ';
      }
      
    }
      
    res[j] = s.toFloat();
    j++;
    i++;
  }

  return res;
}


float * ReadTempHumidity(){
  // intialize avg local variables
  static float res[3];
  float t_avg = 0;
  float h_avg = 0;
  float counter = 0;
  while (counter != readTime){
     // read temperature and humidity
     t_avg += dht.readTemperature();
     h_avg += dht.readHumidity();
     counter += 1.0;
     delay(1000);
    }
  res[0] = t_avg/readTime;
  res[1] = h_avg/readTime;

  return res;
  }


void SetTempAndHumidity(float t, float h){
  
  // check if any one hits the upper/lower bound
  if (h < H_low or t < T_low){
    // turn off compressor and fan thrower
    Serial.println("Turning Compressor-Fan OFF");
    digitalWrite(Relay_Cooler, RELAY_OFF);
    digitalWrite(Relay_AirThrower, RELAY_OFF);
    Serial.println("Compressor locked");
    delay(Compressor_lock_Time);
    }

  if (h > H_high or t > T_high){
    // turn on compressor and fan thrower
    Serial.println("Turning Compressor-Fan ON");
    digitalWrite(Relay_Cooler, RELAY_ON);
    digitalWrite(Relay_AirThrower, RELAY_ON);
  }
    
  }


void setup(void)
{
  Serial.begin(9600); //Begin serial communication
  Serial.println("Germination Control// Serial Monitor Version"); //Print a message
  delay(20000);
  // Start timer now
}


void loop(void)
{ 
  // fetch defrost and fans from serial line
  delay(1000); // fill buffer
  if (Serial.available() > 0){
    String data = collectData();
    float *val = parseData(data);
    float defrost = *val;
    float fans = *(val + 1);
    
    if (fans > 0){
      // turn ON IO fans for 5 mins
      digitalWrite(Relay_Fans, RELAY_ON);
      Serial.println("Air flush");
      delay(Air_Flush_Time);
      }
    
    if (defrost > 0){
      // turn OFF compressor for 15 mins 
      digitalWrite(Relay_Cooler, RELAY_OFF);
      Serial.println("Defrosting Chiller");
      delay(Defrost_Time);
      }
           
    }
        
  
  
  else {
    // calculate average temperature and humidity for last 'readTime' mins
    float *val = ReadTempHumidity();
    
    // de-reference
    float t_avg = *val;
    float h_avg = *(val+1);
    Serial.print("Average Temperature : ");
    Serial.print(t_avg);
    Serial.print(" | ");
    Serial.print("Average Humidity : ");
    Serial.println(h_avg);
    
    // Maintain air temperature and Humidity
    SetTempAndHumidity(t_avg, h_avg);
  }

  delay(5000);
  
  
}
