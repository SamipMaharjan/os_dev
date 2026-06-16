// Header guard to prevent this header file from being included more than once
// in a single compilation
#pragma once

void putc(char c);
void puts(const char *str);
void _cdecl printf(const char *fmt, ...);
void printRawBytes(const char *buffer, int no_of_bytes);
