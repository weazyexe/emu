#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct chip8_t
{
    uint8_t mem[4096];
    uint8_t V[16];
    uint16_t I;
    uint16_t pc;
    uint16_t stack[16];
    uint8_t sp;
    uint8_t delay, sound;
    uint8_t display[32][64];
    uint8_t keys[16];
};

void draw(struct chip8_t *chip8, uint8_t vx, uint8_t vy, uint8_t n)
{
    uint8_t x0 = chip8->V[vx] % 64;
    uint8_t y0 = chip8->V[vy] % 32;
    chip8->V[0xF] = 0;

    for (int r = 0; r < n; r++)
    {
        int row = chip8->mem[chip8->I + r];
        int y = y0 + r;
        if (y >= 32)
            break;
        for (int b = 0; b < 8; b++)
        {
            int x = x0 + b;
            if (x >= 64)
                break;
            uint8_t bit = (row >> (7 - b)) & 1;
            if (chip8->display[y][x] == 1 && bit == 1)
            {
                chip8->V[0xF] = 1;
            }
            chip8->display[y][x] ^= bit;
        }
    }
}

void display(struct chip8_t *chip8)
{
    for (int y = 0; y < 32; y++)
    {
        for (int x = 0; x < 64; x++)
        {
            if (chip8->display[y][x] == 1)
            {
                printf("#");
            }
            else
            {
                printf(" ");
            }
        }
        printf("\n");
    }
}

int main(int argc, char **argv)
{
    static struct chip8_t chip8;
    chip8.pc = 0x200;
    chip8.sp = 0;
    chip8.delay = 0;
    chip8.sound = 0;

    if (argc < 2)
    {
        fprintf(stderr, "usage: chip8 </path/to/ch8>\n");
        return 1;
    }

    char *image_path = argv[1];
    FILE *image_file = fopen(image_path, "rb");
    if (image_file == NULL)
    {
        printf("failed to open ch8 at %s\n", image_path);
        return 1;
    }

    fseek(image_file, 0, SEEK_END);
    size_t file_size = ftell(image_file);
    rewind(image_file);

    if (file_size <= 0)
    {
        fprintf(stderr, "file is empty\n");
        fclose(image_file);
        return 1;
    }

    if (file_size > 0x1000 - 0x200)
    {
        fprintf(stderr, "image is too big\n");
        fclose(image_file);
        return 1;
    }

    size_t bytes_read = fread(chip8.mem + 0x200, 1, file_size, image_file);
    if (bytes_read != file_size)
    {
        fprintf(stderr, "wtf\n");
        fclose(image_file);
        return 1;
    }

    int running = 1;
    while (running)
    {
        uint16_t op = (uint16_t)(chip8.mem[chip8.pc]) << 8 | (uint16_t)(chip8.mem[chip8.pc + 1]);
        chip8.pc += 2;

        uint8_t op_code = (op >> 12) & 0xF;
        uint16_t n = op & 0xF;
        uint16_t nn = op & 0xFF;
        uint16_t nnn = op & 0xFFF;
        uint8_t x = (op >> 8) & 0xF;
        uint8_t y = (op >> 4) & 0xF;

        switch (op_code)
        {
        case 0x0:
            if (op == 0x00e0)
            {
                memset(chip8.display, 0, sizeof chip8.display);
            }
            else
            {
                fprintf(stderr, "unknown opcode %04x at pc %04x\n", op, chip8.pc - 2);
                return 1;
            }
            break;
        case 0x1:
            if (chip8.pc - 2 == nnn)
            {
                running = 0;
            }
            else
            {
                chip8.pc = nnn;
            }
            break;
        case 0x6:
            chip8.V[x] = nn;
            break;
        case 0x7:
            chip8.V[x] += nn;
            break;
        case 0xa:
            chip8.I = nnn;
            break;
        case 0xd:
            draw(&chip8, x, y, n);
            break;
        default:
            fprintf(stderr, "unknown opcode %04x at pc %04x\n", op, chip8.pc - 2);
            return 1;
        }
    }

    display(&chip8);
    fclose(image_file);
    return 0;
}