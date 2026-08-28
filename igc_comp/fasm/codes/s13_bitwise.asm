; ============================================================
; Section 13 - Bitwise / Utility Instructions
; These instructions appear in the compiler's generated code
; for internal purposes (not from C-level bitwise operators).
;
;   XOR reg, reg  -- fastest way to zero a register
;   OR  reg, reg  -- set flags to test sign without modifying reg
;
; This program demonstrates both techniques and prints
; confirmation values showing they work as expected.
; Expected output:
;   0    (XOR EDX,EDX zeroed EDX; printed as 0)
;   1    (OR EAX,EAX on negative number set SF; we branched to 1)
;   0    (OR EAX,EAX on positive number did NOT set SF; branched to 0)
; ============================================================

format ELF executable 3
entry main

segment readable writeable

segment readable executable
main:
    PUSH EBP
    MOV  EBP, ESP

    ; ---- XOR reg, reg ----
    ; Used before DIV to clear EDX (the high half of the dividend).
    MOV EDX, 12345      ; EDX has some garbage value
    XOR EDX, EDX        ; cheaply zero it
    ; Move EDX into EAX so we can print it
    MOV EAX, EDX
    PUSH EAX
    CALL OUTDEC         ; prints 0
    POP  EAX

    ; ---- OR reg, reg: sign detection ----
    ; Used in OUTDEC to check whether EAX is negative.
    ; OR EAX, EAX computes EAX AND EAX, updates SF (sign flag),
    ; but does NOT change EAX itself.

    ; Test 1: negative input
    MOV EAX, -7
    OR  EAX, EAX        ; sets SF=1 because EAX < 0
    JGE L_pos1          ; if SF=0 (positive), skip
    MOV EAX, 1          ; signal: "was negative"
    JMP L_end1
L_pos1:
    MOV EAX, 0
L_end1:
    PUSH EAX
    CALL OUTDEC         ; prints 1
    POP  EAX

    ; Test 2: positive input
    MOV EAX, 42
    OR  EAX, EAX        ; sets SF=0 because EAX > 0
    JGE L_pos2
    MOV EAX, 1
    JMP L_end2
L_pos2:
    MOV EAX, 0
L_end2:
    PUSH EAX
    CALL OUTDEC         ; prints 0
    POP  EAX

main_exit:
    MOV EAX, 1
    XOR EBX, EBX
    INT 0x80
    ADD ESP, 0
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
