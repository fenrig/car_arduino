int analogPin = 5;     // potentiometer wiper (middle terminal) connected to analog pin 3
                       // outside leads to ground and +5V
int val = 0;           // variable to store the value read
float voltage = 0;

void setup()
{
  Serial.begin(9600);          //  setup serial
}

void loop()
{
  val = analogRead(analogPin);    // read the input pin
  voltage = val * (5.0 / 1023.0);
  Serial.println(voltage);             // debug value
}