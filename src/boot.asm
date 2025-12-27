; boot.asm - Multiboot-compliant bootloader for 32-bit mode

; Define Macros
%define MULTIBOOT_MAGIC  0x1BADB002
%define MULTIBOOT_FLAGS  0x00000007
%define MULTIBOOT_ZERO   0x00000000

section .multiboot
align 4
    ; Multiboot header
    dd MULTIBOOT_MAGIC                      ; Magic number
    dd MULTIBOOT_FLAGS                      ; Flags: align modules on page boundaries and provide memory map
    dd -(MULTIBOOT_MAGIC+MULTIBOOT_FLAGS)   ; Checksum
    
    ; Only valid if flag[16] is set 
    dd MULTIBOOT_ZERO                ; header_addr
    dd MULTIBOOT_ZERO                ; load_addr
    dd MULTIBOOT_ZERO                ; load_end_addr
    dd MULTIBOOT_ZERO                ; bss_end_addr
    dd MULTIBOOT_ZERO                ; entry_addr

    ; Only valid if flag[2] is set (VIDEO MODE)
    dd 0x00000000                     ; mode_type    
    dd 800                            ; width
    dd 600                            ; height
    dd 32                             ; depth

section .data
align 4
mboot_info_ptr:
    dd 0                         ; Store multiboot info pointer here
mboot_magic_number:
    dd 0                         ; Store the multiboot magic number here

section .bss
align 16
stack_bottom:
    resb 16384                   ; 16 KiB stack
stack_top:

section .text

; Global symbols to export from the asm
global _start
global mboot_info_ptr            ; Export memory info to C++

; Extern C functions
extern kernel_main

_start:
    ; Save multiboot info pointer passed in ebx by GRUB
    mov [mboot_info_ptr], ebx

    ; Save multiboot magic number passed in eax by GRUB
    mov [mboot_magic_number], eax

    ; Set up the stack
    mov esp, stack_top
    
    ; Make sure interrupts are disabled
    cli
    
    ; Load GDT (Global Descriptor Table) for 32-bit mode
    lgdt [gdt_descriptor]
    
    ; Update segment registers
    jmp 0x08:reload_segments     ; Far jump to code segment
    
reload_segments:
    mov ax, 0x10                 ; Data segment selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    
    ; Enable A20 line (might already be enabled by bootloader)
    in al, 0x92
    or al, 2
    out 0x92, al
    
    ; Disable interrupts
    cli
    
    ; Enable SSE execution
        ; --- CR0 Setup ---
        mov eax, cr0
        and ax, 0xFFFB      ; Clear CR0.EM (bit 2)
        or ax, 0x2          ; Set CR0.MP (bit 1)
        mov cr0, eax

        ; --- CR4 Setup ---
        mov eax, cr4
        or ax, (3 << 9)     ; Set CR4.OSFXSR (bit 9) and CR4.OSXMMEXCPT (bit 10)
        mov cr4, eax
    
    ; Reset FPU
    fninit

    ; Call the kernel main function
    push [mboot_info_ptr]       ; Pass multiboot info pointer to kernel
    
    mov eax, [mboot_magic_number]
    push eax                    ; Pass the magic number to kernel main
    
    call kernel_main
    
    ; If the kernel returns, hang the CPU
hang:
    cli                          ; Disable interrupts
    hlt                          ; Halt the CPU
    jmp hang                     ; Just in case

; GDT (Global Descriptor Table)
align 8
gdt:
    ; Null descriptor
    dq 0

    ; Code segment descriptor
    dw 0xFFFF       ; Limit (bits 0-15)
    dw 0            ; Base (bits 0-15)
    db 0            ; Base (bits 16-23)
    db 10011010b    ; Access byte: Present, Ring 0, Code Segment, Executable, Direction 0, Readable
    db 11001111b    ; Flags + Limit (bits 16-19): Granularity 1, 32-bit, Limit (bits 16-19)
    db 0            ; Base (bits 24-31)

    ; Data segment descriptor
    dw 0xFFFF       ; Limit (bits 0-15)
    dw 0            ; Base (bits 0-15)
    db 0            ; Base (bits 16-23)
    db 10010010b    ; Access byte: Present, Ring 0, Data Segment, Not Executable, Direction 0, Writable
    db 11001111b    ; Flags + Limit (bits 16-19): Granularity 1, 32-bit, Limit (bits 16-19)
    db 0            ; Base (bits 24-31)

gdt_descriptor:
    dw gdt_descriptor - gdt - 1  ; GDT size (minus 1)
    dd gdt                       ; GDT address