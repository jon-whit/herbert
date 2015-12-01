####################
####################
##                ##
##      HELP      ##
##                ##
####################
####################

.PHONY : help
help :
	@echo ""
	@echo "Make Options:"
	@echo "------------"
	@echo ""
	@echo "make all                                     - Build '$(BOOTLOADER_ELF_FILE)' and '$(APP_ELF_FILE)'"
	@echo "make clean                                   - Cleans all generated files"
	@echo "make bootloader                              - Build '$(BOOTLOADER_ELF_FILE)'"
	@echo "make bootloader_clean                        - Cleans bootloader files"
	@echo "make app                                     - Build '$(APP_ELF_FILE)'"
	@echo "make app BUILD_NOTES=\"NOTES HERE\"            - Build '$(APP_ELF_FILE)' (adds notes to version string)"
	@echo "make app_clean                               - Cleans app files"
	@echo ""




##########################
##########################
##                      ##
##      BOOTLOADER      ##
##                      ##
##########################
##########################

##############
# Setup Info
##############

BOOTLOADER_OBJ_DIR := bootloader/OBJ_BOOTLOADER_DEBUG
BOOTLOADER_ELF_FILE := bootloader.elf
BOOTLOADER_MAP_FILE := bootloader.map
BOOTLOADER_LINKER_SCRIPT := bootloader.ld
BOOTLOADER_LINKER_SCRIPT_FLAGS := -Wl,-T -Wl,$(BOOTLOADER_LINKER_SCRIPT)
BOOTLOADER_LD := mb-ld
BOOTLOADER_CC := mb-gcc
BOOTLOADER_CC_SIZE := mb-size
BOOTLOADER_CC_OPT := -Os
BOOTLOADER_CFLAGS := -Wall
# Xilinx 10.1 EDK HACK!  In order not to have reent.h obliterate the
# definition of assert we need to #define DEBUG :-(
# BOOTLOADER_CC_DEBUG_FLAG := -g -DDEBUG
BOOTLOADER_CC_INFERRED_FLAGS :=     \
    -mno-xl-soft-mul                \
    -mhard-float                    \
    -mxl-float-convert              \
    -mxl-float-sqrt                 \
    -mxl-barrel-shift               \
    -mxl-pattern-compare            \
    -mno-xl-soft-div                \
    -mcpu=v7.10.d

BOOTLOADER_OTHER_CC_FLAGS :=                \
    $(BOOTLOADER_CC_DEBUG_FLAG)             \
    $(BOOTLOADER_CC_INFERRED_FLAGS)         \
    $(BOOTLOADER_LINKER_SCRIPT_FLAGS)


#######################################
# Source Files, Include/Library Paths
#######################################

BOOTLOADER_SOURCES :=           \
    bootloader/main.c           \
    bootloader/comm.c           \
    bootloader/serial.c         \
    bootloader/stubs.c          \
    bootloader/timer.c          \
    bootloader/vectors.c        \
    common/assert.c             \
    common/sprintf.c            \
    common/printf.c             \
    common/AT45DB161D.c         \
    common/conversion.c         \
    common/crc32.c              \
    common/errors.c             \
    common/flash.c              \
    common/interrupt.c          \
    common/processor.c          \
    common/reboot.c             \
    common/spi.c                \
    common/update.c

BOOTLOADER_INCLUDES :=                          \
    ../third_party/microblaze_0/include/        \
    bootloader/                                 \
    common/

BOOTLOADER_DEFINES :=       \
    -DBOOTLOADER

BOOTLOADER_LIBRARY_PATHS :=                 \
    ../third_party/microblaze_0/lib/


########################
# Internal Scaffolding
########################

# Determine the list of source directories.  The patsubst function
# removes the '/filename' part of each source file, and the sort
# function removes any duplicates in the list.
BOOTLOADER_SOURCE_DIRS := $(sort $(patsubst %/,%,$(dir $(BOOTLOADER_SOURCES))))

# Add -I to the beginning of the Includes
BOOTLOADER_INCLUDES := $(BOOTLOADER_INCLUDES:%=-I%)

# Add -L to the beginning of the Library Paths
BOOTLOADER_LIBRARY_PATHS := $(BOOTLOADER_LIBRARY_PATHS:%=-L%)

