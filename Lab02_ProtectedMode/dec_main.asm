.386p 


descr struc     
	lim 	dw 0				; Limit of segment
	base_l 	dw 0				; Low base part
	base_m 	db 0				; Middle base part
	attr_1	db 0				
	attr_2	db 0	
	base_h 	db 0				; High base part
descr ends

int_descr struc 
	offs_l 	dw 0 				; Low offset part
	sel		dw 0				; Segment selector that contains int. handler
	counter db 0  				; Not in use
	attr	db 0  
	offs_h 	dw 0  				; High offset part
int_descr ends


PM_seg	SEGMENT PARA PUBLIC 'CODE' USE32		
												                    
										                        
	                ASSUME	CS:PM_seg

    
  	GDT		label	byte

  	
  	gdt_null	descr <>

  	
	; This segment has granularity bit setted
  	gdt_flatDS	descr <0FFFFh,0,0,92h,11001111b,0>	

  	
  	gdt_16bitCS	descr <RM_seg_size-1,0,0,98h,0,0>	

  	
  	gdt_32bitCS	descr <PM_seg_size-1,0,0,98h,01000000b,0>

  	
  	gdt_32bitDS	descr <PM_seg_size-1,0,0,92h,01000000b,0>

  	
  	gdt_32bitSS	descr <stack_l-1,0,0, 92h, 01000000b,0>

  	gdt_size = $-GDT 

  	gdtr	df 0	

    
    SEL_flatDS     equ   8
    SEL_16bitCS    equ   16
    SEL_32bitCS    equ   24
    SEL_32bitDS    equ   32
    SEL_32bitSS    equ   40

    
    IDT	label	byte

    
    int_descr 32 dup (<0, SEL_32bitCS,0, 8Eh, 0>)

    
    int08 int_descr <0, SEL_32bitCS,0, 8Eh, 0>

    
    int09 int_descr	<0, SEL_32bitCS,0, 8Eh, 0>

    idt_size = $-IDT 

    idtr		df 0 

    idtr_real 	dw	3FFh,0,0 

    master		db 0					 
    slave		db 0					 

    escape		db 0				 
    time_08		dd 0				 

	msg1 		db 'We are in Real Mode now. To move to Protected Mode press any key...$'
	msg2 		db 'In Real Mode again!$'

		
		
		ASCII_table	db 0, 0, 49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 45, 61, 0, 0
					db 81, 87, 69, 82, 84, 89, 85, 73, 79, 80, 91, 93, 0, 0, 65, 83
					db 68, 70, 71, 72, 74, 75, 76, 59, 39, 96, 0, 92, 90, 88, 67
					db 86, 66, 78, 77, 44, 46, 47
		out_position	dd 1E0h 


		
create_number macro
		add edx, '0'
endm

print_eax macro
	local next1
	local print1
	local finish1
	
	add ebp, 0B8010h
	
	push eax
	
							; Pushing stop-symbol
	mov edx, 'x'
	push edx
	

next1:
	
	mov edx, 0
	mov ecx, 10
	div ecx 				; eax = eax / ecx; edx = R(eax / ecx)
	push edx 				; Pushing remainder
	cmp eax, 0
	jne next1
	
print1:
	pop edx
	cmp edx, 'x'
	je finish1
	create_number 0
	mov es:[ebp],dl
	add ebp, 2
	jmp print1
	
finish1:
	;pop dx
	pop eax
	
	sub ebp, 0B8010h
endm


	
PM_entry:
		
		
		mov	ax,SEL_32bitDS			; Loading preprocessed selectors
		mov	ds,ax
		mov	ax,SEL_flatDS
		mov	es,ax
		mov	ax,SEL_32bitSS
		mov	ebx,stack_l
		mov	ss,ax
		mov	esp,ebx

		
		sti 						; Now we can enable maskable int-s

		
		call	compute_memory

		
		
