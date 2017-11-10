//Include libraries
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#define DHTPIN 32
#define DHTTYPE DHT22
#define water_level A9
#define Relay_On 0
#define Relay_Off 1
#define Air_Thrower 28
#define Compressor 27
#define IO_Fans 30
//#define water_level_pstv  A8
//#define water_level_ngtv  A11
DHT dht(DHTPIN, DHTTYPE);
// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 31
// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
int R1= 1000;
int Ra=25; //Resistance of powering Pins
int ECPin= A0;
int ECGround=A1;
int ECPower =A4;
int EC_Solution_min = 0.00115; // in Seimens
int EC_Solution_max = 0.0012;
float PPMconversion=0.5;
float Temp_LM = 0;
float h, Temp_DHT, level;

// int LM_Pin = A1;
 
//*************Compensating for temperature ************************************//
//The value below will change depending on what chemical solution we are measuring
//0.019 is generaly considered the standard for plant nutrients [google "Temperature compensation EC" for more info
float TemperatureCoef = 0.019; //this changes depending on what chemical we are measuring 
 
//********************** Cell Constant For Ec Measurements *********************//
//If you get bad readings you can use the calibration script to get a better estimate for K
float K=2.88;
float Temperature=10;
float EC=0;
float EC25 =0;
int ppm =0;
 
 
float raw= 0;
float Vin= 5;
float Vdrop= 0;
float Rc= 0;
float buffer=0;

unsigned long pickUp_Time = 0; // code uploaded at 14 hours
unsigned long Start_Time = millis() + pickUp_Time; 


void setup(void)
{
  Serial.begin(9600); //Begin serial communication
  Serial.println("Arduino Digital Temperature // Serial Monitor Version"); //Print a message
  
  sensors.begin();
  dht.begin();

  pinMode(ECPin, INPUT);
  pinMode(water_level, INPUT);
  pinMode(Compressor, OUTPUT);
  pinMode(Air_Thrower, OUTPUT);
  pinMode(IO_Fans, OUTPUT);

  digitalWrite(Air_Thrower,Relay_On);
  digitalWrite(IO_Fans, Relay_Off);
  digitalWrite(Compressor, Relay_Off);
  
  delay(300000);
  
}

void fetchValues(){
  // digitalWrite(ECPower, HIGH);
  // digitalWrite(ECGround, LOW);
//  digitalWrite(water_level_pstv, HIGH);
//  digitalWrite(water_level_ngtv, LOW);
// ************** Reading Humidity and Temperature Values from DHT Sensor *******************// 
  sensors.requestTemperatures();
  Serial.print( "Water Temperature: ");
  Temperature  = sensors.getTempCByIndex(0);
  Serial.print(Temperature); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
  h = dht.readHumidity();
  Temp_DHT = dht.readTemperature();
// ******************************************************************************************//
// *************** Reading Temperature Values from LM35 Sensor ******************************//  
  // Temp_LM = analogRead(LM_Pin);
  // Temp_LM = Temp_LM * 0.48828125; 
// *****************************************************************************************//
  Serial.print(" | ");
  Serial.print("Air Temperature from DHT: ");
  Serial.print(Temp_DHT);
  Serial.print(" | ");
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" | ");
  Serial.print("Air Temperature from LM35: ");
  Serial.print(Temp_LM);
  Serial.print(" | ");
  //************Estimates Resistance of Liquid ****************//
  digitalWrite(ECPower,HIGH);
  raw= analogRead(ECPin);
  raw= analogRead(ECPin);// This is not a mistake, First reading will be low beause if charged a capacitor
  digitalWrite(ECPower,LOW);
  //***************** Converts to EC **************************//
  Vdrop= (Vin*raw)/1024.0;
  Rc=(Vdrop*R1)/(Vin-Vdrop);
  Rc=Rc-Ra; //acounting for Digital Pin Resitance
  EC = 1000/(Rc*K);
  //*************Compensating For Temperaure********************//
  EC25  =  EC/ (1+ TemperatureCoef*(Temperature-25.0));
  ppm=(EC25)*(PPMconversion*1000);
  Serial.print("EC Value: ");
  Serial.print(EC25, 7);
  level = ((analogRead(water_level))/1023.0)*100;
  Serial.print(" | ");
  Serial.print("Water Level: ");
  Serial.print(level,2);
  Serial.println("%");


}

