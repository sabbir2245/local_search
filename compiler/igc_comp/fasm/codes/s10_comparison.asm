; ============================================================
; Section 10 - Comparison and Conditional Jumps
; For each relation, the compiler:
;   1. Pushes right operand, loads left, pops right into EBX
;   2. CMP EAX, EBX
;   3. Jumps to a "true" label or falls through to "false"
;   4. Sets EAX to 0 or 1
; a=5, b=3
; Expected output:
;   0   (5 < 3)
;   1   (5 > 3)
;   0   (5 == 3)
;   1   (5 != 3)
;   0   (5 <= 3)
;   1   (5 >= 3)
; ============================================================

format ELF executable 3
entry main

segment readable writeable

segment readable executable
main:
    PUSH EBP
    MOV  EBP, ESP
    SUB  ESP, 8

    MOV EAX, 5
    MOV [EBP-4], EAX   ; a = 5
    MOV EAX, 3
    MOV [EBP-8], EAX   ; b = 3

    ; a < b
    MOV EAX, [EBP-8]   ; right = b
    PUSH EAX
    MOV EAX, [EBP-4]   ; left  = a
    POP EBX
    CMP EAX, EBX
    JL  L0_true
    MOV EAX, 0
    JMP L0_end
L0_true:
    MOV EAX, 1
L0_end:
    PUSH EAX
    CALL OUTDEC
    POP  EAX

    ; a > b
    MOV EAX, [EBP-8]
    PUSH EAX
    MOV EAX, [EBP-4]
    POP EBX
    CMP EAX, EBX
    JG  L1_true
    MOV EAX, 0
    JMP L1_end
L1_true:
    MOV EAX, 1
L1_end:
    PUSH EAX
    CALL OUTDEC
    POP  EAX

    ; a == b
    MOV EAX, [EBP-8]
    PUSH EAX
    MOV EAX, [EBP-4]
    POP EBX
    CMP EAX, EBX
    JE  L2_true
    MOV EAX, 0
    JMP L2_end
L2_true:
    MOV EAX, 1
L2_end:
    PUSH EAX
    CALL OUTDEC
    POP  EAX

    ; a != b
    MOV EAX, [EBP-8]
    PUSH EAX
    MOV EAX, [EBP-4]
    POP EBX
    CMP EAX, EBX
    JNE L3_true
    MOV EAX, 0
    JMP L3_end
L3_true:
    MOV EAX, 1
L3_end:
    PUSH EAX
    CALL OUTDEC
    POP  EAX

    ; a <= b
    MOV EAX, [EBP-8]
    PUSH EAX
    MOV EAX, [EBP-4]
    POP EBX
    CMP EAX, EBX
    JLE L4_true
    MOV EAX, 0
    JMP L4_end
L4_true:
    MOV EAX, 1
L4_end:
    PUSH EAX
    CALL OUTDEC
    POP  EAX

    ; a >= b
    MOV EAX, [EBP-8]
    PUSH EAX
    MOV EAX, [EBP-4]
    POP EBX
    CMP EAX, EBX
    JGE L5_true
    MOV EAX, 0
    JMP L5_end
L5_true:
    MOV EAX, 1
L5_end:
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
