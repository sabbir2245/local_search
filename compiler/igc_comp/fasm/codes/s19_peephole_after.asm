; ============================================================
; Section 19 - Peephole Optimizer: AFTER optimization
; The four redundant patterns have been commented out.
; The result is identical (prints 7) but with fewer instructions.
; ============================================================

format ELF executable 3
entry main

segment readable writeable

segment readable executable
main:
    PUSH EBP
    MOV  EBP, ESP
    SUB  ESP, 4

    MOV EAX, 7
    MOV [EBP-4], EAX

    ; Redundancy 1 removed -- PUSH EAX / POP EAX eliminated
    ;;;;;;;;;;;;;; PUSH EAX
    ;;;;;;;;;;;;;; POP  EAX

    ; Redundancy 2 removed -- ADD ESP, 0 eliminated
    ;;;;;;;;;;;;;; ADD ESP, 0

    ; Redundancy 3 removed -- second MOV EAX,EBX eliminated
    MOV EAX, [EBP-4]
    MOV EBX, EAX
    ;;;;;;;;;;;;;; MOV EAX, EBX

    ; Redundancy 4 removed -- MOV EBX,1 / MUL EBX both eliminated
    ;;;;;;;;;;;;;; MOV EBX, 1
    ;;;;;;;;;;;;;; MUL EBX

    PUSH EAX
    CALL OUTDEC
    POP  EAX

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
