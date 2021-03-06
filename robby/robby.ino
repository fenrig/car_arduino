#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
 
// ---- spi
#if (defined(__AVR_ATmega48__) || defined(_AVR_ATmega88__) || defined(__AVR_ATmega168__) || defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__))
 #define SPI_SS_PIN PORTB2
 #define SPI_SCK_PIN PORTB5
 #define SPI_MOSI_PIN PORTB3
 #define SPI_MISO_PIN PORTB4
#else
 #error unknown processor - add to spi.h
#endif
 
// SPI clock modes
#define SPI_MODE_0 0x00 /* Sample (Rising) Setup (Falling) CPOL=0, CPHA=0 */
#define SPI_MODE_1 0x01 /* Setup (Rising) Sample (Falling) CPOL=0, CPHA=1 */
#define SPI_MODE_2 0x02 /* Sample (Falling) Setup (Rising) CPOL=1, CPHA=0 */
#define SPI_MODE_3 0x03 /* Setup (Falling) Sample (Rising) CPOL=1, CPHA=1 */
 
// data direction
#define SPI_LSB 1 /* send least significant bit (bit 0) first */
#define SPI_MSB 0 /* send most significant bit (bit 7) first */
 
// whether to raise interrupt when data received (SPIF bit received)
#define SPI_NO_INTERRUPT 0
#define SPI_INTERRUPT 1
 
// slave or master with clock diviser
#define SPI_SLAVE 0xF0
#define SPI_MSTR_CLK4 0x00 /* chip clock/4 */
#define SPI_MSTR_CLK16 0x01 /* chip clock/16 */
#define SPI_MSTR_CLK64 0x02 /* chip clock/64 */
#define SPI_MSTR_CLK128 0x03 /* chip clock/128 */
#define SPI_MSTR_CLK2 0x04 /* chip clock/2 */
#define SPI_MSTR_CLK8 0x05 /* chip clock/8 */
#define SPI_MSTR_CLK32 0x06 /* chip clock/32 */
 
#define BUFSIZE 2
 
#define en12 5 // was 9
#define en34 6
#define a3 4
#define a4 3
#define a1 7
#define a2 8
 
#define roadwith 250
#define defaultspeed 150

#define IRPORT 5
int val = 0;
float voltage = 0;

#define logger 1

volatile unsigned char incomming[BUFSIZE];
volatile short int received=0;

volatile int pwm_right = 0;
volatile int pwm_left = 0;

// --- debugging
#define logger true
#if (defined(logger))
  boolean newoffset = true;
#endif
boolean globalstop = false;
int leftpwm = 0;
int rightpwm = 0;
// -------------
void pwm_init(void){
	/*
	// Non inverting-mode
	TCCR0A |= (1 << COM0A1);
	//TCCR0B |= (1 << COM0B1);
	// Set fast PWM-mode
	TCCR0A |= (1 << WGM01) | (1 << WGM00);
	TCCR0B |= (1 << CS01);
	*/
      DDRD |= (1 << DD5);
      DDRD |= (1 << DD6);
      TCCR0A |= (1 << WGM01) | (1 << WGM00);
      OCR0A = 0;
      OCR0B = 0;
      TCCR0A |= (1 << COM0A1);
      TCCR0A |= (1 << COM0B1);
      TCCR0B = (1 << 2) | (0 << 1) | (1 << 0); 
      
	return;
}

void pwm_write(char pin, char duty_cycle){
	// set PWM duty cycle
	if(pin == 5){
		OCR0A = duty_cycle;
	}else if(pin == 6){
		OCR0B = duty_cycle;
	}else{
		return;
	}
	
}

void setup_spi(uint8_t mode, int dord, int interrupt, uint8_t clock)
{
  // specify pin directions for SPI pins on port B
  if (clock == SPI_SLAVE) { // if slave SS and SCK is input
    DDRB &= ~(1<<SPI_MOSI_PIN); // input
    DDRB |= (1<<SPI_MISO_PIN); // output
    DDRB &= ~(1<<SPI_SS_PIN); // input
    DDRB &= ~(1<<SPI_SCK_PIN);// input
  } else {
    DDRB |= (1<<SPI_MOSI_PIN); // output
    DDRB &= ~(1<<SPI_MISO_PIN); // input
    DDRB |= (1<<SPI_SCK_PIN);// output
    DDRB |= (1<<SPI_SS_PIN);// output
  }
  SPCR = ((interrupt ? 1 : 0)<<SPIE) // interrupt enabled
    | (1<<SPE) // enable SPI
    | (dord<<DORD) // LSB or MSB
    | (((clock != SPI_SLAVE) ? 1 : 0) <<MSTR) // Slave or Master
    | (((mode & 0x02) == 2) << CPOL) // clock timing mode CPOL
    | (((mode & 0x01)) << CPHA) // clock timing mode CPHA
    | (((clock & 0x02) == 2) << SPR1) // cpu clock divisor SPR1
    | ((clock & 0x01) << SPR0); // cpu clock divisor SPR0
  SPSR = (((clock & 0x04) == 4) << SPI2X); // clock divisor SPI2X
}
 
