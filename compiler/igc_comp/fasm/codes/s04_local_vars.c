/* Section 4 - Local Variable & Parameter Layout
 * Demonstrates nextLocalOffset / nextParamOffset assignment.
 *   x at [EBP+8], y at [EBP+12]
 *   a at [EBP-4],  b at [EBP-8]
 * Expected output:
 *   14
 */
#include <stdio.h>

int foo(int x, int y) {
    int a;
    int b;
    a = x + y;
    b = a * 2;
    return b;
}

int main() {
    printf("%d\n", foo(3, 4));   /* (3+4)*2 = 14 */
    return 0;
}
