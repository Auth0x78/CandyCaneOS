# Environment Setup
set architecture i386
set disassembly-flavor intel

# Connection to QEMU
# Connects to the -s (port 1234) stub in QEMU
target remote localhost:1234

# 3. Symbol Loading
# Adjust path to match your Makefile's debug output
symbol-file build/debug/kernel.bin


# Break at your kernel_main entry point
break kernel_main

# Continue execution until the breakpoint is hit
continue

# Debugger UI Enhancements
# 'split' shows source and assembly; 'regs' shows CPU state
layout split

# Break at _start in boot.asm
# break _start 

# Helpful Hooks
# This ensures that whenever the OS stops, we see the instructions
define hook-stop
    info registers
    x/5i $pc
end

# Focus on the command window (cmd)
focus cmd
