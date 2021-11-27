#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    printf("-12 % 27 = %d\n", mod(-12, 27));
    return 0;
}

int mod(int a, int b)
{
    int remainder = a % b;
    return remainder < 0 ? remainder + b : remainder;
}