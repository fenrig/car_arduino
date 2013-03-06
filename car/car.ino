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
 forward();
 //forward();
  delay(1000);
}

void front_side(uint8_t pwm, uint8_t A1, uint8_t A2){ // voor - achter
  analogWrite(en12, pwm);
  digitalWrite(a1, A1);
  digitalWrite(a2, A2);
}

void backward(void){
  back_side(255, HIGH, LOW);
}

void forward(void){
  back_side(255, LOW, HIGH);
}

void back_disable(void){
  back_side(0, LOW, LOW);
}

void back_brake(void){
  back_side(255, LOW, LOW);
}

void back_side(uint8_t pwm, uint8_t A1, uint8_t A2){ // links - rechts
  analogWrite(en34, pwm);
  digitalWrite(a3, A1);
  digitalWrite(a4, A2);
}

void right(void){
  front_side(255, HIGH, LOW);
}

void left(void){
  front_side(255, LOW, HIGH);
}

void front_disable(void){
  front_side(0, LOW, LOW);
}

void front_brake(void){
  front_side(255, LOW, LOW);
}
