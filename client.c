#include <fcntl.h>
#include <math.h>
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fixed-point.h"

#define CALC_DEV "/dev/calc"

int fd;
void test_expr(char *s, double expect)
{
    ssize_t value = write(fd, s, strlen(s));
    fixedp ipt = {0};
    ipt.data = value;

    double result;
    if (ipt.data == NAN_INT)
        result = NAN;
    else if (ipt.data == INF_INT)
        result = INFINITY;
    else
        result = (int) ipt.inte + ((double) ipt.frac / UINT32_MAX);

    if (isnan(expect) && isnan(result))
        printf("[PASS]: %s == +-%f\n", s, result);
    else if (isinf(expect) && isinf(result))
        printf("[PASS]: %s == +-%f\n", s, result);
    else if (fabs(result - expect) < 0.00001f)
        printf("[PASS]: %s == %f\n", s, result);
    else
        printf("[FAIL]: %s expect %f but get %f\n", s, expect, result);
}


int main()
{
    fd = open(CALC_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    printf("\nStart bench......\n");

    for (int i = 0; i < 100; i += 10) {
        char expr[128] = "sqrt(";
        char num[16];
        snprintf(num, 16, "%d", i);
        strcat(expr, num);
        strcat(expr, ")");
        test_expr(expr, sqrt(i));
    }

    for (float i = 100; i < 101; i += 0.1) {
        char expr[128] = "sqrt(";
        char num[16];
        snprintf(num, 16, "%f", i);
        strcat(expr, num);
        strcat(expr, ")");
        test_expr(expr, sqrt(i));
    }

    printf("Done.\n\n");

    close(fd);
    return 0;
}
