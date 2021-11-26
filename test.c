#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void test_func(char *t)
{
    free(t);
    t = malloc(5);
    char *t1 = "hey!";
    strcpy(t, t1);
}

int main(void)
{
    char *t = malloc(0);
    test_func(t);
    printf("%s\n", t);
    return 0;
}