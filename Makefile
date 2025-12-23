# --- Project Configuration ---
OS_NAME    := Candycane OS
SRC_DIR    := src
INC_DIR    := include
BUILD_ROOT := build
LD_FILE    := linker.ld

# --- Toolchain ---
CC         := gcc
ASM        := nasm
LD         := ld

# --- GDB Init file --- 
GDB_INIT := .gdbinit

# --- Base Compilation Flags ---
# We removed -O2 from here to set it per-mode
BASE_CC_FLAGS := -std=gnu2x -m32 -ffreestanding -fno-stack-protector \
                 -fno-pie -fno-pic -nostdlib -nostartfiles -I$(INC_DIR) -Wall -Wextra
LD_FLAGS      := -m elf_i386 -T $(LD_FILE) -z noexecstack

# --- Mode Switching Logic ---
# Default to release if no mode specified
MODE ?= release

ifeq ($(MODE), debug)
    BUILD_DIR := $(BUILD_ROOT)/debug
    CC_FLAGS  := $(BASE_CC_FLAGS) -O0 -ggdb3
    QEMU_FLAGS := -s -S
    MSG       := "DEBUG MODE"
else
    BUILD_DIR := $(BUILD_ROOT)/release
    CC_FLAGS  := $(BASE_CC_FLAGS) -O2
    QEMU_FLAGS := 
    MSG       := "RELEASE MODE"
endif

# --- Files Discovery ---
C_SRCS     := $(notdir $(wildcard $(SRC_DIR)/*.c))
ASM_SRCS   := $(notdir $(wildcard $(SRC_DIR)/*.asm))
OBJS       := $(addprefix $(BUILD_DIR)/, $(C_SRCS:.c=.o))
OBJS       += $(addprefix $(BUILD_DIR)/, $(ASM_SRCS:.asm=.o))

# --- ISO Paths ---
ISO_SUBDIR := $(BUILD_DIR)/iso_root
ISO_NAME   := CandyCane.iso
FINAL_ISO  := $(BUILD_DIR)/$(ISO_NAME)

# --- Build Rules ---

.PHONY: all clean run iso debug

all: 
	@echo "[*] Building in $(MSG)"
	@$(MAKE) $(FINAL_ISO) MODE=$(MODE)

# Link the kernel
$(BUILD_DIR)/kernel.bin: $(OBJS)
	@echo "[*] Linking kernel ($@)"
	@$(LD) $(LD_FLAGS) -o $@ $(OBJS)

# Rule for C files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@echo "    CC  $<"
	@$(CC) $(CC_FLAGS) -c $< -o $@

# Rule for ASM files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(BUILD_DIR)
	@echo "    ASM $<"
	@$(ASM) -f elf32 -g $< -o $@

# Create ISO
$(FINAL_ISO): $(BUILD_DIR)/kernel.bin
	@mkdir -p $(ISO_SUBDIR)/boot/grub
	@cp $(BUILD_DIR)/kernel.bin $(ISO_SUBDIR)/boot/
	@echo 'set timeout=0' > $(ISO_SUBDIR)/boot/grub/grub.cfg
	@echo 'set default=0' >> $(ISO_SUBDIR)/boot/grub/grub.cfg
	@echo 'menuentry "$(OS_NAME)" {' >> $(ISO_SUBDIR)/boot/grub/grub.cfg
	@echo '    multiboot /boot/kernel.bin' >> $(ISO_SUBDIR)/boot/grub/grub.cfg
	@echo '    boot' >> $(ISO_SUBDIR)/boot/grub/grub.cfg
	@echo '}' >> $(ISO_SUBDIR)/boot/grub/grub.cfg
	@grub-mkrescue -o $@ $(ISO_SUBDIR) 2>/dev/null

# Standard Run
run: all
	@echo "[*] Running QEMU ($(MODE))"
	@qemu-system-i386 -cdrom $(FINAL_ISO) $(QEMU_FLAGS)

# Integrated Debug Target
debug:
	@$(MAKE) MODE=debug all
	@echo "[*] Launching QEMU and GDB..."
	@qemu-system-i386 -cdrom $(BUILD_ROOT)/debug/$(ISO_NAME) -s -S & \
	sleep 1 && \
	gdb -x $(GDB_INIT)
clean:
	@echo "[*] Cleaning all builds"
	@rm -rf $(BUILD_ROOT)