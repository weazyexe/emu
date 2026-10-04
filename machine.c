#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define LOAD 0x01
#define ADD 0x02
#define JNZ 0x03
#define PRINT 0x04
#define HALT 0xFF

#define DEBUG "debug"

struct context_t
{
    uint8_t mem[256];
    uint8_t r[4];
    uint8_t pc;
};

const char *op_name(uint8_t opcode)
{
    switch (opcode)
    {
    case LOAD:
        return "load";
    case ADD:
        return "add";
    case JNZ:
        return "jnz";
    case PRINT:
        return "print";
    case HALT:
        return "halt";
    default:
        return "huh?";
    }
}

int main(int argc, char **argv)
{
    int debug = 0;

    const char *debug_flag = argv[1];
    if (debug_flag != NULL && strcmp(debug_flag, DEBUG) == 0)
    {
        debug = 1;
    }

    struct context_t ctx = {.mem = {0x01, 0x00, 0x03, 0x01, 0x01, 0xFF, 0x04, 0x00, 0x00, 0x02, 0x00, 0x01, 0x03, 0x00, 0x06, 0xFF, 0x00, 0x00}};
    ctx.pc = 0;

    if (debug)
    {
        printf("machine created\n");
        printf("memory loaded: \n");
        for (size_t i = 0; i < sizeof(ctx.mem); i++)
        {
            if (i != 0 && i % 3 == 0)
            {
                printf("\n");
            }
            printf("%02x ", ctx.mem[i]);
        }
        printf("\n\n");
    }

    int running = 1;
    while (running)
    {
        uint8_t op = ctx.mem[ctx.pc];
        uint8_t a = ctx.mem[ctx.pc + 1];
        uint8_t b = ctx.mem[ctx.pc + 2];

        if (debug)
        {
            printf("running: op = %s (%02x), a = %02x, b = %02x, pc = %02x\n", op_name(op), op, a, b, ctx.pc);
        }

        ctx.pc += 3;

        switch (op)
        {
        case LOAD:
            ctx.r[a] = b;
            break;
        case ADD:
            ctx.r[a] = ctx.r[a] + ctx.r[b];
            break;
        case JNZ:
            if (ctx.r[a] != 0)
            {
                ctx.pc = b;
            }
            break;
        case PRINT:
            printf("%d\n", ctx.r[a]);
            break;
        case HALT:
            running = 0;
            break;
        default:
            break;
        }
    }

    if (debug)
    {
        printf("machine has stopped\n");
    }

    return 0;
}