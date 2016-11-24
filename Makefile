# INCLUDES
# ----------------

include config/sources.mk
include config/tools.mk

# OPTIONS
# -----------------

# Debug mode
# Options are TRUE, FALSE
# Default is TRUE
DEBUG=TRUE

# Logger verbosity
# Options are INFO, WARN, ERR, NONE
# Default is ERR
LOG=ERR

# Command interface
# Options are TRUE, FALSE
# Default is TRUE
CMD=TRUE

# Camera module
# Options are ARDUCAM_OV5647
# Default is ARDUCAM_OV5647
CAMERA=ARDUCAM_OV5647

# Wifi Module
# Options are ESP8266
# Default is ESP8266
WIFI=ESP8266

# VARIABLES
# -----------------

COMP_FLAGS = STM32F429_439xx USE_STDPERIPH_DRIVER HSE_VALUE=8000000

ifeq ($(DEBUG),FALSE)
  DEBUG_FLAGS = -O2
else
  ifeq ($(DEBUG),TRUE) 
    DEBUG_FLAGS = -g


    ifneq ($(LOG), NONE) 
      ifeq ($(LOG),INFO)
        COMP_FLAGS += __LOG __INFO
      else
        ifeq ($(LOG),WARN)
          COMP_FLAGS += __LOG __WARN
        else
          ifeq ($(LOG), ERR)
            COMP_FLAGS += __LOG __ERR
          else 
            $(error Bad value for LOG)
          endif
        endif
      endif
    endif
  

    ifeq ($(CMD),TRUE)
      COMP_FLAGS += __CMD
    else
      ifneq ($(CMD),FALSE)
        $(error Bad value for CMD)
      endif
    endif

    ifeq ($(PROF),TRUE)
      COMP_FLAGS += __PROF
    else
      ifneq ($(PROF),FALSE)
        $(error, Bad value for PROF)
      endif
    endif

  else
    $(error Bad value for DEBUG)
  endif
endif

ifeq ($(CAMERA),ARDUCAM_OV5647)
  COMP_FLAGS += __ARDUCAM_OV5647
else
  $(error Bad value for CAMERA)
endif

ifeq ($(WIFI),ESP8266)
  COMP_FLAGS += __ESP8266
else
  $(error Bad value for WIFI)
endif

CFLAGS  = $(DEBUG_FLAGS) --std=c99 -Wall -T$(LINKER_FILE)
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