work:								; Infinite loop, waiting for int-s 8 or 9
		test	escape, 1
		jz	work

goback:
									; Returning to RM
		
		cli 						; Disabling maskable int-s

		
		db	0EAh 					; far jump to PM_return		
		dd	offset RM_return		; It is placed in 16-bit segment
		dw	SEL_16bitCS


	
new_int08:
		
		push eax
		push ebp
		push ecx
		push dx
		mov  eax,time_08

		
		push ebp
		mov ebp, 0					
		print_eax			
		pop ebp							

		inc eax
		mov time_08,eax

		
		pop dx
		pop ecx
		pop ebp

		
		mov	al,20h
		out	20h,al
		pop eax

		iretd 

		
new_int09: ;; SCAN CODES: http://www.ee.bgu.ac.il/~microlab/MicroLab/Labs/ScanCodes.htm
			push eax
			push ebx
			push ebp
			push edx

			in	al,60h 		; Reading scan-code from keyboard 

			cmp	al,1Ch 		; Comparison with ENTER	     
			jne	not_leave 	 
			mov escape,1     
			jmp leav
not_leave:
			cmp al,80h 		; Comparing: if button was pressed or released
			ja leav 		; Released - leaving
			
			push ax
			
			cmp al, 0Eh 	; Comparing with backspace
			jne print_symbol
			
			sub out_position, 2
			
			mov al, 0

print_symbol:
			xor ah,ah	 
			mov bp,ax
			mov dl,ASCII_table[ebp] 
			mov ebp,0B8000h
			mov ebx,out_position   
			mov es:[ebp+ebx],dl

			pop ax
			cmp al, 0Eh ; Comparing with backspace again
			je leav
			
			add ebx,2			   
			mov out_position,ebx
			
leav:
			; Reenabling keyboard processing
			in	al,61h
			or	al,80h
			out	61h,al

			; EOI
			mov	al,20h
			out	20h,al

			pop edx
			pop ebp
			pop ebx
			pop	eax

			
			iretd






compute_memory	proc

		push	ds            
		mov	ax, SEL_flatDS	
		mov	ds, ax					
		mov	ebx, 100001h		
		mov	dl,	10101010b	  
												

		mov	ecx, 0FFEFFFFEh	

		
check:
		mov	dh, ds:[ebx]		
												
												
												
		mov	ds:[ebx], dl		
		cmp	ds:[ebx], dl		
		jnz	end_of_memory		
		mov	ds:[ebx], dh		
		inc	ebx							
												
												
		loop	check
end_of_memory:
		pop	ds							
		xor	edx, edx
		mov	eax, ebx				
		mov	ebx, 100000h		
		div	ebx

		push ebp
		mov ebp,20					
		print_eax		
		pop ebp							

		ret
	compute_memory	endp


	PM_seg_size = $-GDT
PM_seg	ENDS



stack_seg	SEGMENT  PARA STACK 'STACK'
	stack_start	db	100h dup(?)
	stack_l = $-stack_start							
stack_seg 	ENDS





RM_seg	SEGMENT PARA PUBLIC 'CODE' USE16		
	ASSUME CS:RM_seg, DS:PM_seg, SS:stack_seg

