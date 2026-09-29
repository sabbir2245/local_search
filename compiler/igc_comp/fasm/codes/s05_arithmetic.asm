; ============================================================
; Section 5 - Arithmetic Operations
; a=10, b=3  stored as locals [EBP-4] and [EBP-8].
; Prints:  a+b, a-b, a*b, a/b, a%b, -a
; Expected output:
;   13
;   7
;   30
;   3
;   1
;   -10
; ============================================================

format ELF executable 3
entry main

segment readable writeable

segment readable executable
main:
    PUSH EBP
    MOV  EBP, ESP
    SUB  ESP, 8     ; space for a and b

    ; a = 10
    MOV EAX, 10
    MOV [EBP-4], EAX

    ; b = 3
    MOV EAX, 3
    MOV [EBP-8], EAX

    ; a + b: evaluate right (b) first, push, then left (a)
    MOV EAX, [EBP-8]
    PUSH EAX
    MOV EAX, [EBP-4]
    POP EBX
    ADD EAX, EBX        ; EAX = 13
    PUSH EAX
    CALL OUTDEC
    POP  EAX

    ; a - b
    MOV EAX, [EBP-8]
    PUSH EAX
    MOV EAX, [EBP-4]
    POP EBX
    SUB EAX, EBX        ; EAX = 7
    PUSH EAX
    CALL OUTDEC
    POP  EAX

    ; a * b
    MOV EAX, [EBP-8]   ; b (right operand first)
    PUSH EAX
    MOV EAX, [EBP-4]   ; a
    POP EBX
    MUL EBX             ; EAX = 30
    PUSH EAX
    CALL OUTDEC
    POP  EAX

    ; a / b  (XOR EDX,EDX clears high half before DIV)
    MOV EAX, [EBP-8]   ; b
    PUSH EAX
    MOV EAX, [EBP-4]   ; a
    POP EBX
    XOR EDX, EDX
    DIV EBX             ; EAX = quotient = 3, EDX = remainder = 1
    PUSH EAX
    CALL OUTDEC         ; prints 3
    POP  EAX

    ; a % b  (same steps, but move EDX into EAX at the end)
    MOV EAX, [EBP-8]
    PUSH EAX
    MOV EAX, [EBP-4]
    POP EBX
    XOR EDX, EDX
    DIV EBX
    MOV EAX, EDX        ; move remainder into EAX
    PUSH EAX
    CALL OUTDEC         ; prints 1
    POP  EAX

    ; -a  (unary negation)
    MOV EAX, [EBP-4]
    NEG EAX             ; EAX = -10
    PUSH EAX
    CALL OUTDEC
    POP  EAX

main_exit:
    MOV EAX, 1
    XOR EBX, EBX
    INT 0x80
    ADD ESP, 8
    POP EBP
    RET

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
