; ============================================================
; Section 11 - Control Flow: WHILE loop
; Counts down: x = 5, prints x while x > 0.
; Expected output:
;   5
;   4
;   3
;   2
;   1
; ============================================================

format ELF executable 3
entry main

segment readable writeable

segment readable executable
main:
    PUSH EBP
    MOV  EBP, ESP
    SUB  ESP, 4

    ; x = 5
    MOV EAX, 5
    MOV [EBP-4], EAX

L_start:
    ; condition: x > 0
    MOV EAX, 0
    PUSH EAX
    MOV EAX, [EBP-4]   ; x
    POP EBX
    CMP EAX, EBX
    JLE L_end           ; x <= 0: exit loop

    ; body: print x, then x = x - 1
    PUSH EAX
    MOV  EAX, [EBP-4]
    CALL OUTDEC
    POP  EAX

    MOV EAX, 1
    PUSH EAX
    MOV EAX, [EBP-4]
    POP EBX
    SUB EAX, EBX        ; x - 1
    PUSH EAX
    MOV [EBP-4], EAX   ; store back
    POP EAX

    JMP L_start

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
