#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define RESULT_OK 1
#define RESULT_HALT 0
#define RESULT_FAIL_UNKNOWN_OPCODE -1
#define RESULT_FAIL_EMPTY_STACK -2
#define RESULT_FAIL_STACK_OVERFLOW -3
#define RESULT_FAIL_MEMORY_OUT_OF_BOUNDS -4

#define len(x) (sizeof(x) / sizeof((x)[0]))

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

uint16_t get_current_op(struct chip8_t *chip8)
{
    return (uint16_t)(chip8->mem[chip8->pc]) << 8 | (uint16_t)(chip8->mem[chip8->pc + 1]);
}

void clr(struct chip8_t *chip8)
{
    memset(chip8->display, 0, sizeof chip8->display);
}

int ret(struct chip8_t *chip8)
{
    if (chip8->sp == 0)
    {
        return RESULT_FAIL_EMPTY_STACK;
    }

    chip8->sp--;
    chip8->pc = chip8->stack[chip8->sp];

    return RESULT_OK;
}

int jump(struct chip8_t *chip8, uint16_t nnn)
{
    if (chip8->pc - 2 == nnn)
    {
        return RESULT_HALT;
    }

    chip8->pc = nnn;
    return RESULT_OK;
}

void assign(struct chip8_t *chip8, uint8_t x, uint16_t nn)
{
    chip8->V[x] = nn;
}

void increment(struct chip8_t *chip8, uint8_t x, uint16_t nn)
{
    chip8->V[x] += nn;
}

void set_image(struct chip8_t *chip8, uint16_t nnn)
{
    chip8->I = nnn;
}

int draw(struct chip8_t *chip8, uint8_t vx, uint8_t vy, uint8_t n)
{
    if (chip8->I + n - 1 > 0xFFF)
    {
        return RESULT_FAIL_MEMORY_OUT_OF_BOUNDS;
    }

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

    return RESULT_OK;
}

int call(struct chip8_t *chip8, uint16_t nnn)
{
    if (chip8->sp >= len(chip8->stack))
    {
        return RESULT_FAIL_STACK_OVERFLOW;
    }
    chip8->stack[chip8->sp] = chip8->pc;
    chip8->sp++;
    chip8->pc = nnn;

    return RESULT_OK;
}

void skip_next_eq_nn(struct chip8_t *chip8, uint8_t x, uint16_t nn)
{
    if (chip8->V[x] == nn)
    {
        chip8->pc += 2;
    }
}

void skip_next_neq_nn(struct chip8_t *chip8, uint8_t x, uint16_t nn)
{
    if (chip8->V[x] != nn)
    {
        chip8->pc += 2;
    }
}

void skip_next_eq_xy(struct chip8_t *chip8, uint8_t x, uint8_t y)
{
    if (chip8->V[x] == chip8->V[y])
    {
        chip8->pc += 2;
    }
}

void skip_next_neq_xy(struct chip8_t *chip8, uint8_t x, uint8_t y)
{
    if (chip8->V[x] != chip8->V[y])
    {
        chip8->pc += 2;
    }
}

int maths(struct chip8_t *chip8, uint8_t x, uint8_t y, uint8_t n)
{
    switch (n)
    {
    case 0x0:
    {
        chip8->V[x] = chip8->V[y];
        break;
    }

    case 0x1:
    {
        chip8->V[x] |= chip8->V[y]; // quirk
        break;
    }
    case 0x2:
    {
        chip8->V[x] &= chip8->V[y]; // quirk
        break;
    }
    case 0x3:
    {
        chip8->V[x] ^= chip8->V[y]; // quirk
        break;
    }
    case 0x4:
    {
        int sum = chip8->V[x] + chip8->V[y];
        chip8->V[x] = sum;
        chip8->V[0xF] = sum > 0xFF;
        break;
    }
    case 0x5:
    {
        uint8_t sub = chip8->V[x] - chip8->V[y];
        uint8_t vf = chip8->V[x] >= chip8->V[y];
        chip8->V[x] = sub;
        chip8->V[0xF] = vf;
        break;
    }
    case 0x6:
    {
        uint8_t shifted_bit = chip8->V[x] & 1;
        uint8_t res = chip8->V[x] >> 1;
        chip8->V[x] = res;
        chip8->V[0xF] = shifted_bit; // quirk
        break;
    }
    case 0x7:
    {
        uint8_t sub = chip8->V[y] - chip8->V[x];
        uint8_t vf = chip8->V[y] >= chip8->V[x];
        chip8->V[x] = sub;
        chip8->V[0xF] = vf;
        break;
    }
    case 0xE:
    {
        uint8_t shifted_bit = (chip8->V[x] >> 7) & 1;
        uint8_t res = chip8->V[x] << 1;
        chip8->V[x] = res;
        chip8->V[0xF] = shifted_bit; // quirk
        break;
    }

    default:
        return RESULT_FAIL_UNKNOWN_OPCODE;
    }

    return RESULT_OK;
}

