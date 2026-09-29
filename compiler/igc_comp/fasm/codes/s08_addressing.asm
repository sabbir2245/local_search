; ============================================================
; Section 8 - Memory Addressing Modes
; Exercises all five modes used by the compiler:
;   [label]           -- global scalar
;   [EBP - N]         -- local variable (negative displacement)
;   [EBP + N]         -- parameter    (positive displacement)
;   [label + EBX]     -- global array element
;   [EBP + ESI]       -- local array element (ESI is negative)
; Writes a value through each mode then reads it back and prints it.
; Expected output:
;   10
;   20
;   30
;   40
;   50
; ============================================================

format ELF executable 3
entry main

segment readable writeable
    g_scalar dd 1 dup (0)   ; for mode 1: [g_scalar]
    g_arr    dd 3 dup (0)   ; for mode 4: [g_arr + EBX]

segment readable executable

; helper: passes one int and prints it via OUTDEC
; demonstrates mode 3: [EBP+8] is the parameter
print_param:
    PUSH EBP
    MOV  EBP, ESP
    MOV  EAX, [EBP+8]   ; mode 3: positive displacement from EBP
    PUSH EAX
    CALL OUTDEC
    POP  EAX
    ADD  ESP, 0
    POP  EBP
    RET  4               ; callee pops 1 parameter

main:
    PUSH EBP
    MOV  EBP, ESP
    SUB  ESP, 12         ; 3 local ints: [EBP-4], [EBP-8], [EBP-12]

    ; Mode 1: [label] -- global scalar
    MOV EAX, 10
    MOV [g_scalar], EAX
    MOV EAX, [g_scalar]
    PUSH EAX
    CALL OUTDEC
    POP  EAX

    ; Mode 2: [EBP - N] -- local variable
    MOV EAX, 20
    MOV [EBP-4], EAX
    MOV EAX, [EBP-4]
    PUSH EAX
    CALL OUTDEC
    POP  EAX

    ; Mode 3: [EBP + N] -- parameter (via print_param)
    MOV EAX, 30
    PUSH EAX
    CALL print_param     ; prints [EBP+8] inside print_param

    ; Mode 4: [label + EBX] -- global array element (index 1)
    MOV EAX, 1           ; index
    MOV EBX, 4
    MUL EBX              ; byte offset = 4
    MOV EBX, EAX
    MOV EAX, 40
    MOV [g_arr + EBX], EAX
    MOV EAX, [g_arr + EBX]
    PUSH EAX
    CALL OUTDEC
    POP  EAX

    ; Mode 5: [EBP + ESI] -- local array element
    ; Local "array" starts at [EBP-4].  We want element index 2.
    ; base_offset = 4 (abs value of -4)
    ; byte_index  = 2 * 4 = 8
    ; total distance from EBP = 8 + 4 = 12, so [EBP-12]
    ; ESI = -(8 + 4) = -12  =>  [EBP + (-12)] = [EBP-12]
    MOV EAX, 2           ; index
    MOV EBX, 4
    MUL EBX              ; EAX = 8
    MOV EBX, 4           ; |base_offset|
    ADD EAX, EBX         ; EAX = 12
    MOV ESI, EAX
    NEG ESI              ; ESI = -12
    MOV EAX, 50
    MOV [EBP + ESI], EAX ; write 50 to [EBP-12]
    MOV EAX, [EBP + ESI]
    PUSH EAX
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
