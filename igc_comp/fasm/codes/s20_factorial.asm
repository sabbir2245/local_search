; ============================================================
; Section 20 - Complete Example: Recursive Factorial
; Uses: function calls, if-statement, multiplication, return.
; factorial(5) = 120, factorial(1) = 1, factorial(0) = 1
; Expected output:
;   120
;   1
;   1
; ============================================================

format ELF executable 3
entry main

segment readable writeable
    g dd 1 dup (0)     ; global: stores the result

segment readable executable
main:
    PUSH EBP
    MOV  EBP, ESP

    ; g = factorial(5)
    MOV EAX, 5
    PUSH EAX
    CALL factorial      ; EAX = 120
    PUSH EAX
    MOV [g], EAX
    POP  EAX

    ; println(g)
    PUSH EAX
    MOV  EAX, [g]
    CALL OUTDEC
    POP  EAX

    ; println(factorial(1))
    MOV EAX, 1
    PUSH EAX
    CALL factorial
    PUSH EAX
    CALL OUTDEC
    POP  EAX

    ; println(factorial(0))
    MOV EAX, 0
    PUSH EAX
    CALL factorial
    PUSH EAX
    CALL OUTDEC
    POP  EAX

main_exit:
    MOV EAX, 1
    XOR EBX, EBX
    INT 0x80
    ADD ESP, 0
    POP EBP
    RET

; int factorial(int n)
; n at [EBP+8]
factorial:
    PUSH EBP
    MOV  EBP, ESP
    ; no locals

    ; if (n <= 1) return 1
    MOV EAX, 1         ; right operand
    PUSH EAX
    MOV EAX, [EBP+8]   ; n (left operand)
    POP EBX
    CMP EAX, EBX        ; compare n and 1
    JLE factorial_base  ; n <= 1: go to base case
    JMP factorial_rec

factorial_base:
    MOV EAX, 1
    JMP factorial_exit

factorial_rec:
    ; return n * factorial(n - 1)
    ; compute n - 1
    MOV EAX, 1
    PUSH EAX
    MOV EAX, [EBP+8]   ; n
    POP EBX
    SUB EAX, EBX        ; EAX = n - 1

    ; call factorial(n - 1)
    PUSH EAX
    CALL factorial      ; EAX = factorial(n-1)

    ; multiply: n * factorial(n-1)
    PUSH EAX            ; save factorial(n-1)
    MOV  EAX, [EBP+8]  ; load n
    POP  EBX            ; EBX = factorial(n-1)
    MUL  EBX            ; EAX = n * factorial(n-1)
    JMP factorial_exit

factorial_exit:
    ADD ESP, 0
    POP EBP
    RET 4               ; callee pops n (1 param * 4 bytes)

; ---- OUTDEC ----
OUTDEC:
    PUSH EBX
    PUSH ECX
    PUSH EDX
    PUSH ESI
    OR   EAX, EAX
    JGE  OUTDEC_POSITIVE
    NEG  EAX
    PUSH EAX
    SUB  ESP, 4
    MOV  byte [ESP], '-'
    MOV  EAX, 4
    MOV  EBX, 1
    MOV  ECX, ESP
    MOV  EDX, 1
    INT  0x80
    ADD  ESP, 4
    POP  EAX
OUTDEC_POSITIVE:
    XOR  ECX, ECX
    MOV  EBX, 10
OUTDEC_DIGIT_LOOP:
    XOR  EDX, EDX
    DIV  EBX
    ADD  DL, 30h
    PUSH EDX
    INC  ECX
    TEST EAX, EAX
    JNZ  OUTDEC_DIGIT_LOOP
    MOV  ESI, ECX
    MOV  EBX, 1
    MOV  EDX, 1
OUTDEC_PRINT_LOOP:
    TEST ESI, ESI
    JZ   OUTDEC_NEWLINE
    MOV  EAX, 4
    MOV  ECX, ESP
    INT  0x80
    ADD  ESP, 4
    DEC  ESI
    JMP  OUTDEC_PRINT_LOOP
OUTDEC_NEWLINE:
    SUB  ESP, 4
    MOV  byte [ESP], 10
    MOV  EAX, 4
    MOV  ECX, ESP
    INT  0x80
    ADD  ESP, 4
    POP  ESI
    POP  EDX
    POP  ECX
    POP  EBX
    RET
