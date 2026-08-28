; ============================================================
; Section 2 - Data Declarations (Global Variables)
; Declares a global int and a global int[5], assigns values,
; then prints them using OUTDEC.
; Expected output:
;   42
;   10
;   30
;   50
; ============================================================

format ELF executable 3
entry main

segment readable writeable
    x   dd 1 dup (0)    ; int x   -- 1 dword initialised to 0
    arr dd 5 dup (0)    ; int arr[5] -- 5 dwords initialised to 0

segment readable executable
main:
    PUSH EBP
    MOV  EBP, ESP

    ; x = 42
    MOV EAX, 42
    MOV [x], EAX

    ; arr[0] = 10  (byte offset = 0 * 4 = 0)
    MOV EAX, 0
    MOV EBX, 4
    MUL EBX
    MOV EBX, EAX        ; EBX = byte offset = 0
    MOV EAX, 10
    MOV [arr + EBX], EAX

    ; arr[2] = 30  (byte offset = 2 * 4 = 8)
    MOV EAX, 2
    MOV EBX, 4
    MUL EBX
    MOV EBX, EAX        ; EBX = 8
    MOV EAX, 30
    MOV [arr + EBX], EAX

    ; arr[4] = 50  (byte offset = 4 * 4 = 16)
    MOV EAX, 4
    MOV EBX, 4
    MUL EBX
    MOV EBX, EAX        ; EBX = 16
    MOV EAX, 50
    MOV [arr + EBX], EAX

    ; print x
    PUSH EAX
    MOV  EAX, [x]
    CALL OUTDEC
    POP  EAX

    ; print arr[0]
    MOV EAX, 0
    MOV EBX, 4
    MUL EBX
    MOV EBX, EAX
    PUSH EAX
    MOV  EAX, [arr + EBX]
    CALL OUTDEC
    POP  EAX

    ; print arr[2]
    MOV EAX, 2
    MOV EBX, 4
    MUL EBX
    MOV EBX, EAX
    PUSH EAX
    MOV  EAX, [arr + EBX]
    CALL OUTDEC
    POP  EAX

    ; print arr[4]
    MOV EAX, 4
    MOV EBX, 4
    MUL EBX
    MOV EBX, EAX
    PUSH EAX
    MOV  EAX, [arr + EBX]
    CALL OUTDEC
    POP  EAX

main_exit:
    MOV EAX, 1
    XOR EBX, EBX
    INT 0x80
    ADD ESP, 0
    POP EBP
    RET

; ---- OUTDEC: print integer in EAX as decimal, followed by newline ----
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
