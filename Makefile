# INCLUDES
# ----------------

include config/sources.mk
include config/tools.mk

# OPTIONS
# -----------------

DEBUG=TRUE

# VARIABLES
# -----------------

ifeq ($(DEBUG),TRUE)
	DEBUG_FLAGS = -g
else
	DEBUG_FLAGS = -O2
endif

COMP_FLAGS = STM32F429_439xx

CFLAGS  = $(DEBUG_FLAGS) -Wall -T$(LINKER_FILE)
CFLAGS += -mlittle-endian -mthumb -mcpu=cortex-m4 -mthumb-interwork
CFLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16 -specs=nosys.specs
CFLAGS += $(addprefix -I, $(INC_DIR))
CFLAGS += $(addprefix -D, $(COMP_FLAGS))

RM_F = rm -f
MKDIR_P = mkdir -p
CP = cp

# TARGETS
# -----------------

# Default to build
all: build

# Build elf file
.PHONY: build
build: $(BIN_DIR)/$(PROJ_NAME).elf

$(BIN_DIR)/$(PROJ_NAME).elf: $(addprefix $(BUILD_DIR)/, $(OBJS))
	@$ $(MKDIR_P) $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ 
	$(OBJCOPY) -O ihex $(BIN_DIR)/$(PROJ_NAME).elf $(BIN_DIR)/$(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(BIN_DIR)/$(PROJ_NAME).elf $(BIN_DIR)/$(PROJ_NAME).bin

# Output object files from source
.PHONY: compile
compile: $(addprefix $(BUILD_DIR)/, $(OBJS))

$(BUILD_DIR)/%.o: %.c
	@$ $(MKDIR_P) $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: %.s
	@$ $(MKDIR_P) $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Output assembly files from source
.PHONY: assemble
assemble: $(addprefix $(BUILD_DIR)/, $(ASMS))

$(BUILD_DIR)/%.s: %.c
	@$ $(MKDIR_P) $(BUILD_DIR)
	$(CC) $(CFLAGS) -S $< -o $@

$(BUILD_DIR)/%.s: %.s
	@$ $(MKDIR_P) $(BUILD_DIR)
	@$ $(CP) $< $(BUILD_DIR)

clean:
	@$ $(RM_F) $(BIN_DIR)/* $(BUILD_DIR)/*

# Flash the STM32F4
burn: build
	st-flash write $(BIN_DIR)/$(PROJ_NAME).bin 0x8000000