void loop(void)
{ 
  
  unsigned long Current_Time = millis();
  unsigned long Elapsed_Time = Current_Time - Start_Time;
// **************** Converting Elapsed Time to hours, minutes, seconds ***************************//  
  unsigned long hours = Elapsed_Time / 3600000;
  unsigned long rem_h = Elapsed_Time % 3600000;
  unsigned long minutes = rem_h / 60000;
  unsigned long rem_m = rem_h % 60000;
  unsigned long seconds = rem_m / 1000;
  
  // fetch and print values
  fetchValues();
  
  //Update value every 1 sec.

  if (h >= 70)
  {
    Serial.println("Humidity Levels crossed 70%, turning Compressor/Air_Thrower ON.");
    Serial.print(hours);
    Serial.print(":");
    Serial.print(minutes);
    Serial.print(":");
    Serial.println(seconds);
    // execute till condition is not met
    while(h >= 70){
      digitalWrite(Compressor, Relay_On);
      digitalWrite(Air_Thrower, Relay_On);
      fetchValues();
    }
  }

  if (h <= 50)
  {
    Serial.print("Humidity fallen below 50%, turning Compressor/Air_Thrower OFF | ");
    Serial.print(hours);
    Serial.print("hours ");
    Serial.print(minutes);
    Serial.print("minutes ");
    Serial.print(seconds);
    Serial.println("seconds ");
    // execute till condition is not met
    while(h <= 50){
      digitalWrite(Compressor, Relay_Off);
      digitalWrite(Air_Thrower, Relay_Off);
      fetchValues();
    }
  }

  if(Temp_DHT >= 26)
  {
    Serial.println("Temprature above 26 deg. Cel., turning Compressor/Air_Thrower OFF | ");
    Serial.print(hours);
    Serial.print(":");
    Serial.print(minutes);
    Serial.print(":");
    Serial.println(seconds);
    // execute till condition is not met
    while(Temp_DHT >= 26){
      digitalWrite(Compressor, Relay_On);
      digitalWrite(Air_Thrower, Relay_On);
      fetchValues();
    }
    
  }
  if(Temp_DHT <= 24)
  {
    Serial.println("Temprature below 24 deg. Cel., turning Compressor/Air_Thrower OFF | ");
    Serial.print(hours);
    Serial.print(":");
    Serial.print(minutes);
    Serial.print(":");
    Serial.println(seconds);
    // execute till condition is not met
    while(Temp_DHT <= 24){
      digitalWrite(Compressor, Relay_Off);
      digitalWrite(Air_Thrower, Relay_Off);
      fetchValues();
    }
  }
  
  // if ((((Elapsed_Time % 7200000) <= 120000)) && (Elapsed_Time > 120000))
  // {
  //   bool io_flag = false;
  //   Serial.println("Defrosting for better cooling, the Compressor is being turned off for 5 minutes.");
  //   Serial.print("Compressor turned off at: ");
  //   Serial.print(hours);
  //   Serial.print(":");
  //   Serial.print(minutes);
  //   Serial.print(":");
  //   Serial.println(seconds);
  //   digitalWrite(Compressor, Relay_Off);
    
  if ((((Elapsed_Time % 21600000) < 120000)))
  {
    // io_flag = true; // Indicator, if IO Fans are ON or OFF
    Serial.println("Flushing out the air.");
    Serial.print("I/O Fans turned ON , Air Thrower turned OFF |");
    Serial.print(hours);
    Serial.print(":");
    Serial.print(minutes);
    Serial.print(":");
    Serial.println(seconds);
    digitalWrite(IO_Fans, Relay_On);
    digitalWrite(Air_Thrower, Relay_Off);
    delay(300000);
    digitalWrite(IO_Fans, Relay_Off);
    digitalWrite(Air_Thrower, Relay_On);
  }

  
  //   Serial.println("Turning Compressor ON");
  //   digitalWrite(Compressor, Relay_On);
    
  //   if (io_flag == true){
  //     Serial.println("Turning IO Fans OFF and Air Thrower ON");
  //     digitalWrite(IO_Fans, Relay_Off);
  //     digitalWrite(Air_Thrower, Relay_On);

  //     }
  // }
         
  delay(1000);
}
