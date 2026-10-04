#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

struct numbers_t
{
    int a, b, c;
};

struct mixed_t
{
    uint8_t a;
    int x;
};

int main()
{
    struct numbers_t n = {1, 2, 3};

    printf("%p\n", (void *)&n.a);
    printf("%p\n", (void *)&n.b);
    printf("%p\n", (void *)&n.c);

    printf("%zu\n", offsetof(struct numbers_t, a));
    printf("%zu\n", offsetof(struct numbers_t, b));
    printf("%zu\n", offsetof(struct numbers_t, c));

    printf("%zu\n", offsetof(struct mixed_t, a));
    printf("%zu\n", offsetof(struct mixed_t, x));
    printf("%zu\n", sizeof(struct mixed_t));

    return 0;
}
