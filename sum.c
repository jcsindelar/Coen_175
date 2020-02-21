/* sum.c */

int printf(char *s, ...);

int main(void)
{
    int i, n, the_sum;

    i = 0;
    the_sum = 0;
    n = 10;

    for (i = 1; i <= n; i = i + 1)
	the_sum = the_sum + i;

    printf("%d %d\n", the_sum, n * (n + 1) / 2);
}
