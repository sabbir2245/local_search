; ============================================================
; Section 1 - FASM File Structure
; This is the minimal skeleton for any FASM ELF program.
; It just exits with code 0. No output is produced.
; Verify: run the program and check  echo $?  => 0
; ============================================================

format ELF executable 3   ; produce a 32-bit Linux ELF binary
entry main                 ; the OS jumps to "main" first

segment readable writeable
    ; data segment: global variables go here (empty for this example)

segment readable executable
main:
    PUSH EBP
    MOV  EBP, ESP

    ; nothing to do — just exit

main_exit:
    MOV EAX, 1    ; syscall number: sys_exit
    XOR EBX, EBX  ; exit code 0 (success)
    INT 0x80

    ADD ESP, 0    ; release locals (none here)
    POP EBP
    RET
