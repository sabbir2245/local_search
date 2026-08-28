; ============================================================
; Section 19 - Peephole Optimizer: BEFORE optimization
; This is the raw output the compiler would produce.
; It contains four redundant patterns:
;
;   1. PUSH EAX / POP EAX  (same operand -- no-op)
;   2. ADD ESP, 0           (add zero -- no-op)
;   3. MOV EAX, EBX / MOV EBX, EAX  (redundant second MOV)
;   4. MOV EBX, 1 / MUL EBX         (multiply by 1 -- no-op)
;
; Expected output: 7  (the redundancies don't change the result)
; ============================================================

format ELF executable 3
entry main

segment readable writeable

segment readable executable
main:
    PUSH EBP
    MOV  EBP, ESP
    SUB  ESP, 4

    ; Load 7 into EAX
    MOV EAX, 7
    MOV [EBP-4], EAX

    ; Redundancy 1: PUSH/POP same register (optimizer removes both)
    PUSH EAX
    POP  EAX

    ; Redundancy 2: ADD ESP, 0 (optimizer removes it)
    ADD ESP, 0

    ; Redundancy 3: MOV A,B followed by MOV B,A (second is removed)
    MOV EAX, [EBP-4]
    MOV EBX, EAX
    MOV EAX, EBX      ; <-- redundant: EAX already = [EBP-4]

    ; Redundancy 4: MUL by 1 (optimizer removes both lines)
    MOV EBX, 1
    MUL EBX

    ; Print result (should be 7)
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
