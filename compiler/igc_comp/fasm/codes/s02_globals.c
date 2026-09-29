/* Section 2 - Data Declarations (Global Variables)
 * Expected output:
 *   42
 *   10
 *   30
 *   50
 */
#include <stdio.h>

int x;
int arr[5];

int main() {
    x = 42;
    arr[0] = 10;
    arr[2] = 30;
    arr[4] = 50;

    printf("%d\n", x);
    printf("%d\n", arr[0]);
    printf("%d\n", arr[2]);
    printf("%d\n", arr[4]);
    return 0;
}
