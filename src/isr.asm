global isr_keyboard
global isr_ud
global isr_gp
global isr_df

extern keyboard_handler
isr_keyboard:
    pusha
    call keyboard_handler
    popa
    iretd

isr_ud:
    cli
    jmp isr_hang

isr_gp:
    cli
    pop eax ; move error code into eax
    jmp isr_hang

isr_df:
    cli
    pop eax ; move error code into eax
    jmp isr_hang

isr_hang:
    hlt
    jmp isr_hang
