format ELF executable
entry main

segment readable executable

main:
    push ebp
    mov ebp, esp
    mov eax, 0
    pop ebp
    ret
