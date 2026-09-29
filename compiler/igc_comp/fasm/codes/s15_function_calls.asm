; ============================================================
; Section 15 - Function Calls and Calling Convention
; Callee-cleanup: RET N pops N bytes of arguments.
;
; multiply(6, 7): 2 params => RET 8
; add_three(1, 2, 3): 3 params => RET 12
;
; Expected output:
;   42
;   6
; ============================================================

format ELF executable 3
entry main

segment readable writeable

segment readable executable
main:
    PUSH EBP
    MOV  EBP, ESP

    ; multiply(6, 7) -- push right-to-left
    MOV EAX, 7    ; b (right arg)
    PUSH EAX
    MOV EAX, 6    ; a (left arg)
    PUSH EAX
    CALL multiply  ; EAX = 42

    PUSH EAX
    CALL OUTDEC
    POP  EAX

    ; add_three(1, 2, 3) -- push right-to-left
    MOV EAX, 3    ; c
    PUSH EAX
    MOV EAX, 2    ; b
    PUSH EAX
    MOV EAX, 1    ; a
    PUSH EAX
    CALL add_three ; EAX = 6

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

; int multiply(int a, int b)
; a at [EBP+8], b at [EBP+12]
multiply:
    PUSH EBP
    MOV  EBP, ESP

    MOV EAX, [EBP+12]  ; b (right operand first for MUL pattern)
    PUSH EAX
    MOV EAX, [EBP+8]   ; a
    POP EBX
    MUL EBX             ; EAX = a * b

multiply_exit:
    ADD ESP, 0
    POP EBP
    RET 8               ; callee pops a and b (2 * 4 = 8)

; int add_three(int a, int b, int c)
; a at [EBP+8], b at [EBP+12], c at [EBP+16]
add_three:
    PUSH EBP
    MOV  EBP, ESP

    ; a + b
    MOV EAX, [EBP+12]
    PUSH EAX
    MOV EAX, [EBP+8]
    POP EBX
    ADD EAX, EBX        ; EAX = a + b

    ; (a + b) + c
    MOV EBX, [EBP+16]
    ADD EAX, EBX        ; EAX = a + b + c

add_three_exit:
    ADD ESP, 0
    POP EBP
    RET 12              ; callee pops a, b, c (3 * 4 = 12)

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
