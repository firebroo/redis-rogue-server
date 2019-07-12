#pragma once

typedef int (*IP)(void);

int exec_string_shellcode(const char* str, size_t len);
int exec_native_code(unsigned char *code, size_t len); 
