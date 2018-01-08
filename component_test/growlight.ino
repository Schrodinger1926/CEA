int pin = 22;

void setup() {
  
  for(int i=0; i < 4; i++){
    digitalWrite(pin + i, HIGH);
    }
  
  for(int i=0; i < 4; i++){
    pinMode(pin + i, OUTPUT);
    }

}

void loop() {

   for(int i=0; i < 4; i++){
    digitalWrite(pin + i, LOW);
    }

   delay(1000);
   for(int i=0; i < 4; i++){
    digitalWrite(pin + i, HIGH);
    }
   delay(1000);
   
}
