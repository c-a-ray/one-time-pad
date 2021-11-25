#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    char *t1 = (char *) malloc(5);
    t1[0] = 'h';
    t1[1] = 'e';
    t1[2] = 'l';
    t1[3] = 'o';
    t1[4] = '\0';

    t1 = (char *) realloc(t1, 10);
    t1[4] = ' ';
    t1[5] = 'b';
    t1[6] = 'u';
    t1[7] = 'b';
    t1[8] = '\0';

    printf("%s\n", t1);
    return 0;
}