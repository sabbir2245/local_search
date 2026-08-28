/* Section 3 - Stack Frame / Function Prologue & Epilogue
 * Demonstrates PUSH EBP / MOV EBP,ESP / ADD ESP,N / POP EBP / RET N.
 * Expected output:
 *   8
 */
#include <stdio.h>

int add(int a, int b) {
    return a + b;
}

int main() {
    printf("%d\n", add(3, 5));   /* 8 */
    return 0;
}
