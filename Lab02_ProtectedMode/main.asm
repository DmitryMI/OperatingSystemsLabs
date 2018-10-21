; Changing processor mode to Protected and back
.386P

create_number macro
		local number1
			cmp al,10
			jl number1
			add al,'A' - '0' - 10
		number1:
			add al,'0'
endm

PrintDec macro
		push eax
		push ebx			
		mov ebx, eax
		xor eax, eax	
		
		add di, 14

		std
		
		REPT 8
			mov ah, 1Eh
			mov al, bl
			and al, 0Fh
			create_number
			stosw
			
			ror ebx, 4
		ENDM
		
		add di, 14
		
		pop ebx
		pop eax
endm



; Segment description structure
descr struc
    lim     dw 0
    base_1  dw 0
    base_m  db 0
    attr_1  db 0
    attr_2  db 0
    base_h  db 0
descr ends

; Data segment
data segment use16
    gdt_null descr <0,0,0,0,0,0>
    gdt_data descr <data_size-1, 0, 0, 92h, 0, 0>
    gdt_code descr <code_size-1, 0, 0, 98h, 0, 0>
    gdt_stack descr <255, 0, 0, 92h, 0, 0>
    gdt_screen descr <3999, 8000h, 0Bh, 92h, 0, 0>
    gdt_size = $-gdt_null

    pdescr  df 0
    sym     db 1
    attr    db 1Eh
	msg 	db 27, ' We are in real mode ', 27, '$'
	msg_before_protected db 27, 'Preparations complete. Changing to protected mode...', '$'
    msg_memory db 'Available memory: ', '$'
	
	number_cell dd 123456789

    data_size = $-gdt_null
data ends


text segment use16
    assume CS:text, DS:data

main    proc

    xor eax, eax
    mov ax, data
    mov ds, ax
	
    ; Data segment loading
    shl eax, 4
    mov ebp, eax
    mov bx, offset gdt_data
    mov [bx].base_1, ax
    shr eax, 16
    mov [bx].base_m, al

    ; Code segment loading
    xor eax, eax
    mov eax, cs
    shl eax, 4
    mov bx, offset gdt_code
    mov [bx].base_1, ax
    shr eax, 16
    mov [bx].base_m, al

    ; Stack segment loading
    xor eax, eax
    mov eax, ss
    shl eax, 4
    mov bx, offset gdt_stack
    mov [bx].base_1, ax
    shr eax, 16
    mov [bx].base_m, al

    ; Loading GDTR
    mov dword ptr pdescr + 2, ebp
    mov word ptr pdescr, gdt_size - 1
    lgdt pdescr
    
    ; Preparing for going back to real mode
    ;mov ax, 40h
    ;mov es, ax
    ;mov word ptr es:[67h], offset return
    ;mov es:[69h], cs
    ;mov al, 0Fh
    ;out 70h, al
    ;mov al, 0Ah
    ;out 71h, al
    cli
	
	; Outputting message
	push ax
	push dx	
    mov ah, 09h
    mov dx, offset msg_before_protected
    int 21h	
	pop dx
	pop ax
		

    ; Changing mode to protected
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ;-------------------------------------------------;
    ;        We are in protected mode now             ;
    ;-------------------------------------------------;

    db 0EAh ; far jump
    dw offset continue
    dw 16
	
continue:
    xor eax, eax
    ; Make data addressable
    mov ax, 8
    mov ds, ax

    ; Make stack addressable
    mov ax, 24
    mov ss, ax

    ; Initializing video buffer
    mov ax, 32
    mov es, ax

    ; Outputting message
    ;mov di, 1920
    ;mov cx, 80
    ;mov ax, word ptr sym
;scrn: 
    ;stosw
    ;inc al
    ;loop scrn
	
	
	
	call MenuProc



    ; Going back to real mode

    mov gdt_data.lim, 0FFFFh
    mov gdt_code.lim, 0FFFFh
    mov gdt_stack.lim, 0FFFFh
    mov gdt_screen.lim, 0FFFFh

    ; Shadow registers loading
    push ds
    pop ds
    push ss
    pop ss
    push es
    pop es

    ; Loading CS shadow register
    db 0EAh ; FAR JUMP
    dw offset go
    dw 16

    ; Switching processor mode
go:
    mov eax, cr0
    and eax, 0FFFFFFFEh
    mov cr0, eax
    db 0EAh ; FAR JUMP
    dw offset return
    dw text

    ;-------------------------------------------------;
    ;        We are in REAL mode again                ;
    ;-------------------------------------------------;
return:
    ; Restoring context of real mode

    ; Making segments addressable
	xor eax, eax
    mov ax, data
    mov ds, ax
    mov ax, stk
    mov ss, ax
    mov sp, 256
    sti

    ; Outputting message
    mov ah, 09h
    mov dx, offset msg
    int 21h
    mov eax, 4C00h
    int 21h

main endp



MenuProc proc
	
	; Calculating available memory
	;call CalcMem
	mov eax, 123456
	mov ebx, 654321
	
	push eax
	push ebx
	
	xor ebx, ebx	
	
	; Printing available memory
	mov di, 1600
    mov cx, 64	
	mov bl, offset msg_memory	
mem_print:
	mov ah, 1Eh
	mov al, [bx]
	cmp al, '$'
	je mem_print_end
	stosw
	inc bx	
	loop mem_print	
mem_print_end:

	pop ebx
	pop eax
	
	mov eax, 12345678h
	PrintDec
	
	ret
MenuProc endp


CalcMem proc ; RETURNS EAX
    push ebp
    mov ebp, esp

	mov ebx, 100001h
	mov dl, 10101010b
	
	mov ecx, 0FFEFFFFEh	; Memory limit
	
count_loop:
	mov dh, ds:[ebx]	
	mov ds:[ebx], dl
	cmp ds:[ebx], dl
	jnz count_loop_finish
	mov ds:[ebx], dh
	inc ebx
	loop count_loop
	
count_loop_finish:
	xor edx, edx
	mov eax, ebx
	mov ebx, 100000h
	div ebx	
	
	pop ebp
	    
	ret    
CalcMem endp






code_size = $-main
text ends


stk segment stack use16
    db 256 dup ('^')
stk ends

end main