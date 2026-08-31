#include "io_calls.h"
#include "ps2_8042_asm.h"
#include <stdint.h>

#define RW_DATA_PORT 0x60       // Read write IO data buffers
#define R_STATUS_REGISTER 0x64  // Read status register
#define W_COMMAND_REGISTER 0x64 // Write to command register

// COMMANDS
#define R_CCB 0x20 // Read Controller Configuration Byte
#define W_CCB 0x60 // Write Controller Configuration Byte

static void PS2_Disable_Translation() {
  wait_input_buffer_empty();
  outb(W_COMMAND_REGISTER, R_CCB); // Puts CCB in output data buffer.

  wait_output_buffer_full();
  uint8_t ccb = inb(RW_DATA_PORT); // Reading from output buffer
  ccb = ccb & 0b10111111;          // setting the translation bit to 0

  wait_input_buffer_empty();
  outb(W_COMMAND_REGISTER,
       W_CCB); // write the data that will be inserted in input buffer to CCB
  wait_input_buffer_empty();
  outb(RW_DATA_PORT, ccb); // The data to be written to CCB
}

void PS2_Init() { PS2_Disable_Translation(); }
