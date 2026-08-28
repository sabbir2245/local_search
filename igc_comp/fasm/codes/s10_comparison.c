/* Section 10 - Comparison and Conditional Jumps
 * Each relational operator produces 1 (true) or 0 (false).
 * a = 5, b = 3.
 * Expected output:
 *   0
 *   1
 *   0
 *   1
 *   0
 *   1
 */
#include <stdio.h>

int main() {
    int a = 5, b = 3;
    printf("%d\n", a < b);    /* 0 */
    printf("%d\n", a > b);    /* 1 */
    printf("%d\n", a == b);   /* 0 */
    printf("%d\n", a != b);   /* 1 */
    printf("%d\n", a <= b);   /* 0 */
    printf("%d\n", a >= b);   /* 1 */
    return 0;
}
