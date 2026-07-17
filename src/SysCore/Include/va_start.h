/* &(LASTARG) points to the LEFTMOST argument of the function call
(before the ...) */
#define va_start(AP, LASTARG) (AP = ((va_list) & (LASTARG) + VA_SIZE(LASTARG)))
