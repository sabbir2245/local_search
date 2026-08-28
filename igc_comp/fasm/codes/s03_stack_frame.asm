; ============================================================
; Section 3 - Stack Frame / Function Prologue & Epilogue
; Calls add(3, 5) and prints the result.
; Stack layout inside add:
;   [EBP+12] = b = 5
;   [EBP+8]  = a = 3
;   [EBP+4]  = return address
;   [EBP+0]  = saved EBP
; Expected output:
;   8
; ============================================================

format ELF executable 3
entry main

segment readable writeable

segment readable executable
main:
    PUSH EBP
    MOV  EBP, ESP

    ; call add(3, 5): push arguments right-to-left
    MOV EAX, 5       ; second argument (b)
    PUSH EAX
    MOV EAX, 3       ; first argument (a)
    PUSH EAX
    CALL add_nums     ; EAX = 8 after return (callee pops 8 bytes via RET 8)

    ; print the return value (already in EAX)
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

; int add_nums(int a, int b)  -- named add_nums because "add" is a FASM keyword
add_nums:
    PUSH EBP
    MOV  EBP, ESP
    ; no local variables

    MOV EAX, [EBP+8]   ; load a (first parameter)
    MOV EBX, [EBP+12]  ; load b (second parameter)
    ADD EAX, EBX        ; EAX = a + b

add_nums_exit:
    ADD ESP, 0          ; no locals to release
    POP EBP
    RET 8               ; return and pop 2 x 4-byte parameters

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
