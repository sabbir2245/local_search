/* Section 14 - Increment and Decrement (Post-fix)
 * Post-increment returns the OLD value; variable is modified after.
 * Expected output:
 *   5   (b = old value of a before increment)
 *   6   (a after increment)
 *   6   (c = old value of a before decrement)
 *   5   (a after decrement)
 */
#include <stdio.h>

int main() {
    int a = 5;
    int b = a++;    /* b = 5, a becomes 6 */
    printf("%d\n", b);   /* 5 */
    printf("%d\n", a);   /* 6 */

    int c = a--;    /* c = 6, a becomes 5 */
    printf("%d\n", c);   /* 6 */
    printf("%d\n", a);   /* 5 */

    return 0;
}
