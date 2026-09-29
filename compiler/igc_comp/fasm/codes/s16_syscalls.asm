; ============================================================
; Section 16 - Linux System Calls (INT 0x80)
; Demonstrates sys_write (eax=4) and sys_exit (eax=1).
; Writes the string "Hello\n" directly (no OUTDEC needed).
; Then exits with code 0.
; Expected output:
;   Hello
; ============================================================

format ELF executable 3
entry main

segment readable writeable
    msg db "Hello", 10   ; "Hello\n"  (10 = ASCII newline)
    msg_len = $ - msg    ; length computed by FASM

segment readable executable
main:
    PUSH EBP
    MOV  EBP, ESP

    ; sys_write(fd=1, buf=msg, count=msg_len)
    MOV EAX, 4         ; syscall: sys_write
    MOV EBX, 1         ; fd: stdout
    MOV ECX, msg       ; pointer to the string
    MOV EDX, msg_len   ; number of bytes
    INT 0x80

main_exit:
    ; sys_exit(code=0)
    MOV EAX, 1
    XOR EBX, EBX       ; exit code 0
    INT 0x80
    ADD ESP, 0
    POP EBP
    RET
