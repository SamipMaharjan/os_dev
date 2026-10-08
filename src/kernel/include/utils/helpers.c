#include "helpers.h"
#include <stdint.h>
#define PAGE_SHIFT 12

void halt() { __asm__ volatile("hlt"); }
