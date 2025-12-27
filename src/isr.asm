global isr_keyboard
global isr_ud
global isr_gp
global isr_df

; Kernel panic function 
extern k_panic

; Keyboard isr handler function
extern keyboard_handler

; Keyboard ISR wrapper
isr_keyboard:
    pusha
    call keyboard_handler
    popa
    iretd

isr_ud:
    cli
    call k_panic
    jmp isr_hang

isr_gp:
    cli
    pop eax ; move error code into eax
    call k_panic
    jmp isr_hang

isr_df:
    cli
    pop eax ; move error code into eax
    call k_panic
    jmp isr_hang

isr_hang:
    hlt
    jmp isr_hang
