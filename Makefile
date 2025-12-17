# --- Project Configuration ---
OS_NAME    := Candycane OS
SRC_DIR    := src
INC_DIR    := include
BUILD_DIR  := build
ISO_SUBDIR := $(BUILD_DIR)/iso
ISO_NAME   := CandyCane.iso
LD_FILE    := linker.ld

# --- Toolchain ---
CC         := gcc
ASM        := nasm
LD         := ld

# --- Compilation Flags ---
CC_FLAGS   := -std=gnu2x -m32 -ffreestanding -fno-stack-protector \
              -fno-pie -fno-pic -nostdlib -nostartfiles -O2 -I$(INC_DIR) -Wall -Wextra
LD_FLAGS   := -m elf_i386 -T $(LD_FILE) -z noexecstack

# --- Files Discovery ---
C_SRCS     := $(notdir $(wildcard $(SRC_DIR)/*.c))
ASM_SRCS   := $(notdir $(wildcard $(SRC_DIR)/*.asm))

# Create the object list: build/file.o
OBJS       := $(addprefix $(BUILD_DIR)/, $(C_SRCS:.c=.o))
OBJS       += $(addprefix $(BUILD_DIR)/, $(ASM_SRCS:.asm=.o))

# --- ISO Paths ---
FINAL_ISO  := $(ISO_SUBDIR)/$(ISO_NAME)
TEMP_ISO   := $(BUILD_DIR)/tmp_$(ISO_NAME)

# --- Build Rules ---

.PHONY: all clean run iso

all: $(BUILD_DIR)/kernel.bin iso

# Link the kernel
$(BUILD_DIR)/kernel.bin: $(OBJS)
	@echo "[*] Linking kernel"
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
	@$(ASM) -f elf32 $< -o $@

# Create ISO: Source is build/iso, Temp output in build/, Final move to build/iso/
iso: $(FINAL_ISO)

$(FINAL_ISO): $(BUILD_DIR)/kernel.bin
	@echo "[*] Preparing ISO source directory structure"
	@mkdir -p $(ISO_SUBDIR)/boot/grub
	@cp $(BUILD_DIR)/kernel.bin $(ISO_SUBDIR)/boot/
	@echo 'set timeout=0' > $(ISO_SUBDIR)/boot/grub/grub.cfg
	@echo 'set default=0' >> $(ISO_SUBDIR)/boot/grub/grub.cfg
	@echo 'menuentry "$(OS_NAME)" {' >> $(ISO_SUBDIR)/boot/grub/grub.cfg
	@echo '    multiboot /boot/kernel.bin' >> $(ISO_SUBDIR)/boot/grub/grub.cfg
	@echo '    boot' >> $(ISO_SUBDIR)/boot/grub/grub.cfg
	@echo '}' >> $(ISO_SUBDIR)/boot/grub/grub.cfg
	@echo "[*] Creating temporary ISO: $(TEMP_ISO)"
	@grub-mkrescue -o $(TEMP_ISO) $(ISO_SUBDIR)
	@echo "[*] Moving final ISO to: $@"
	@mv $(TEMP_ISO) $@

# Run in QEMU
run: iso
	@echo "[*] Running in QEMU"
	@qemu-system-i386 -cdrom $(FINAL_ISO)

# Cleanup
clean:
	@echo "[*] Cleaning build directory"
	@rm -rf $(BUILD_DIR)