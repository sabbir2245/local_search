/* Section 5 - Arithmetic Operations
 * Covers ADD, SUB, MUL, DIV (quotient), DIV (remainder), NEG.
 * Expected output:
 *   13
 *   7
 *   30
 *   3
 *   1
 *   -10
 */
#include <stdio.h>

int main() {
    int a = 10, b = 3;
    printf("%d\n", a + b);   /* 13 */
    printf("%d\n", a - b);   /* 7  */
    printf("%d\n", a * b);   /* 30 */
    printf("%d\n", a / b);   /* 3  */
    printf("%d\n", a % b);   /* 1  */
    printf("%d\n", -a);      /* -10 */
    return 0;
}
