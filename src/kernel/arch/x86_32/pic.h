#include <stdint.h>
void PIC_sendEOI(uint8_t irq);
void PIC_remap(int offset1, int offset2);
uint16_t pic_get_isr();
uint16_t pic_get_irr();
uint16_t pic_get_imr();
void PIC_clear_mask(uint8_t irq_line);
void PIC_set_mask(uint8_t irq_line);
void PIC_disable();
