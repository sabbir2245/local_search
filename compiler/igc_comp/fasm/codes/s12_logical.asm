; ============================================================
; Section 12 - Logical Operators with Short-Circuit Evaluation
; a=1, b=0.  Tests &&, ||, !a, !b.
; Expected output:
;   0   (a && b)
;   1   (a || b)
;   0   (!a)
;   1   (!b)
; ============================================================

format ELF executable 3
entry main

segment readable writeable

segment readable executable
main:
    PUSH EBP
    MOV  EBP, ESP
    SUB  ESP, 8

    MOV EAX, 1
    MOV [EBP-4], EAX   ; a = 1
    MOV EAX, 0
    MOV [EBP-8], EAX   ; b = 0

    ; ---- a && b ----
    ; Evaluate left (a): if 0 skip right and result = 0
    MOV EAX, [EBP-4]
    TEST EAX, EAX
    JE   L_and_skip
    ; left was true: evaluate right (b)
    MOV EAX, [EBP-8]
    JMP  L_and_end
L_and_skip:
    MOV EAX, 0
L_and_end:
    PUSH EAX
    CALL OUTDEC
    POP  EAX

    ; ---- a || b ----
    ; Evaluate left (a): if non-zero skip right and result = 1
    MOV EAX, [EBP-4]
    TEST EAX, EAX
    JNE  L_or_skip
    ; left was false: evaluate right (b)
    MOV EAX, [EBP-8]
    JMP  L_or_end
L_or_skip:
    MOV EAX, 1
L_or_end:
    PUSH EAX
    CALL OUTDEC
    POP  EAX

    ; ---- !a ----
    MOV EAX, [EBP-4]   ; a = 1
    TEST EAX, EAX
    JNE  L_nota_true
    MOV EAX, 1
    JMP  L_nota_end
L_nota_true:
    MOV EAX, 0
L_nota_end:
    PUSH EAX
    CALL OUTDEC
    POP  EAX

    ; ---- !b ----
    MOV EAX, [EBP-8]   ; b = 0
    TEST EAX, EAX
    JNE  L_notb_true
    MOV EAX, 1
    JMP  L_notb_end
L_notb_true:
    MOV EAX, 0
L_notb_end:
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
