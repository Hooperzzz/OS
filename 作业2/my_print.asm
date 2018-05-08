section .data

color_red:  db 1Bh,'[31;1m',0
red_len:       equ $ - color_red
color_blue: db 1Bh,'[34;1m',0
blue_len:      equ $ - color_blue
color_Default: db 1Bh, '[3333;0m',0
default_len:   equ $ - color_Default

section .text

global my_print  ;string str,int colorNum

my_print:
    push ebp
    mov ebx,[esp+8]
    mov eax,[esp+12]
    mov edx,[esp+16]
    
    push edx
    push eax
    push ebx
    cmp edx,0
    je toRed
    mov ecx,color_blue
    mov edx,blue_len
    jmp changeColor
toRed:
    mov ecx,color_red
    mov edx,red_len
changeColor:
    mov eax,4
    mov ebx,1
    int 80h
    pop ebx
    pop eax
    pop edx

    mov edx,eax
    mov ecx,ebx
    mov eax,4
    mov ebx,1
    int 80h

    mov eax,4
    mov ebx,1
    mov ecx,color_Default
    mov edx,default_len
    int 80h

    pop ebp
    ret
    
