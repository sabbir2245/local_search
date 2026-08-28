/* Section 12 - Logical Operators with Short-Circuit Evaluation
 * Expected output:
 *   0
 *   1
 *   0
 *   1
 */
#include <stdio.h>

int main() {
    int a = 1, b = 0;
    printf("%d\n", a && b);   /* 0  -- b is evaluated but result is 0 */
    printf("%d\n", a || b);   /* 1  -- short-circuit: a is 1 so 1 */
    printf("%d\n", !a);       /* 0 */
    printf("%d\n", !b);       /* 1 */
    return 0;
}
