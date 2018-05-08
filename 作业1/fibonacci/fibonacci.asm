section .data
inputIntroFrom: db 'Please input number x and y:', 0Ah,0

spaceOne: db 0Ah

color_red:  db 1Bh,'[31;1m',0
red_len:       equ $ - color_red
color_blue: db 1Bh,'[34;1m',0
blue_len:      equ $ - color_blue
color_Default: db 1Bh, '[3333;0m',0
default_len:   equ $ - color_Default

section .bss
currentColor: resb 4 ;当前颜色
inputAll: resb 80 ;input number
inputFrom: resb 40 ;input number from
inputTo: resb 40 ;input number to
temp: resb 1000 ; temp storage
fSub2: resb 1000
fSub1: resb 1000

section .text
global _start
_start:;主程序地址
    ;改变输出颜色
    mov dword[currentColor],1
    call changeColor
    call changeColor

    ;输入整数起始
    mov eax,inputIntroFrom
    call changeColor
    call output
    mov ecx,inputAll
    mov edx,80
    call input

    ;以空格分割输入字符串
    push eax
    push ecx
    push ebx
    push edx
    mov ecx,inputAll
    mov eax,inputFrom
    mov ebx,inputTo
    GetNumFrom:
        mov dl,byte[ecx]
        mov byte[eax],dl
        inc ecx
        inc eax
        cmp byte[ecx],20h
        jne GetNumFrom
    mov dl,20h
    mov byte[eax],dl
    inc ecx
    GetNumTo:
        mov dl,byte[ecx]
        mov byte[ebx],dl
        inc ebx
        inc ecx
        cmp byte[ecx],0
        jne GetNumTo
    pop edx
    pop ebx
    pop ecx
    pop eax

    mov eax,inputFrom
    call getStrRevert
    call getNumAfterRevert
    push eax
    mov eax,inputTo
    call getStrRevert
    call getNumAfterRevert
    mov ebx,eax
    pop eax

    ;用函数计算并输出(此函数使用了fSub1,fSub2),不通用
    call calculateNormal

    ;exit system
    mov eax,1
    mov ebx,0
    int 80h

    ;计算每一项并输出,项数存在eax,ebx中
calculateNormal:
    push ecx
    push edx
    mov cl,31h
    mov ch,30h
    mov byte[fSub1],cl
    mov byte[fSub2],ch
    cmp eax,2
    ja calculateFinish
    cmp eax,2
    je fromEqualTwo
    push eax
    mov eax,fSub2
    call changeColor
    call output
    mov eax,spaceOne
    call output
    mov eax,fSub1
    call changeColor
    call output
    pop eax
    jmp calculateFinish
    fromEqualTwo:
        push eax
        mov eax,fSub1
        call changeColor
        call output
        pop eax
    calculateFinish:
    push eax
    mov eax,spaceOne
    call output
    pop eax
    mov edx,2

    calculateInnerLoop: ;输出数列的循环
    push eax
    push ebx
    push ecx
    mov eax,fSub1
    mov ebx,fSub2
    mov ecx,temp
    call calculateSingle
    pop ecx
    pop ebx
    pop eax
    cmp eax,edx
    ja noOutput

    push eax
    mov eax,temp
    call countStart
    push ebx
    push ecx
    mov bl,0Ah
    mov ecx,temp
    add eax,ecx
    mov byte[eax],bl
    push eax
    mov eax,temp
    call getStrRevert
    call changeColor
    call output
    pop eax
    mov byte[eax],bl
    mov eax,temp
    call getStrRevert
    pop ecx
    pop ebx
    mov eax,spaceOne
    call output
    pop eax

    noOutput:
    push eax
    push ebx
    mov eax,fSub1
    mov ebx,fSub2
    call copyString
    mov eax,temp
    mov ebx,fSub1
    call copyString
    pop ebx
    pop eax
    inc edx
    cmp ebx,edx
    jnb calculateInnerLoop
    pop edx
    pop ecx
    ret

    ;高精度加法运算，加数在eax，ebx，结果在ecx
