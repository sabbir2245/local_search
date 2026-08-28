/* Section 9 - Array Indexing (global and local)
 * Expected output:
 *   0
 *   100
 *   200
 *   0
 *   10
 *   20
 */
#include <stdio.h>

int g[3];

int main() {
    int local[3];
    int i;

    g[0] = 0;
    g[1] = 100;
    g[2] = 200;

    local[0] = 0;
    local[1] = 10;
    local[2] = 20;

    for (i = 0; i < 3; i++) printf("%d\n", g[i]);
    for (i = 0; i < 3; i++) printf("%d\n", local[i]);

    return 0;
}
