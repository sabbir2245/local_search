; ============================================================
; Section 7 - Stack Operations (PUSH / POP)
; Demonstrates:
;   - saving a register around a computation
;   - the evaluate-right-push-evaluate-left-pop pattern for a+b
; Values: a=7, b=5.  Prints a+b = 12, then a-b = 2.
; Expected output:
;   12
;   2
; ============================================================

format ELF executable 3
entry main

segment readable writeable

segment readable executable
main:
    PUSH EBP
    MOV  EBP, ESP
    SUB  ESP, 8

    ; a = 7 at [EBP-4], b = 5 at [EBP-8]
    MOV EAX, 7
    MOV [EBP-4], EAX
    MOV EAX, 5
    MOV [EBP-8], EAX

    ; a + b  using the push-right-then-left pattern
    MOV EAX, [EBP-8]  ; right operand (b)
    PUSH EAX
    MOV EAX, [EBP-4]  ; left operand (a)
    POP EBX
    ADD EAX, EBX       ; EAX = 12

    ; save EAX across the OUTDEC call
    PUSH EAX
    CALL OUTDEC
    POP  EAX

    ; a - b
    MOV EAX, [EBP-8]  ; right (b)
    PUSH EAX
    MOV EAX, [EBP-4]  ; left (a)
    POP EBX
    SUB EAX, EBX       ; EAX = 2

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
