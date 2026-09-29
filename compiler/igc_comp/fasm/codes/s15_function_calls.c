/* Section 15 - Function Calls and Calling Convention
 * Arguments pushed right-to-left; callee cleans with RET N.
 * Expected output:
 *   42
 *   6
 */
#include <stdio.h>

int multiply(int a, int b) {
    return a * b;
}

int add_three(int a, int b, int c) {
    return a + b + c;
}

int main() {
    printf("%d\n", multiply(6, 7));       /* 42 */
    printf("%d\n", add_three(1, 2, 3));   /* 6  */
    return 0;
}
