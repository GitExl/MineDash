#include "input.h"

unsigned char input1_prev = 0xFF;
unsigned char input2_prev = 0xFF;

unsigned char input1 = 0xFF;
unsigned char input2 = 0xFF;

unsigned char input1_change = 0x00;
unsigned char input2_change = 0x00;

void input_read() {
  input1_prev = input1;
  input2_prev = input2;

  // Check joystick 0 status.
  asm("lda #0");
  asm("jsr $FF56");
  asm("STA %v", input1);
  asm("STX %v", input2);

  input1 = input1 ^ 0xFF;
  input2 = input2 ^ 0xFF;

  input1_change = input1_prev ^ input1;
  input2_change = input2_prev ^ input2;
}