int load_store(struct chip8_t *chip8, uint8_t x, uint16_t nn)
{
    switch (nn)
    {
    case 0x1e:
        chip8->I += chip8->V[x];
        break;
    case 0x55:
        if (chip8->I + x > 0xFFF)
        {
            return RESULT_FAIL_MEMORY_OUT_OF_BOUNDS;
        }
        for (int i = 0; i <= x; i++)
        {
            chip8->mem[chip8->I + i] = chip8->V[i]; // quirk
        }
        break;
    case 0x65:
        if (chip8->I + x > 0xFFF)
        {
            return RESULT_FAIL_MEMORY_OUT_OF_BOUNDS;
        }
        for (int i = 0; i <= x; i++)
        {
            chip8->V[i] = chip8->mem[chip8->I + i]; // quirk
        }
        break;

    default:
        return RESULT_FAIL_UNKNOWN_OPCODE;
    }
    return RESULT_OK;
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
    while (running == RESULT_OK)
    {
        uint16_t op = get_current_op(&chip8);
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
                clr(&chip8);
            }
            else if (op == 0x00ee)
            {
                running = ret(&chip8);
            }
            else
            {
                running = RESULT_FAIL_UNKNOWN_OPCODE;
            }
            break;
        case 0x1:
            running = jump(&chip8, nnn);
            break;
        case 0x6:
            assign(&chip8, x, nn);
            break;
        case 0x7:
            increment(&chip8, x, nn);
            break;
        case 0xa:
            set_image(&chip8, nnn);
            break;
        case 0xd:
            running = draw(&chip8, x, y, n);
            break;
        case 0x2:
            running = call(&chip8, nnn);
            break;
        case 0x3:
            skip_next_eq_nn(&chip8, x, nn);
            break;
        case 0x4:
            skip_next_neq_nn(&chip8, x, nn);
            break;
        case 0x5:
            skip_next_eq_xy(&chip8, x, y);
            break;
        case 0x9:
            skip_next_neq_xy(&chip8, x, y);
            break;
        case 0x8:
            running = maths(&chip8, x, y, n);
            break;
        case 0xF:
            running = load_store(&chip8, x, nn);
            break;
        default:
            running = RESULT_FAIL_UNKNOWN_OPCODE;
            break;
        }
    }

    // fprintf(stderr, "%02x\n", chip8.V[0]);
    display(&chip8);
    fclose(image_file);

    chip8.pc -= 2;
    switch (running)
    {
    case RESULT_HALT:
        return 0;
    case RESULT_FAIL_UNKNOWN_OPCODE:
        fprintf(stderr, "unknown opcode %04x at pc %04x\n", get_current_op(&chip8), chip8.pc);
        return 1;
    case RESULT_FAIL_EMPTY_STACK:
        fprintf(stderr, "empty stack at pc %04x\n", chip8.pc);
        return 1;
    case RESULT_FAIL_STACK_OVERFLOW:
        fprintf(stderr, "stack overflow at pc %04x\n", chip8.pc);
        return 1;
    case RESULT_FAIL_MEMORY_OUT_OF_BOUNDS:
        fprintf(stderr, "memory out of bounds at pc %04x\n", chip8.pc);
        return 1;
    default:
        fprintf(stderr, "unknown result code %d, exiting with code 1\n", running);
        return 1;
    }
}