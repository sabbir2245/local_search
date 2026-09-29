; ============================================================
; Section 14 - Increment and Decrement (Post-fix)
; a at [EBP-4], b at [EBP-8], c at [EBP-12].
; Post-increment pattern: PUSH old value, INC in memory, POP old.
; Expected output:
;   5   (b = old a)
;   6   (a after ++)
;   6   (c = old a)
;   5   (a after --)
; ============================================================

format ELF executable 3
entry main

segment readable writeable

segment readable executable
main:
    PUSH EBP
    MOV  EBP, ESP
    SUB  ESP, 12     ; a at [EBP-4], b at [EBP-8], c at [EBP-12]

    ; a = 5
    MOV EAX, 5
    MOV [EBP-4], EAX

    ; b = a++
    ;   step 1: PUSH the current (old) value of a
    PUSH dword [EBP-4]
    ;   step 2: increment a in place
    INC  dword [EBP-4]
    ;   step 3: POP old value as the expression result
    POP  EAX
    ;   now store into b
    MOV [EBP-8], EAX

    ; print b  (expect 5)
    PUSH EAX
    MOV  EAX, [EBP-8]
    CALL OUTDEC
    POP  EAX

    ; print a  (expect 6)
    PUSH EAX
    MOV  EAX, [EBP-4]
    CALL OUTDEC
    POP  EAX

    ; c = a--  (same trick with DEC)
    PUSH dword [EBP-4]
    DEC  dword [EBP-4]
    POP  EAX
    MOV [EBP-12], EAX

    ; print c  (expect 6)
    PUSH EAX
    MOV  EAX, [EBP-12]
    CALL OUTDEC
    POP  EAX

    ; print a  (expect 5)
    PUSH EAX
    MOV  EAX, [EBP-4]
    CALL OUTDEC
    POP  EAX

main_exit:
    MOV EAX, 1
    XOR EBX, EBX
    INT 0x80
    ADD ESP, 12
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