void disable_spi()
{
  SPCR = 0;
}
 
uint8_t send_spi(uint8_t out)
{
  SPDR = out;
  while (!(SPSR & (1<<SPIF)));
  return SPDR;
}
 
uint8_t received_from_spi(uint8_t data)
{
  SPDR = data;
  return SPDR;
}

ISR(SPI_STC_vect)
{  
  incomming[received++] = received_from_spi(0x00);
  if (received == BUFSIZE || incomming[received-1] == 0x00 || incomming[received-1] == (unsigned char)255 ) {
      parse_message();
      received = 0;
   }
}
// -----------------
void spi_init(void){
  setup_spi(SPI_MODE_0, SPI_MSB, SPI_INTERRUPT,SPI_SLAVE);
}
 
void l293d_init(void){
  // initialize the digital pins as an outputs.
  pinMode(en12, OUTPUT);
  pinMode(en34, OUTPUT);
  pinMode(a3, OUTPUT);
  pinMode(a4, OUTPUT);
  pinMode(a1, OUTPUT);
  pinMode(a2, OUTPUT); 
}
 
void right_forward(int pwm){
  right_side(pwm, HIGH, LOW);
}
void left_forward(int pwm){
  left_side(pwm, HIGH, LOW);
}
void right_backward(int pwm){
  right_side(pwm, LOW, HIGH);
}
void left_backward(int pwm){
  left_side(pwm, LOW, HIGH);
}
void right_disable(int pwm){
  right_side(0, LOW, LOW);
}
void left_disable(int pwm){
  left_side(0, LOW, LOW);
}
void right_brake(int pwm){
  right_side(pwm, LOW, LOW);
}
void left_brake(int pwm){
  left_side(pwm, LOW, LOW);
}
 
void left_side(uint8_t pwm, uint8_t A1, uint8_t A2){
  pwm_left = pwm;
  //analogWrite(en12, pwm);
  pwm_write(en12, pwm);
  digitalWrite(a1, A1);
  digitalWrite(a2, A2);
}
 
void right_side(uint8_t pwm, uint8_t A1, uint8_t A2){
  pwm_right = pwm;
  //analogWrite(en34, pwm);
  pwm_write(en34, pwm);
  digitalWrite(a3, A1);
  digitalWrite(a4, A2);
}

void setup(void) {
  // Begin Serial
  #if (defined(logger))
    Serial.begin(9600);
  #endif
  // Init PWM
  Serial.print(TCCR0B);
  pwm_init();
  Serial.print(TCCR0B);
  // Init l293d
  l293d_init();
  // Init SPI
  spi_init();
  // Clear array
  int i = 0;
  for(i;i<BUFSIZE;i++){
    incomming[i] = 0;
  }
}
 
void loop(){
  int i = 0;
  for(i = 5; i < 255; i--) i += 19;
 //Serial.write("loop\n");
 /*
 Serial.print("Right side: ");
 Serial.print(pwm_right);
 Serial.print(" - ");
 Serial.print("Left side: ");
 Serial.print(pwm_left);
 Serial.println(" ");*/
 /*
 #if (defined(logger))
   if(newoffset){
    String* ptr = new String("Offset: ");
    *ptr = *ptr + incomming[0] + " - " + incomming[1] + " -> " + String(leftpwm) + " - " + String(rightpwm);
    Serial.println(*ptr);
    delete(ptr);
    newoffset = false;
   }
 #endif
 val = analogRead(IRPORT);    // read the input pin
 voltage = val * (5.0 / 1023.0); // convert to voltage
 #if (defined(IRLOGGER))
    Serial.println(voltage); 
 #endif
 if(voltage > 2.0){
  globalstop = true;
  Serial.println("Stopped, close object found");
  right_brake(255);
  left_brake(255);
  leftpwm = rightpwm = 255;
 }else{
  globalstop = false;
 }
 */
}
 
