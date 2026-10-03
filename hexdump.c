#include <stdio.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("usage: hexdump FILE\n");
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (f == NULL)
    {
        printf("cannot open file %s\n", argv[1]);
        return 1;
    }

    int c;
    long offset = 0;
    while ((c = fgetc(f)) != EOF)
    {
        if (offset % 16 == 0)
        {
            if (offset != 0)
            {
                printf("\n");
            }
            printf("%08lx: ", offset);
        }
        printf("%02x ", c);
        offset++;
    }

    printf("\n");
    fclose(f);
    return 0;
}