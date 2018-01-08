//int airT;
//int waterT;
//int Humidity;
//
//int lightI;

void setup() {
  Serial.begin(9600);
  
  // Humidity | EC | CO2 | pH
  
  delay(1000); // buffer fillup
  if (Serial.available() > 0){
    String data = collectData();
    Serial.print("received raw data ");
    Serial.print(data);
    // parse incoming space separated row containing constant variables
    float *val;
    val = parseData(data);
    Serial.print(" Humidity ");
    Serial.print(*val);
    Serial.print(" EC ");
    Serial.print(*(val+1));
    Serial.print(" CO2 ");
    Serial.print(*(val+2));
    Serial.print(" pH ");
    Serial.println(*(val+3));
    
    
    }
  else{
    Serial.print("nothing received");
    }

}

void loop() {
   // parse incoming space separated row containing dynamic variables
   // LightIntensity | AirTemperature
   // WaterTemperature (dependent variable) = AirTemperature - 4
   delay(1000); // buffer fillup
   if (Serial.available() > 0){
    String data = collectData();
//    String data;
//    while(Serial.available() > 0){
//              char c = Serial.read();
//              data += c;
//            }
    Serial.print("received ");
//    Serial.println(data);
    float *val;
    val = parseData(data);
    Serial.print("LightIntensity ");
    Serial.print(*val);
    Serial.print(" AirTemperature ");
    Serial.println(*(val+1));
    }
   
}

String collectData(){
  String data;
  while(Serial.available() > 0){
          char c = Serial.read();
          data += c;
        }
  return data;
  }

float * parseData(String data){
  static float res[10];
  // fill in the data in each element of the float array
  // data --> string type
  int i = 0, j = 0;
  String temp;
  
  while(i < data.length()){
    char c = data.charAt(i);
    String s;
    while(c != ' '){
      s += c;
      i++;
      // only access the next char if index is with String length
      if (i < data.length()){
        c = data.charAt(i);
        }
      else{
        c = ' ';
        }
      
      }
      
    res[j] = s.toFloat();
    j++;
    i++;
    }

  return res;
  }


