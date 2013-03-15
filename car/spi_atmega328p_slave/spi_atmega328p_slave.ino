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

#define BUFSIZE 1
volatile unsigned char incoming[BUFSIZE];
volatile short int received=0;

// ------ l293d
#define en12 9
#define en34 6
#define a3 4
#define a4 3
#define a1 7
#define a2 8

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
  // Serial.write("Interrupt\n");
  incoming[received++] = received_from_spi(0x00);
  if (received == BUFSIZE || incoming[received-1] == 0x00 || incoming[received-1] == (unsigned char)255 ) {
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

void setup(){
  // Begin Serial
  Serial.begin(9600);
  // Init l293d
  l293d_init();
  // Init SPI
  spi_init();
  // Clear array
  int i = 0;
  for(i;i<BUFSIZE;i++){
    incoming[i] = 0;
  }
}

void loop(){
 //Serial.write("loop\n");
 delay(500); 
}

void parse_message(){
  int i = 0;
  // Serial.write("Parsing: \n");
  for(;i < received;i++){
    Serial.println((int)incoming[i]);
    switch(incoming[i]){
      case ((unsigned char)1):
        forward();
        break;
      case ((unsigned char)2):
        backward();
        break;
      case ((unsigned char)3):
        back_brake();
        break;
      case ((unsigned char)4):
        left();
        break;
      case ((unsigned char)5):
        right();
        break;
      case ((unsigned char)6):
        front_brake();
        break;
    }
  }
  // Serial.write("\nDone Parsing;\n");
}

// send a SPI message to the other device - 3 bytes then go back into 
// slave mode
void send_message()
{
  setup_spi(SPI_MODE_1, SPI_MSB, SPI_NO_INTERRUPT, SPI_MSTR_CLK8);
  if (SPCR & (1<<MSTR)) { // if we are still in master mode
    send_spi(0x02);
    send_spi(0x00);
  }
  setup_spi(SPI_MODE_1, SPI_MSB, SPI_INTERRUPT, SPI_SLAVE);
}

// INTERRUPT METHOD
/*
ISR(SPF_STC_vect){
  const char result[7] = SPDR;
  Serial.println("DATA: " + result);
}

// POLLING METHOD
unsigned char SPI_SlaveReceive(){
  // Shift everything out the 2 byte buffer
  // padding is zero
  while(!(SPSR & (1<<SPIF)));
  return SPDR;
}
*/



