; ============================================================
; Section 11 - Control Flow: FOR loop
; for (i = 1; i <= 4; i++) print i
; Expected output:
;   1
;   2
;   3
;   4
; ============================================================

format ELF executable 3
entry main

segment readable writeable

segment readable executable
main:
    PUSH EBP
    MOV  EBP, ESP
    SUB  ESP, 4         ; i at [EBP-4]

    ; initialiser: i = 1
    MOV EAX, 1
    MOV [EBP-4], EAX
    PUSH EAX
    MOV [EBP-4], EAX   ; store i
    POP EAX

L_for_start:
    ; condition: i <= 4
    MOV EAX, 4         ; right operand
    PUSH EAX
    MOV EAX, [EBP-4]  ; left operand (i)
    POP EBX
    CMP EAX, EBX
    JG  L_for_end      ; i > 4: exit

    ; body: print i
    PUSH EAX
    MOV  EAX, [EBP-4]
    CALL OUTDEC
    POP  EAX

    ; increment: i++ (post-increment, result discarded)
    PUSH dword [EBP-4]
    INC  dword [EBP-4]
    POP  EAX            ; EAX = old i (discarded)

    JMP L_for_start

L_for_end:

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
