#pragma once

#include "stdint.h"

uint32_t align(uint32_t number, uint32_t alignTo);
uint32_t far_pointer_to_linear_address(void far *farPointer);
void breakpoint();
