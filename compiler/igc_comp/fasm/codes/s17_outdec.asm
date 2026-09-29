; ============================================================
; Section 17 - The OUTDEC Procedure (Printing Integers)
; Tests OUTDEC with a range of values:
;   0, 1, -1, 42, -42, 100, 12345, -99999
; Expected output:
;   0
;   1
;   -1
;   42
;   -42
;   100
;   12345
;   -99999
; ============================================================

format ELF executable 3
entry main

segment readable writeable

segment readable executable
main:
    PUSH EBP
    MOV  EBP, ESP

    MOV EAX, 0
    CALL OUTDEC

    MOV EAX, 1
    CALL OUTDEC

    MOV EAX, -1
    CALL OUTDEC

    MOV EAX, 42
    CALL OUTDEC

    MOV EAX, -42
    CALL OUTDEC

    MOV EAX, 100
    CALL OUTDEC

    MOV EAX, 12345
    CALL OUTDEC

    MOV EAX, -99999
    CALL OUTDEC

main_exit:
    MOV EAX, 1
    XOR EBX, EBX
    INT 0x80
    ADD ESP, 0
    POP EBP
    RET

; ---- OUTDEC: prints integer in EAX as decimal + newline ----
; Algorithm:
;   1. If EAX < 0: print '-', negate EAX.
;   2. Repeatedly divide EAX by 10; push each remainder (a digit).
;   3. Pop and print each digit (reverses extraction order).
;   4. Print newline.
OUTDEC:
    PUSH EBX
    PUSH ECX
    PUSH EDX
    PUSH ESI

    ; step 1: handle negative
    OR   EAX, EAX           ; sets SF flag; does not change EAX
    JGE  OUTDEC_POSITIVE
    NEG  EAX                ; make positive
    PUSH EAX                ; save for later
    SUB  ESP, 4
    MOV  byte [ESP], '-'    ; write '-' to a stack buffer
    MOV  EAX, 4             ; sys_write
    MOV  EBX, 1             ; stdout
    MOV  ECX, ESP           ; buffer = [ESP]
    MOV  EDX, 1             ; length = 1
    INT  0x80
    ADD  ESP, 4             ; free the '-' buffer
    POP  EAX                ; restore positive value

OUTDEC_POSITIVE:
    XOR  ECX, ECX           ; ECX = digit count
    MOV  EBX, 10            ; divisor

    ; step 2: extract digits (least significant first)
OUTDEC_DIGIT_LOOP:
    XOR  EDX, EDX           ; clear EDX (DIV reads EDX:EAX)
    DIV  EBX                ; EAX = EAX/10, EDX = EAX%10
    ADD  DL, 30h            ; convert digit to ASCII ('0' = 0x30)
    PUSH EDX                ; push the character (stack reverses the order)
    INC  ECX
    TEST EAX, EAX
    JNZ  OUTDEC_DIGIT_LOOP  ; continue while quotient != 0

    ; step 3: print digits (pop gives most-significant first)
    MOV  ESI, ECX           ; save count
    MOV  EBX, 1
    MOV  EDX, 1

OUTDEC_PRINT_LOOP:
    TEST ESI, ESI
    JZ   OUTDEC_NEWLINE
    MOV  EAX, 4             ; sys_write
    MOV  ECX, ESP           ; [ESP] holds the next digit byte
    INT  0x80
    ADD  ESP, 4             ; pop the digit
    DEC  ESI
    JMP  OUTDEC_PRINT_LOOP

    ; step 4: print newline
OUTDEC_NEWLINE:
    SUB  ESP, 4
    MOV  byte [ESP], 10     ; ASCII 10 = '\n'
    MOV  EAX, 4
    MOV  ECX, ESP
    INT  0x80
    ADD  ESP, 4

    POP  ESI
    POP  EDX
    POP  ECX
    POP  EBX
    RET
