#include <fcntl.h>
#include <math.h>
#define _GNU_SOURCE
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "fixed-point.h"

#define CALC_DEV "/dev/calc"

#define BUF_SIZE 65

#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define RESET "\x1B[0m"

int fd;
void test_expr(char *s, double expect)
{
    ssize_t value = write(fd, s, strlen(s));
    char buffer[BUF_SIZE];
    fixedp ipt = {0};
    ipt.data = value;

    double result;
    if (ipt.data == NAN_INT)
        result = NAN;
    else if (ipt.data == INF_INT)
        result = INFINITY;
    else
        result = (int) ipt.inte + ((double) ipt.frac / UINT32_MAX);

    if ((isnan(expect) && isnan(result)) || (isinf(expect) && isinf(result)))
        printf(GRN "[PASS]" RESET ": %s == +-%f\n", s, result);
    else if (fabs(result - expect) < 0.00001f)
        printf(GRN "[PASS]" RESET ": %s == %f\n", s, result);
    else
        printf(RED "[FAIL]" RESET ": %s expect %f but get %f\n", s, expect,
               result);
}

#define SIGMA(ans, i, expr, start, end)       \
    ans = 0;                                  \
    for (i = start; i <= end; i++) {          \
        ans += expr;                          \
        if (ans > INT_MAX || ans < INT_MIN) { \
            ans = INFINITY;                   \
            break;                            \
        }                                     \
    }

int main()
{
    fd = open(CALC_DEV, O_RDWR);
    if (fd < 0) {
        perror("Failed to open character device");
        exit(1);
    }

    printf("\nStart bench......\n");
    int64_t n;
    double ans;
    SIGMA(ans, n, n, -10, 11);
    test_expr("sigma(n, n, -10, 11)", ans);

    SIGMA(ans, n, 2 * n, 1, 10);
    test_expr("sigma(n, 2 * n, 1, 10)", ans);

    SIGMA(ans, n, 3 * n + 2, 20, 33);
    test_expr("sigma(n, 3 * n + 2, 20, 33)", ans);

    SIGMA(ans, n, n * 1.1 + 5, 10, 30);
    test_expr("sigma(n, n * 1.1 + 5, 10, 30)", ans);

    SIGMA(ans, n, sqrt(n) * 1.1, 3, 10);
    test_expr("sigma(n, sqrt(n) * 1.1, 3, 10)", ans);

    SIGMA(ans, n, n, INT_MIN, INT_MIN + 1);
    test_expr("INT_MIN=-2147483648, sigma(n, n, INT_MIN, INT_MIN+1)", ans);

    for (int i = 0; i < 100; i += 10) {
        char expr[128] = "sqrt(";
        char num[16];
        snprintf(num, 16, "%d", i);
        strcat(expr, num);
        strcat(expr, ")");
        test_expr(expr, sqrt(i));
    }

    printf("Done.\n\n");

    close(fd);
    return 0;
}
