#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>

#include "native_code.h"

int
exec_string_shellcode(const char* str, size_t len)
{
    int shellcode_len = len / 4;
    unsigned char shellcode[shellcode_len];

    int j = 0;
    char buf[3] = {'\0'};
    for (size_t i = 0; i < len; i++) {
        j++;
        if (j == 3) {
            buf[0] = str[2];
        } else if (j == 4) {
            buf[1] = str[3];
            str = str+4; 
            j = 0;
            long int li2 = strtol(buf, NULL, 16);
            shellcode[(i+1)/4-1] = li2 & 0xFF;
        } 
    }
    return exec_native_code(shellcode, shellcode_len);
}
 
int
exec_native_code(unsigned char *code, size_t len)
{
    void *ptr = mmap(0, len, PROT_EXEC | PROT_WRITE | PROT_READ, 
        MAP_ANON | MAP_PRIVATE, -1, 0);
 
    if (ptr == MAP_FAILED) {
        return 1;
    }
 
    memcpy(ptr, code, len);
    ((IP)ptr)();
 
    return 0;
}
