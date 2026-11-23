/**
* NES To USB
* By Adrian Schiller
*/

#include "pins_arduino.h"

// Pin which the controller listens to, for the query command from the console
#define NES_COMMAND_PIN 2

// Pin which carries the pulse signal, while writing data
#define NES_PULSE_PIN 7

// Pin which contains the actual controller data
#define NES_DATA_PIN 4

// For sending the console's latch command to the controller. 
// Both commands aledgedly translate to 1 opt code, taking 2 cycles
#define NES_COMMAND_HIGH PORTD |= 0x04
#define NES_COMMAND_LOW PORTD &= ~0x04

// For emitting the pulse from the console to the controller
// Like above, each commands translate to 1 opt code, 2 cycles
#define NES_PULSE_HIGH (PORTD |= 0x80)
#define NES_PULSE_LOW (PORTD &= ~0x80)

// For getting the response from the controller
// Translates to 1 opt code, 2 cycles.
#define NES_DATA_QUERY (PIND & 0x10)

struct state {
  // A, B, Select, Start, Up, Down, Left, Right
  unsigned char buttons;
} NES_state;
char NES_raw_dump[8]; // Note: only uses 1 bit for each of the 8 bytes; could be improved

void print_binary(char a){
    int i;
    
    for (i = 0; i < 8; i++) {
         Serial.print(!!((a << i) & 0x80));
    }
    
     Serial.println("");
}

void translate(){
  // Zero out any previous values of NES_state
  memset(&NES_state, 0, sizeof(NES_state));

  for(auto i = 0; i < 8; i++){
    //Note: the values are inverted; "high" means the button is not pressed
    if(!(NES_raw_dump[i] & 0x10)){
      // 0x80 is 1000 0000, and it's shifted `i` times to the right
      // then the binary value of buttons is OR'd with the result, flipping the bit to 1
      NES_state.buttons |= (0x80 >> i);  
    }   
  }
}

void NES_send(){
  // Begin sending the "latch" command from NES -> Controller
  NES_COMMAND_HIGH;

  // block 12us 
  // each row is 16 NOPs, each NOP use 1 cycle - Arduino Uno operates at 16Mhz, or 16 operations per 1us.
  // therefore, 16 NOPs takes 1us, 12 rows makes 12us.
  asm volatile(
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    );
    // Beautiful.

    NES_COMMAND_LOW;
}

void NES_get(){
  uint8_t timeout = 0;
  // Before first pulse, get 'a'
  NES_raw_dump[0] = NES_DATA_QUERY;

  // Wait 6us
  asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
  asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
  asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
  asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
  asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
  asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");

  for(int i = 1; i < 8; i++){
    
    NES_PULSE_HIGH;

    // Wait 6us
    asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
    asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
    asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
    asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
    asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
    asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"); // shave off 2us for the query command, I guess.

    NES_raw_dump[i] = NES_DATA_QUERY;

    NES_PULSE_LOW;
    
    // Wait 6us
    asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
    asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
    asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
    asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
    asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
    asm volatile("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n");
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  
  // Not quite sure how much of this is necessary, but I don't want to accidently send high voltage on
  // these signal lines...
  digitalWrite(NES_COMMAND_PIN, LOW);  
  digitalWrite(NES_PULSE_PIN, LOW);  
  digitalWrite(NES_DATA_PIN, LOW);  

  pinMode(NES_COMMAND_PIN, OUTPUT);
  pinMode(NES_PULSE_PIN, OUTPUT);
  pinMode(NES_DATA_PIN, INPUT);
}

void printData(){
  Serial.println("=====");
  Serial.print("A: ");
  Serial.println(NES_state.buttons & 0x80 ? 1:0);
  Serial.print("B: ");
  Serial.println(NES_state.buttons & 0x40 ? 1:0);
  Serial.print("Select: ");
  Serial.println(NES_state.buttons & 0x20 ? 1:0);
  Serial.print("Start: ");
  Serial.println(NES_state.buttons & 0x10 ? 1:0);

  Serial.print("Up: ");
  Serial.println(NES_state.buttons & 0x08 ? 1:0);
  Serial.print("Down: ");
  Serial.println(NES_state.buttons & 0x04 ? 1:0);
  Serial.print("Left: ");
  Serial.println(NES_state.buttons & 0x02 ? 1:0);
  Serial.print("Right: ");
  Serial.println(NES_state.buttons & 0x01 ? 1:0);
  Serial.println("=====");
  Serial.println("");
}

void loop() {
  noInterrupts();
  
  NES_send();
  NES_get();
  
  interrupts();

  translate();

  // printData();
  Serial.write((uint8_t*) &NES_state, 1);

  delay(10);
}
