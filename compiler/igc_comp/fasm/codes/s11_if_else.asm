; ============================================================
; Section 11 - Control Flow: IF-ELSE statement
; x = -3.  if (x > 0) print 1 else print 0.
; x is negative, so the else branch runs.
; Expected output:
;   0
; ============================================================

format ELF executable 3
entry main

segment readable writeable

segment readable executable
main:
    PUSH EBP
    MOV  EBP, ESP
    SUB  ESP, 4

    ; x = -3
    MOV EAX, 3
    NEG EAX             ; EAX = -3
    MOV [EBP-4], EAX

    ; if (x > 0)
    MOV EAX, 0         ; right operand
    PUSH EAX
    MOV EAX, [EBP-4]  ; left operand (x = -3)
    POP EBX
    CMP EAX, EBX
    JLE L_else          ; not greater: go to else

    ; if-body: print 1
    MOV EAX, 1
    PUSH EAX
    CALL OUTDEC
    POP  EAX
    JMP  L_end

L_else:
    ; else-body: print 0
    MOV EAX, 0
    PUSH EAX
    CALL OUTDEC
    POP  EAX

L_end:

main_exit:
    MOV EAX, 1
    XOR EBX, EBX
    INT 0x80
    ADD ESP, 4
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