start:

		mov   ax,PM_seg
		mov   ds,ax

		mov ah, 09h
		mov edx, offset msg1
		int 21h

		
		push eax
		mov ah,10h
		int 16h
		pop eax

		
		mov	ax,3									; Clear screen
		int	10h

		
		push PM_seg									; Setting shadow register
		pop ds										; of DS

		
		xor	eax,eax									; Base address calculation
		mov	ax,RM_seg
		shl	eax,4									; PARA Alignment		
		mov	word ptr gdt_16bitCS.base_l,ax
		shr	eax,16
		mov	byte ptr gdt_16bitCS.base_m,al
		mov	ax,PM_seg
		shl	eax,4
		push eax									; For IDT address		
		push eax									; For GDT address
		mov	word ptr GDT_32bitCS.base_l,ax
		mov	word ptr GDT_32bitSS.base_l,ax
		mov	word ptr GDT_32bitDS.base_l,ax
		shr	eax,16
		mov	byte ptr GDT_32bitCS.base_m,al
		mov	byte ptr GDT_32bitSS.base_m,al
		mov	byte ptr GDT_32bitDS.base_m,al

		
		pop eax										; Calculating GDT linear
		add	eax,offset GDT 							; EAX = Full GDT linear address			
													; In real mode all addresses are virtual
		mov	dword ptr gdtr+2,eax					; Low bytes = linear address	
		mov word ptr gdtr, gdt_size-1				; Hight bytes = gdt size - 1
		
		lgdt	fword ptr gdtr

													; Same for IDT
		
		pop	eax
		add	eax,offset IDT
		mov	dword ptr idtr+2,eax
		mov word ptr idtr, idt_size-1

													; Calculating offsets
		mov	eax, offset new_int08 
		mov	int08.offs_l, ax
		shr	eax, 16
		mov	int08.offs_h, ax
		mov	eax, offset new_int09 
		mov	int09.offs_l, ax
		shr	eax, 16
		mov	int09.offs_h, ax

													; Saving int. masks
		in	al, 21h							
		mov	master, al					
		in	al, 0A1h						
		mov	slave, al

													; Programming master PIC
		mov	al, 11h									; CMD "Init master"
		out	20h, al									; PORT 20h "On/Off"			
		mov	al, 20h									; Base vector = 32 (dec.)			
		out	21h, al									; CMD "Set base vector"					
				
													
		mov	al, 4
		out	21h, al
		
		mov	al, 1									; CMD "Need to send EOI"		  
		out	21h, al

		
		mov	al, 0FCh								; Disabling all IRQs, except IRQ0 and IRQ1
		out	21h, al									; on master
		
		
		mov	al, 0FFh								; Disabling all IRQs
		out	0A1h, al								; on slave

		
		lidt	fword ptr idtr

		
													; Opening line A20
		in	al,92h						
		or	al,2							
		out	92h,al		
		
		
		cli											; Disabling maskable int-s
								
													; Disabling unmaskable int-s
		in	al,70h
		or	al,80h
		out	70h,al

													; Modificating CR0
		mov	eax,cr0									; to enter PM
		or	al,1
		mov	cr0,eax

		
		db	66h										; Jumping to PM_entry
		db	0EAh
		dd	offset PM_entry
		dw	SEL_32bitCS
		
		
		

RM_return:
								; Reentering RM
		mov	eax,cr0
		and	al,0FEh 			; Clearing PM flag
		mov	cr0,eax

								; Assembler restricts access to CS
		db	0EAh						
		dw	$+4					; Jumping forward		
		dw	RM_seg

								; Jump target
								; Restoring registers for working in RM
		mov	ax,PM_seg		
		mov	ds,ax
		mov	es,ax
		mov	ax,stack_seg
		mov	bx,stack_l
		mov	ss,ax
		mov	sp,bx

								; Reprogramming master PIC to vector 8
		mov	al, 11h				; INIT		
		out	20h, al
		mov	al, 8				; Vector	
		out	21h, al
		mov	al, 4						
		out	21h, al
		mov	al, 1
		out	21h, al

								; Restoring controller masks
		mov	al, master
		out	21h, al
		mov	al, slave
		out	0A1h, al

		
		lidt	fword ptr idtr_real

								; Reenabling unmaskable int-s
		in	al,70h
		and	al,07FH
		out	70h,al

    
		sti						; Reenabling maskable int-s

		
		mov	ax,3				; Clear screen
		int	10h

								; Printing message
		mov ah, 09h
		mov edx, offset msg2
		int 21h


								; Terminating process
		mov	ah,4Ch
		int	21h

RM_seg_size = $-start 	
RM_seg	ENDS
END start



