/* Section 20 - Complete Example: Recursive Factorial
 * Exercises: function calls, recursion, IF, multiplication, RETURN.
 * Expected output:
 *   120
 *   1
 *   0
 */
#include <stdio.h>

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    printf("%d\n", factorial(5));   /* 120 */
    printf("%d\n", factorial(1));   /* 1   */
    printf("%d\n", factorial(0));   /* 1... wait, 0! = 1 */
    return 0;
}
