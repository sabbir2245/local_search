; ============================================================
; Section 4 - Local Variable & Parameter Layout
; Calls foo(3, 4) where:
;   x at [EBP+8], y at [EBP+12]   (nextParamOffset starts at 8)
;   a at [EBP-4],  b at [EBP-8]   (nextLocalOffset starts at -4)
; Expected output:
;   14
; ============================================================

format ELF executable 3
entry main

segment readable writeable

segment readable executable
main:
    PUSH EBP
    MOV  EBP, ESP

    ; call foo(3, 4)
    MOV EAX, 4     ; y (second arg)
    PUSH EAX
    MOV EAX, 3     ; x (first arg)
    PUSH EAX
    CALL foo        ; EAX = 14

    PUSH EAX
    CALL OUTDEC
    POP  EAX

main_exit:
    MOV EAX, 1
    XOR EBX, EBX
    INT 0x80
    ADD ESP, 0
    POP EBP
    RET

; int foo(int x, int y)
foo:
    PUSH EBP
    MOV  EBP, ESP
    SUB  ESP, 8     ; allocate space for a (4 bytes) and b (4 bytes)
                    ; nextLocalOffset goes: -4 (for a), then -8 (for b)

    ; a = x + y
    MOV EAX, [EBP+12]  ; y  (nextParamOffset: x=8, y=12)
    PUSH EAX
    MOV EAX, [EBP+8]   ; x
    POP EBX
    ADD EAX, EBX        ; EAX = x + y = 7
    PUSH EAX
    MOV [EBP-4], EAX    ; store into a  (nextLocalOffset was -4)
    POP EAX

    ; b = a * 2
    MOV EAX, 2
    PUSH EAX
    MOV EAX, [EBP-4]    ; load a
    POP EBX
    MUL EBX             ; EAX = a * 2 = 14
    PUSH EAX
    MOV [EBP-8], EAX    ; store into b  (nextLocalOffset was -8)
    POP EAX

    ; return b
    MOV EAX, [EBP-8]
    JMP foo_exit

foo_exit:
    ADD ESP, 8       ; release a and b:  (-4) - (-12) = 8
    POP EBP
    RET 8            ; pop x and y: 2 params * 4 bytes = 8

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
