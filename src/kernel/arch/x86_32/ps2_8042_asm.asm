global wait_input_buffer_empty
wait_input_buffer_empty: 
  in al, 0x64 ; read status register 
  and al, 0b00000010
  cmp al, 0b00000000 ; see if input bit is 0 
  jnz wait_input_buffer_empty ; if not zero then keep looping this.
  ret ; if zero then return

global wait_input_buffer_full
wait_input_buffer_full: 
  in al, 0x64 ; read status register 
  and al, 0b00000010
  cmp al, 0b00000010 ; see if input bit is 1 
  jnz wait_input_buffer_empty ; if not 1 then keep waiting this.
  ret ; if 1 then return

global wait_output_buffer_empty
wait_output_buffer_empty: 
  in al, 0x64 ; read status register 
  and al, 0b00000001
  cmp al, 0b00000000 ; see if output bit is 0 
  jnz wait_input_buffer_empty ; if not 0  then keep waiting this.
  ret ; if zero then return

global wait_output_buffer_full
wait_output_buffer_full: 
  in al, 0x64 ; read status register 
  and al, 0b00000001
  cmp al, 0b00000001 ; see if output bit is 1 
  jnz wait_input_buffer_empty ; if not 1 then keep waiting this.
  ret ; if 1 then return