void parse_message(){
  int i = 0;
  int pwm = 0; //must be created in offset function
  /*Serial.print("i: ");
  Serial.println(incomming[i]);
  Serial.print("i+1: ");
  Serial.println(incomming[i+1]);*/
 
 
  // Serial+.write("Parsing: \n");
    int left = (int)incomming[0];
    int right = (int)incomming[1];
    if(left )
    if(/*left == 250 && right == 250 ||*/ globalstop) //two default offsets (250) => no road so stop
    {
        right_brake(255);
        left_brake(255);
        leftpwm = rightpwm = 255;
    }
    else if(left != 250 && right != 250) //two offsets
    {
        //find the smallest offset, more power on that side
        if(left < right)
        {
          // OffsetToPwmRight(right);
          //OffsetToPwmLeft(right);
          aansturing(left, right);

        }
        else if(left > right)
        {
          aansturing(left, right);
          // OffsetToPwmLeft(left);
          // OffsetToPwmRight(left);
        }
        else if(left == right)
        {
          left_forward(defaultspeed);
          rightpwm = defaultspeed;
          right_forward(defaultspeed);
          leftpwm = defaultspeed;
        }
    }
    else if(left == 250)
    {
      //OffsetToPwmRight(right);
      aansturing(left, right);
    }
    else if(right==250)
    {
      //OffsetToPwmLeft(left);
      aansturing(left, right);
    }
    #if (defined(logger))
      newoffset = true;
    #endif
  // Serial.write("\nDone Parsing;\n");
}

int algoritme(int offset){
  /* Hier bewerkingen doen in verband met lijnvolg algoritme */
  if(offset < 5)
    return 1;
  //if(offset > 20)
  //  return (offset / 2);
  /*
  if(offset > 25)
    return (offset / 3);
  */
  return (offset / 2);
}


void OffsetToPwmLeft(int OffLeft)
{
 int OffRight,Offset,pwm;
 OffRight = roadwith - OffLeft;
 Offset = OffLeft - OffRight;
 Offset = abs(Offset);
 /*
 //rechter kant op init waarde brengen (150)
 right_forward(defaultspeed);
 //linker kant met juiste waarde verhogen
 pwm = algoritme(Offset);
 left_forward(defaultspeed + pwm);
 leftpwm = defaultspeed + pwm;
 rightpwm = defaultspeed;
 */
 aansturing(OffLeft, OffRight);
}
void OffsetToPwmRight(int OffRight)
{
 int OffLeft;
 int Offset,pwm;
 OffLeft = roadwith - OffRight; 
 Offset = OffLeft - OffRight;
 Offset = abs(Offset);
 /*
  //OFFSET NEGATIEF => NAAR left RIJDEN
  //Linker kant op initiele waar de plaatsen (150)
  left_forward(defaultspeed);
  //Rechter kant met juiste waarde laten versnellen
  pwm = algoritme(Offset);
  right_forward(defaultspeed+pwm);
  rightpwm = defaultspeed + pwm;
  leftpwm = defaultspeed;
  */
  aansturing(OffLeft, OffRight);
}
// send a SPI message to the other device - 3 bytes then go back into 
// slave mode
void send_message(){
  setup_spi(SPI_MODE_1, SPI_MSB, SPI_NO_INTERRUPT, SPI_MSTR_CLK8);
  if (SPCR & (1<<MSTR)) { // if we are still in master mode
    send_spi(0x02);
    send_spi(0x00);
  }
  setup_spi(SPI_MODE_1, SPI_MSB, SPI_INTERRUPT, SPI_SLAVE);
}

void aansturing(int OffLeft, int OffRight){
  int overschot = 0;
  
  if(OffLeft < 126){
    overschot = 125 - OffLeft;
    OffLeft = OffLeft + overschot;
    if(OffRight != 250)
      OffRight = OffRight + overschot;
  }
  if(OffRight < 126){
    overschot = 125 - OffRight;
    if(OffLeft != 250)
      OffLeft = OffLeft + overschot;
    OffRight = OffRight + overschot;
  }
  if(OffRight == 250) OffRight = 125;
  if(OffLeft == 250) OffLeft = 125;
  if(OffRight > 175) OffRight = 175;
  if(OffLeft > 175) OffLeft = 175;
  left_forward(OffRight);
  right_forward(OffLeft);
  leftpwm = OffRight;
  rightpwm = OffLeft;
}
