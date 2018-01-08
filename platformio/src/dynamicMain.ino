#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#define DHTPIN 32
#define DHTTYPE DHT22
#define water_level A9
#define Relay_On 0
#define Relay_Off 1
#define Lights_One 22
#define Lights_Two 23
#define Lights_Three 24
#define Lights_Four 25 
#define Air_Thrower 28
#define Compressor 27
#define IO_Fans 30

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
float h, Temp_DHT, level;

// PowerMOS GATE, vary light intensity
#define gate 3

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
int compressor_state, airthrower_state;
unsigned long pickUp_Time = 0; // code uploaded at 1400 hours

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
  pinMode(Lights_One, OUTPUT);
  pinMode(Lights_Two, OUTPUT);
  pinMode(Lights_Three, OUTPUT);
  pinMode(Lights_Four, OUTPUT);
  pinMode(gate, OUTPUT);

  digitalWrite(Air_Thrower,Relay_On);
  digitalWrite(IO_Fans, Relay_Off);
  digitalWrite(Compressor, Relay_Off);
  digitalWrite(Lights_One, Relay_On);
  digitalWrite(Lights_Two, Relay_On);
  digitalWrite(Lights_Three, Relay_On);
  digitalWrite(Lights_Four, Relay_On);
  analogWrite(gate, 50);
  delay(300000);
  
}

void fetchValues(){
  
  // ************** Reading Humidity and Temperature Values from DHT Sensor *******************// 
  sensors.requestTemperatures();
  Serial.print( "Water Temperature: ");
  Temperature  = sensors.getTempCByIndex(0);
  Serial.print(Temperature); // Why "byIndex"? You can have more than one IC on the same bus. 0 refers to the first IC on the wire
  h = dht.readHumidity();
  Temp_DHT = dht.readTemperature();
  
  // *****************************************************************************************//
  Serial.print(" | ");
  Serial.print("Air Temperature from DHT: ");
  Serial.print(Temp_DHT);
  Serial.print(" | ");
  Serial.print("Humidity: ");
  Serial.print(h);
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
  
  //*************Compensating For Temperaure*******************//
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
  
  
  
  // fetch and print values
  fetchValues();
  

  delay(1000); // buffer fillup
  if (Serial.available() > 0)
  {
    String data = collectData();
    Serial.print("received ");
    Serial.print(data);
    Serial.print(" Parsed ");
    float *val;
    val = parseData(data);
    compressor_state = *val;
    airthrower_state = *(val+1);

    digitalWrite(Compressor, getState(compressor_state));
    digitalWrite(Air_Thrower, getState(airthrower_state));
    delay(1000);
    fetchValues();


    unsigned long Current_Time = millis();
    unsigned long Elapsed_Time = Current_Time - Start_Time;
    // **************** Converting Elapsed Time to hours, minutes, seconds ***************************//  
    unsigned long hours = Elapsed_Time / 3600000;
    unsigned long rem_h = Elapsed_Time % 3600000;
    unsigned long minutes = rem_h / 60000;
    unsigned long rem_m = rem_h % 60000;
    unsigned long seconds = rem_m / 1000;


    if((Elapsed_Time % 64800000 <= 21600000) && (Elapsed_Time >= 21600000))
    {
      Serial.print("Turning Lights Off");
      Serial.print(hours);
      Serial.print(":");
      Serial.print(minutes);
      Serial.print(":");
      Serial.println(seconds);
      digitalWrite(Lights_One, Relay_Off);
      digitalWrite(Lights_Two, Relay_Off);
      digitalWrite(Lights_Three, Relay_Off);
      digitalWrite(Lights_Four, Relay_Off);
    }
  
    if(Elapsed_Time % 64800000 > 21600000)
    {
      Serial.print("Turning Lights On");
      Serial.print(hours);
      Serial.print(":");
      Serial.print(minutes);
      Serial.print(":");
      Serial.println(seconds);
      digitalWrite(Lights_One, Relay_On);
      digitalWrite(Lights_Two, Relay_On);
      digitalWrite(Lights_Three, Relay_On);
      digitalWrite(Lights_Four, Relay_On);
    }


  
    // 40 mins
    if ((((Elapsed_Time % 2400000) <= 300000)) && (Elapsed_Time > 300000))
     {
       Serial.println("Defrosting for better cooling, the Compressor is being turned off for 5 minutes.");
       Serial.print("Compressor turned off at: ");
       Serial.print(hours);
       Serial.print(":");
       Serial.print(minutes);
       Serial.print(":");
       Serial.println(seconds);
       digitalWrite(Compressor, Relay_Off);
       digitalWrite(Air_Thrower, Relay_On); 
    
      // ever6 Hours
      if ((((Elapsed_Time % 21600000) < 300000)) &&( Elapsed_Time > 300000))
      {
        Serial.println("Flushing out the air.");
        Serial.print("I/O Fans and Air Thrower On, Compressor turned OFF |");
        Serial.print(hours);
        Serial.print(":");
        Serial.print(minutes);
        Serial.print(":");
        Serial.println(seconds);
        digitalWrite(IO_Fans, Relay_On);
        digitalWrite(Compressor, Relay_Off);
        digitalWrite(Air_Thrower, Relay_On);  
        delay(600000);
        digitalWrite(IO_Fans, Relay_Off);
      }

       delay(300000);
       Serial.println("Turning Compressor ON");
       digitalWrite(Compressor, Relay_On);
     }
 

  }
  //Update value every 1 sec.

          
  delay(1000);
}


int getState(float i){
  if (i > 0)
  {
    return 1;
  }

  return 0;
}


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
