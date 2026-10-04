#include <stdint.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    uint32_t x = 0x12345678;
    uint8_t *p = (uint8_t *)&x;
    printf("%02x\n", *p);
    printf("%02x\n", *(p + 1));
    printf("%02x\n", *(p + 2));
    printf("%02x\n", *(p + 3));
    printf("\n");
    return 0;
}