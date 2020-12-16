#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>


#define Buzzer PB4
#define Button PB3
#define output_switch PB1

#define EE_DISABLED B10100000
#define  EE_ENABLED B10100011

#define TIMER_RESET TCNT0 = 0
#define CLEAR_COMMAND curr_com_pos = 0
#define TOGGLE_OUTPUT PORTB ^= (1 << output_switch)

#define BIT_ZERO B00010010
#define  BIT_ONE B00100001
#define END_ZERO B00010100
#define  END_ONE B00100100

const uint8_t time_stones[4] = {3, 7, 18, 50};

uint8_t ee_record[4];

volatile uint8_t curr_com[4];
volatile uint8_t curr_com_pos = 0;
volatile uint8_t time_high, time_low, button_pressed, ee_recno;
volatile uint8_t update_mode = false;
volatile bool Open_Door=false;

void ee_rr(uint8_t addr) {

  for (uint8_t i = 0; i < 4; i++)
  {
    ee_record[i] = eeprom_read_byte((uint8_t*)addr + i);
  }
}

boolean ee_free_slot(void) {
  for (ee_recno = 0; ee_recno < 64; ee_recno += 4)
  {
    ee_rr(ee_recno);
    if (ee_record[3] != EE_ENABLED)
    {
      return true;
    }
  }
  return false;
}

boolean ee_find(void) {
  curr_com[3] = EE_ENABLED;
  for (ee_recno = 0; ee_recno < 64; ee_recno += 4)
  {
    ee_rr(ee_recno);
    for (uint8_t i = 0; i < 4; i++)
    {
      if (!(ee_record[i] == curr_com[i]))break;
      if (i == 3)return true;
    }
  }
  return false;
}

void buzz(unsigned int d) {

  d <<= 1;
  for (; d > 0; d--) {
    PORTB ^= (1 << Buzzer);
    for (uint8_t i = 200; i > 0; i--) {
      asm("NOP");    //assembly commadn NOP -> no operation
      asm("NOP");
    }
  }
  _delay_ms (100);
}

void eval_command(void) {
  cli();
  if (ee_find())
  {
    if (update_mode)
    {
      update_mode = false;
      eeprom_write_byte((uint8_t*)ee_recno + 3, EE_DISABLED);
      for (uint8_t i = 0; i < 3; i++) buzz(300);
    }
    else
    {
      Open_Door=true;
    }
  }
  else if (update_mode && ee_free_slot())
  {
    for (uint8_t i = 0; i < 4; i++)
    {
      eeprom_write_byte((uint8_t*)ee_recno + i, curr_com[i]);
    }
    update_mode = false;
    for (uint8_t i = 0; i < 2; i++) buzz(300);
  }
  CLEAR_COMMAND;
  sei();
}

void setup() {
  GIMSK = _BV (PCIE);             // Pin Change Interrupt Enable
  PCMSK = _BV (PCINT0);           // Enable external interrupts PCINT0
  MCUCR = _BV (ISC00);            // PCINT0 is triggered on any change

  TCCR0B |= (1 << CS02) | (1 << CS00); // timer prescaler set to 1024
  TIMSK0 |= 1 << TOIE0;           // enable timer overflow interrupt
  OSCCAL = 75;                          //ocilator calibration
  sei();                          //enable interrupts
  DDRB |= B00010010;              // Set  Buzzer and output_switch  as output
}

void loop() {
  button_pressed = 0;
  while (bit_is_clear(PINB, Button) && button_pressed < 120)
  {
    button_pressed++;
    _delay_ms (25);
  }
  if (button_pressed == 120)
  {
    cli();// disable interrupts
    update_mode = !update_mode;
    buzz(800);
    sei();// enable interrupts
  }
  else
  {
    if (button_pressed > 4 && !update_mode) {
      Open_Door=true;
    }
  }
  if(Open_Door){
          cli();// disable interrupts
          Open_Door=false;
          PORTB |= (1 << output_switch); // HIGH
          buzz(1500);
          PORTB &= ~(1 << output_switch); // LOW
          sei();// enable interrupts
  }
  else{
         PORTB &= ~(1 << output_switch); // LOW
  }
}

ISR(TIM0_OVF_vect) {
  CLEAR_COMMAND;
}

ISR (PCINT0_vect)               // Interrupt on PCINT0 vector
{
  uint8_t tmrval, is_high, pos_div_8, ziptime;
  is_high = bit_is_set(PINB, 0);
  tmrval = TCNT0;
  TIMER_RESET;
  pos_div_8 = curr_com_pos / 8;
  if (is_high) {
    time_low = tmrval;
    ziptime = 0;
    for (uint8_t i = 0; i < 4; i++)
    {
      if (time_low >= time_stones[i]) ziptime++;
      if (time_high >= time_stones[i]) ziptime += 16;
    }
    switch (ziptime)
    {
      case BIT_ZERO:
        curr_com[pos_div_8] <<= 1;
        curr_com_pos++;
        break;
      case BIT_ONE:
        curr_com[pos_div_8] <<= 1;
        curr_com[pos_div_8] |= B00000001;
        curr_com_pos++;
        break;
      case END_ZERO:
      case END_ONE:
        if (curr_com_pos >= 24)
        {
          eval_command();
        }
      default:
        CLEAR_COMMAND;
    }
  }
  else time_high = tmrval;
}
