/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */
 
// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
#define en12 9
#define en34 6
#define a3 4
#define a4 3
#define a1 7
#define a2 8
 
// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pins as an outputs.
  pinMode(en12, OUTPUT);
  pinMode(en34, OUTPUT);
  pinMode(a3, OUTPUT);
  pinMode(a4, OUTPUT);
  pinMode(a1, OUTPUT);
  pinMode(a2, OUTPUT); 
}
 
// the loop routine runs over and over again forever:
void loop() {
  /*
  left_forward();   // turn the LED on (HIGH is the voltage level)
  delay(1000);               // wait for a second
  left_disable();
  delay(1000);
  left_backward();
  delay(1000);
  left_brake();
  delay(1000);
  */
  /*
  left_forward();
  right_forward();
 */
 //left_forward();  // links - rechts
 //right_forward(); // voor  - achter
  delay(1000);
}
 
void left_side(uint8_t pwm, uint8_t A1, uint8_t A2){
  analogWrite(en12, pwm);
  digitalWrite(a1, A1);
  digitalWrite(a2, A2);
}
 
void left_forward(void){
  left_side(255, HIGH, LOW);
}
 
void left_backward(void){
  left_side(255, LOW, HIGH);
}
 
void left_disable(void){
  left_side(0, LOW, LOW);
}
 
void left_brake(void){
  left_side(255, LOW, LOW);
}
 
void right_side(uint8_t pwm, uint8_t A1, uint8_t A2){
  analogWrite(en34, pwm);
  digitalWrite(a3, A1);
  digitalWrite(a4, A2);
}
 
void right_forward(void){
  right_side(255, HIGH, LOW);
}
 
void right_backward(void){
  right_side(255, LOW, HIGH);
}
 
void right_disable(void){
  right_side(0, LOW, LOW);
}
 
void right_brake(void){
  right_side(255, LOW, LOW);
}