# Generate a list of .o and .d files from the sources
BOOTLOADER_DEPS := $(BOOTLOADER_SOURCES:%.c=$(BOOTLOADER_OBJ_DIR)/%.d)
BOOTLOADER_OBJS := $(BOOTLOADER_SOURCES:%.c=$(BOOTLOADER_OBJ_DIR)/%.o)


# Define a function that takes a source directory as the input
# and generates '.o' targets for each of the corresponding source
# files in the passed in directory.  These targets will also
# generate the '.d' dependency files.
# 
# $(1) - source directory
define generate_bootloader_build_rule

$(BOOTLOADER_OBJ_DIR)/$(1)/%.o : $(1)/%.c
	@mkdir -p $(BOOTLOADER_OBJ_DIR)/$(1)
	@echo "Compiling '$$<'"
	@$(BOOTLOADER_CC) $(BOOTLOADER_CFLAGS) $(BOOTLOADER_CC_OPT) $(BOOTLOADER_OTHER_CC_FLAGS) $(BOOTLOADER_DEFINES) $(BOOTLOADER_INCLUDES) -MMD -MP -c $$< -o $$@

endef


# Call 'generate_app_build_rule' for each source directory
$(foreach source_dir,$(BOOTLOADER_SOURCE_DIRS),$(eval $(call generate_bootloader_build_rule,$(source_dir))))

# Ensure the dependency targets are used
-include $(BOOTLOADER_DEPS)

.PHONY : bootloader_clean
bootloader_clean :
	@rm -rf $(BOOTLOADER_ELF_FILE)
	@rm -rf $(BOOTLOADER_MAP_FILE)
	@rm -rf $(BOOTLOADER_OBJ_DIR)

$(BOOTLOADER_ELF_FILE) : $(BOOTLOADER_OBJS) $(BOOTLOADER_LINKER_SCRIPT)
	@echo ""
	@echo "Linking '$@'"
	@echo ""
	@$(BOOTLOADER_CC) $(BOOTLOADER_CFLAGS) $(BOOTLOADER_CC_OPT) $(BOOTLOADER_OTHER_CC_FLAGS) $(BOOTLOADER_OBJS) -o $@ $(BOOTLOADER_LIBRARY_PATHS)
	@$(BOOTLOADER_CC_SIZE) $@
	@echo ""

.PHONY : bootloader
bootloader : $(BOOTLOADER_ELF_FILE) $(BOOTLOADER_MAP_FILE)

$(BOOTLOADER_MAP_FILE) : $(BOOTLOADER_ELF_FILE)
	@echo "Generating Map File '$@'"
	@$(BOOTLOADER_LD) $(BOOTLOADER_ELF_FILE) -o $@
	@echo ""




###################
###################
##               ##
##      APP      ##
##               ##
###################
###################

##############
# Setup Info
##############

APP_OBJ_DIR := app/OBJ_APP_DEBUG
APP_ELF_FILE := app.elf
APP_VERSION_INPUT_FILE := version.mk
APP_VERSION_OUTPUT_FILE := $(APP_ELF_FILE).ver
APP_MAP_FILE := app.map
APP_LINKER_SCRIPT := app.ld
APP_LINKER_SCRIPT_FLAGS := -Wl,-T -Wl,$(APP_LINKER_SCRIPT)
APP_LD := mb-ld
APP_CC := mb-gcc
APP_CC_SIZE := mb-size
APP_CC_OPT := -O0
APP_CFLAGS := -Wall
# Xilinx 10.1 EDK HACK!  In order not to have reent.h obliterate the
# definition of assert we need to #define DEBUG :-(
APP_CC_DEBUG_FLAG := -g -DDEBUG
APP_CC_INFERRED_FLAGS :=        \
    -mno-xl-soft-mul            \
    -mhard-float                \
    -mxl-float-convert          \
    -mxl-float-sqrt             \
    -mxl-barrel-shift           \
    -mxl-pattern-compare        \
    -mno-xl-soft-div            \
    -mcpu=v7.10.d

APP_OTHER_CC_FLAGS :=               \
    $(APP_CC_DEBUG_FLAG)            \
    $(APP_CC_INFERRED_FLAGS)        \
    $(APP_LINKER_SCRIPT_FLAGS)


#######################################
# Source Files, Include/Library Paths
#######################################

