/* Section 11 - Control Flow Patterns (IF, IF-ELSE, WHILE, FOR, RETURN)
 * Expected output:
 *   5
 *   negative
 *   1
 *   0
 *   1
 *   2
 *   3
 *   4
 *   10
 *   9
 *   8
 *   7
 *   6
 *   5
 *   4
 *   3
 *   2
 *   1
 *   42
 */
#include <stdio.h>

int early_return(int n) {
    if (n > 0) return n;
    return 0;
}

int main() {
    int x;

    /* IF: x starts at 10, decrement while > 5 */
    x = 10;
    if (x > 5) x = 5;
    printf("%d\n", x);         /* 5 */

    /* IF-ELSE */
    x = -3;
    if (x > 0) {
        printf("positive\n");  /* not printed */
    } else {
        printf("negative\n");  /* negative */
    }

    /* FOR loop: prints 1 2 3 4 */
    int i;
    for (i = 1; i <= 4; i++) {
        printf("%d\n", i);
    }

    /* WHILE loop: countdown from 10 to 1 */
    x = 10;
    while (x > 0) {
        printf("%d\n", x);
        x = x - 1;
    }

    /* RETURN: early_return(42) returns 42 */
    printf("%d\n", early_return(42));

    return 0;
}
