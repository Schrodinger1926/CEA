
// This is simple P-channel power mosfet test on test-led (12V)
int lights = 22;
int i = 0;
void setup() {
  pinMode(3,OUTPUT);
  for(int j = 0; j < 4; j++){
    pinMode(lights + j, OUTPUT);
    }

   for(int j = 0; j < 4; j++){
    digitalWrite(lights + j, LOW);
    }
}

void loop() {
  // put your main code here, to run repeatedly:
  analogWrite(3, i%255);
  delay(500);
  i = i + 5;
  
  

}
