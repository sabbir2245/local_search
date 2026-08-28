; ============================================================
; Section 6 - Data Movement (MOV)
; Shows every form of MOV used by the compiler:
;   constant -> register, memory -> register, register -> memory,
;   register -> register, byte literal -> memory.
; Prints two values to prove the moves worked.
; Expected output:
;   100
;   42
; ============================================================

format ELF executable 3
entry main

segment readable writeable
    g dd 1 dup (0)      ; global int g

segment readable executable
main:
    PUSH EBP
    MOV  EBP, ESP
    SUB  ESP, 4         ; one local variable at [EBP-4]

    ; 1. immediate -> register
    MOV EAX, 100

    ; 2. register -> memory  (store 100 into local variable)
    MOV [EBP-4], EAX

    ; 3. memory -> register  (load it back)
    MOV EAX, [EBP-4]

    ; print it (should be 100)
    PUSH EAX
    CALL OUTDEC
    POP  EAX

    ; 4. immediate -> register, then register -> register
    MOV EAX, 42
    MOV EBX, EAX        ; copy EAX into EBX

    ; 5. register -> global memory
    MOV [g], EBX

    ; 6. global memory -> register
    MOV EAX, [g]

    ; print it (should be 42)
    PUSH EAX
    CALL OUTDEC
    POP  EAX

    ; 7. byte-sized store (used in OUTDEC for character buffers)
    SUB ESP, 4
    MOV byte [ESP], 65  ; ASCII 'A' -- just to show byte [mem] works
    ADD ESP, 4          ; clean up without printing (just a demo)

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
