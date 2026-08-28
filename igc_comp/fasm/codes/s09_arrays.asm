; ============================================================
; Section 9 - Array Indexing
; Global array g[3]: uses [g + EBX] where EBX = index * 4.
; Local array local[3] at [EBP-4] through [EBP-12]:
;   uses [EBP + ESI] where ESI = -(index*4 + base_offset).
; Expected output:
;   0
;   100
;   200
;   0
;   10
;   20
; ============================================================

format ELF executable 3
entry main

segment readable writeable
    g dd 3 dup (0)      ; global int g[3]

segment readable executable
main:
    PUSH EBP
    MOV  EBP, ESP
    SUB  ESP, 12        ; local int local[3]: at [EBP-4],[EBP-8],[EBP-12]
                        ; base_offset = 4  (|nextLocalOffset when declared| = 4)

    ; --- fill global array ---
    ; g[0] = 0
    MOV EAX, 0
    MOV EBX, 4
    MUL EBX             ; byte offset = 0
    MOV EBX, EAX
    MOV EAX, 0
    MOV [g + EBX], EAX

    ; g[1] = 100
    MOV EAX, 1
    MOV EBX, 4
    MUL EBX             ; byte offset = 4
    MOV EBX, EAX
    MOV EAX, 100
    MOV [g + EBX], EAX

    ; g[2] = 200
    MOV EAX, 2
    MOV EBX, 4
    MUL EBX             ; byte offset = 8
    MOV EBX, EAX
    MOV EAX, 200
    MOV [g + EBX], EAX

    ; --- fill local array ---
    ; local[0] = 0   (index*4 + base_offset = 0+4 = 4, ESI = -4)
    MOV EAX, 0
    MOV EBX, 4
    MUL EBX             ; 0
    MOV EBX, 4          ; base_offset
    ADD EAX, EBX        ; 4
    MOV ESI, EAX
    NEG ESI             ; -4
    MOV EAX, 0
    MOV [EBP + ESI], EAX

    ; local[1] = 10  (1*4 + 4 = 8, ESI = -8)
    MOV EAX, 1
    MOV EBX, 4
    MUL EBX             ; 4
    MOV EBX, 4
    ADD EAX, EBX        ; 8
    MOV ESI, EAX
    NEG ESI             ; -8
    MOV EAX, 10
    MOV [EBP + ESI], EAX

    ; local[2] = 20  (2*4 + 4 = 12, ESI = -12)
    MOV EAX, 2
    MOV EBX, 4
    MUL EBX             ; 8
    MOV EBX, 4
    ADD EAX, EBX        ; 12
    MOV ESI, EAX
    NEG ESI             ; -12
    MOV EAX, 20
    MOV [EBP + ESI], EAX

    ; --- print global array ---
    MOV EAX, 0
    MOV EBX, 4
    MUL EBX
    MOV EBX, EAX
    PUSH EAX
    MOV  EAX, [g + EBX]
    CALL OUTDEC
    POP  EAX

    MOV EAX, 1
    MOV EBX, 4
    MUL EBX
    MOV EBX, EAX
    PUSH EAX
    MOV  EAX, [g + EBX]
    CALL OUTDEC
    POP  EAX

    MOV EAX, 2
    MOV EBX, 4
    MUL EBX
    MOV EBX, EAX
    PUSH EAX
    MOV  EAX, [g + EBX]
    CALL OUTDEC
    POP  EAX

    ; --- print local array ---
    MOV EAX, 0
    MOV EBX, 4
    MUL EBX
    MOV EBX, 4
    ADD EAX, EBX
    MOV ESI, EAX
    NEG ESI
    PUSH EAX
    MOV  EAX, [EBP + ESI]
    CALL OUTDEC
    POP  EAX

    MOV EAX, 1
    MOV EBX, 4
    MUL EBX
    MOV EBX, 4
    ADD EAX, EBX
    MOV ESI, EAX
    NEG ESI
    PUSH EAX
    MOV  EAX, [EBP + ESI]
    CALL OUTDEC
    POP  EAX

    MOV EAX, 2
    MOV EBX, 4
    MUL EBX
    MOV EBX, 4
    ADD EAX, EBX
    MOV ESI, EAX
    NEG ESI
    PUSH EAX
    MOV  EAX, [EBP + ESI]
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