calculateSingle:
    push edx
    push eax
    mov dl,0h
    calculateSingleLoop:
        push eax
        push ebx
        push ecx
        mov cl,byte[eax]
        cmp cl,30h
        jnb fSub1NotZero
        add cl,30h
        fSub1NotZero:
        mov al,cl
        mov cl,byte[ebx]
        cmp cl,30h
        jnb fSub2NotZero
        add cl,30h
        fSub2NotZero:
        add al,cl
        pop ecx
        mov bl,dl
        add al,bl
        sub al,30h
        cmp al,3Ah
        jb belowTen
        sub al,0Ah
        mov dl,1h
        jmp belowFinish
        belowTen:
        mov dl,0h
        belowFinish:
        mov byte[ecx],al
        inc ecx
        pop ebx
        pop eax
        inc eax
        inc ebx
        cmp byte[eax],0
        jne calculateSingleLoop
        cmp byte[ebx],0
        jne calculateSingleLoop
        cmp dl,1h
        je calculateSingleLoop
    pop eax
    pop edx
    ret

    ;拷贝字符串，目标地址在ebx，字符串地址在eax
copyString:
    push edx
    copyStringLoop:
        mov dl,byte[eax]
        mov byte[ebx],dl
        inc eax
        inc ebx
        cmp byte[eax],0h
        jne copyStringLoop
    pop edx
    ret

    ;得到反转字符串后对应的数字，地址和结果都在eax中
getNumAfterRevert:
    push ebx
    push ecx
    push edx
    mov ecx,1
    mov ebx,eax
    mov edx,0
    mov eax,0
    GetNumLoop:
        mov dl,byte[ebx]
        sub dl,30h
        inc ebx
        push eax
        mov eax,ecx
        mul dl
        mov edx,eax
        push edx
        mov eax,ecx
        mov ecx,10
        mul ecx
        mov ecx,eax
        pop edx
        pop eax
        add eax,edx
        cmp byte[ebx],0
        jz GetNumLoopFinish
        cmp byte[ebx],32
        je GetNumLoopFinish
        jmp GetNumLoop
    GetNumLoopFinish:
    pop edx
    pop ecx
    pop ebx
    ret

    ;反转字符串,保存在eax中,只针对最后一个是空格或回车，结果删除空格或回车。
getStrRevert:
    ;str头地址在eax
    push eax
    push ebx
    push ecx
    push edx
    mov ebx,eax
    mov ecx,eax
    call countStart
    add ecx,eax
    dec ecx
    mov al,0h
    mov byte[ecx],al
    dec ecx
    RevertLoop:
        mov al,byte[ecx]
        mov ah,byte[ebx]
        mov byte[ecx],ah
        mov byte[ebx],al
        inc ebx
        dec ecx
        cmp ebx,ecx
        jb RevertLoop
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret

    ;输入的公共方法
input:
    ;ecx存放输入首地址
    ;edx存放输入位数
    push eax
    push ebx
    mov eax,3
    mov ebx,0
    int 80h
    pop ebx
    pop eax
    ret
    ;输出的公共方法
output:
    ;eax存放首地址
    push eax
    push ecx
    push ebx
    push edx
    mov ecx,eax
    call countStart
    mov edx,eax
    mov ebx,1
    mov eax,4
    int 80h
    pop edx
    pop ebx
    pop ecx
    pop eax
    ret

    ;改变输出颜色
changeColor:
    push eax
    push ebx
    push ecx
    push edx
    mov eax,dword[currentColor]
    cmp eax,2
    ja changeRed
    cmp eax,1
    ja changeBlue
    jmp changeDefault
    changeBlue:
        mov ecx,color_blue
        mov edx,blue_len
        jmp changeFinish
    changeDefault:
        mov ecx,color_Default
        mov edx,default_len
        jmp changeFinish
    changeRed:
        mov eax,0
        mov ecx,color_red
        mov edx,red_len
    changeFinish:
    inc eax
    mov dword[currentColor],eax
    mov eax,4
    mov ebx,1
    int 80h
    pop edx
    pop ecx
    pop ebx
    pop eax
    ret

    ;计算字符串长度公共方法
countStart:
    push ebx
    mov ebx,eax
count:
    ;eax存放字符串首地址,返回字符串数量
    cmp byte[eax], 0
    jz countFinished
    inc eax
    jmp count
countFinished:
    sub eax,ebx
    pop ebx
    ret
