#include <stdio.h>
#include <stdint.h>

uint32_t load32_be(uint8_t *p)
{
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
}

void store32_be(uint8_t *p, uint32_t n)
{
    // 0x12345678 -> {0x12, 0x34, 0x56, 0x78}
    p[0] = (uint8_t)(n >> 24);
    p[1] = (uint8_t)((n >> 16) & 0xFF);
    p[2] = (uint8_t)((n >> 8) & 0xFF);
    p[3] = (uint8_t)(n & 0xFF);
}

int main()
{
    uint8_t mem[4] = {0x12, 0x34, 0x56, 0x78};
    // uint8_t mem[4] = {0xca, 0xfe, 0xba, 0xbe};
    uint32_t v = load32_be(mem);
    uint8_t dump[4];
    store32_be(dump, v);

    printf("%08x\n", v);

    for (int i = 0; i < 4; i++)
    {
        printf("%02x ", dump[i]);
    }

    printf("\n");

    return 0;
}