APP_SOURCES :=                  \
    app/cache.c                 \
    app/comm.c                  \
    app/initialization.c        \
    app/main.c                  \
    app/motion.c                \
    app/os.c                    \
    app/relay.c                 \
    app/serial.c                \
    app/signal.c                \
    app/startup.c               \
    app/stepper_hw.c            \
    app/steppers.c              \
    app/switch.c                \
    app/timer.c                 \
    common/assert.c             \
    common/AT45DB161D.c         \
    common/conversion.c         \
    common/crc16.c              \
    common/crc32.c              \
    common/errors.c             \
    common/flash.c              \
    common/interrupt.c          \
    common/lcd.c                \
    common/printf.c             \
    common/processor.c          \
    common/reboot.c             \
    common/spi.c                \
    common/sprintf.c            \
    common/update.c

APP_INCLUDES :=                                 \
    ../third_party/microblaze_0/include/        \
    app/                                        \
    common/

APP_DEFINES :=      \

APP_LIBRARY_PATHS :=                        \
    ../third_party/microblaze_0/lib/


########################
# Internal Scaffolding
########################

ifeq ($(OFFICIAL_BUILD),1)
    APP_VERSION := $(shell cat $(APP_VERSION_INPUT_FILE))
else
    APP_VERSION := X.X.ENG_$(shell whoami):$(shell hostname)_$(shell date +'%D_%R')
endif

ifneq ($(BUILD_NOTES),)
    APP_VERSION := $(APP_VERSION) - $(BUILD_NOTES)
endif

# Add the version define to the list
APP_DEFINES :=                          \
    $(APP_DEFINES)                      \
#    -DFW_VER_APP=\""$(APP_VERSION)"\"

# Determine the list of source directories.  The patsubst function
# removes the '/filename' part of each source file, and the sort
# function removes any duplicates in the list.
APP_SOURCE_DIRS := $(sort $(patsubst %/,%,$(dir $(APP_SOURCES))))

# Add -I to the beginning of the Includes
APP_INCLUDES := $(APP_INCLUDES:%=-I%)

# Add -L to the beginning of the Library Paths
APP_LIBRARY_PATHS := $(APP_LIBRARY_PATHS:%=-L%)

# Generate a list of .o and .d files from the sources
APP_DEPS := $(APP_SOURCES:%.c=$(APP_OBJ_DIR)/%.d)
APP_OBJS := $(APP_SOURCES:%.c=$(APP_OBJ_DIR)/%.o)


# Define a function that takes a source directory as the input
# and generates '.o' targets for each of the corresponding source
# files in the passed in directory.  These targets will also
# generate the '.d' dependency files.
# 
# $(1) - source directory
define generate_app_build_rule

$(APP_OBJ_DIR)/$(1)/%.o : $(1)/%.c
	@mkdir -p $(APP_OBJ_DIR)/$(1)
	@echo "Compiling '$$<'"
	@$(APP_CC) $(APP_CFLAGS) $(APP_CC_OPT) $(APP_OTHER_CC_FLAGS) $(APP_DEFINES) $(APP_INCLUDES) -MMD -MP -c $$< -o $$@

endef


# Call 'generate_app_build_rule' for each source directory
$(foreach source_dir,$(APP_SOURCE_DIRS),$(eval $(call generate_app_build_rule,$(source_dir))))

# Ensure the dependency targets are used
-include $(APP_DEPS)

.PHONY : app_clean
app_clean :
	@rm -rf $(APP_ELF_FILE)
	@rm -rf $(APP_MAP_FILE)
	@rm -rf $(APP_OBJ_DIR)
	@rm -rf $(APP_VERSION_OUTPUT_FILE)

$(APP_ELF_FILE) : $(APP_OBJS) $(APP_LINKER_SCRIPT)
	@echo ""
	@echo "Linking '$@'"
	@echo ""
	@$(APP_CC) $(APP_CFLAGS) $(APP_CC_OPT) $(APP_OTHER_CC_FLAGS) $(APP_OBJS) -o $@ $(APP_LIBRARY_PATHS)
	@$(APP_CC_SIZE) $@
	@echo $(APP_VERSION) > $(APP_VERSION_OUTPUT_FILE)
	@echo ""

.PHONY : app
app : $(APP_ELF_FILE) $(APP_MAP_FILE)

$(APP_MAP_FILE) : $(APP_ELF_FILE)
	@echo "Generating Map File '$@'"
	@$(APP_LD) $(APP_ELF_FILE) -o $@
	@echo ""




#################
# Other Targets
#################

.PHONY : all
all : bootloader app

.PHONY : clean
clean : bootloader_clean app_clean
