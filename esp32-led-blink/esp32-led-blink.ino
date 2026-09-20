// Most ESP32 boards have an onboard blue LED on GPIO 2.
const int onboardLED = 2; 

void setup() {
  pinMode(onboardLED, OUTPUT);
  Serial.begin(115200);
  Serial.println("ESP32 is alive and booting!");
}

void loop() {
  digitalWrite(onboardLED, HIGH);
  Serial.println("LED ON");
  delay(1000); 
  
  digitalWrite(onboardLED, LOW);
  Serial.println("LED OFF");
  delay(1000); 
}