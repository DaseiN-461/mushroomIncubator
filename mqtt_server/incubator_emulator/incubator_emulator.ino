int count = 0;
void setup() {
  Serial.begin(115200);
  delay(1000);
}

void loop() {
  
  Serial.printf("%i,%i,%i,%i\n",count,count+1,count+2,count+3);
  count++;
  delay(1000);

}
