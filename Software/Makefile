##################################
##  Microcontroller Properties  ##
##################################

MCU = arm7tdmi
CHIP = at91sam7xc256
BOARD = mariokartv1
MEMORY ?= flash

# Trace level used for compilation
# (can be overriden by adding TRACE_LEVEL=#number to the command-line)
# TRACE_LEVEL_DEBUG      5
# TRACE_LEVEL_INFO       4
# TRACE_LEVEL_WARNING    3
# TRACE_LEVEL_ERROR      2
# TRACE_LEVEL_FATAL      1
# TRACE_LEVEL_NO_TRACE   0
TRACE_LEVEL ?= 4


########################
##  Toolchain Config  ##
########################

PREFIX = arm-none-eabi-
TOOL_DIR = tools/

CC      = $(PREFIX)gcc
AR      = $(PREFIX)ar
GDB     = $(PREFIX)gdb
RANLIB  = $(PREFIX)ranlib
OBJCOPY = $(PREFIX)objcopy

OPENOCD = openocd

MAKEMAKE = $(TOOL_DIR)makemake.py
GRAPHDEPS = $(TOOL_DIR)graphdeps.py

OUTPUT_FOLDER  = build
OBJECTS_FOLDER = $(OUTPUT_FOLDER)/objs

OCD_CHECK_FILE = /tmp/mariokart-ocd.running

VPATH += $(OUTPUT_FOLDER)
VPATH += $(OBJECTS_FOLDER)

# Define any computer specific PREFIX etc. in here
-include local.mk

GRAPH_OUTPUT=../Documentation/graphs/

#############
##  Flags  ##
#############

LIBS =  lib/boards/$(BOARD)
LIBS += lib/peripherals
LIBS += lib/components
LIBS += lib/drivers
LIBS += lib

LIBS += lib/usb/common/cdc
LIBS += lib/usb/common/core
LIBS += lib/usb/device/core
LIBS += lib/usb/device/cdc-serial

INCLUDES = $(addprefix -I,$(LIBS))

CFLAGS += -Wall -std=gnu99 -pedantic
CFLAGS += -mcpu=$(MCU)
#CFLAGS += -Os
CFLAGS += -g
CFLAGS += -D$(CHIP)
CFLAGS += -D$(MEMORY)
CFLAGS += -DTRACE_LEVEL=$(TRACE_LEVEL)
CFLAGS += -mthumb-interwork
CFLAGS += $(INCLUDES)

# Generate dependency information
CFLAGS += -MD -MP -MF .dep/$(subst /,-,$@).d

#ASFLAGS += -Os
ASFLAGS += -g
ASFLAGS += -D$(CHIP)
ASFLAGS += -D$(MEMORY)
ASFLAGS += -D__ASSEMBLY__
ASFLAGS += -mthumb-interwork
ASFLAGS += $(INCLUDES)

LINK_SCRIPT = -T"lib/boards/$(BOARD)/$(CHIP)/$(MEMORY).lds"

LDFLAGS += -nostartfiles -Wl,--gc-sections
LDFLAGS += -Lbuild
LDFLAGS += -lmario

C_FILES =  $(wildcard lib/utility/*.c)
C_FILES += $(wildcard lib/drivers/*/*.c)
C_FILES += $(wildcard lib/components/*.c)
C_FILES += $(wildcard lib/peripherals/*/*.c)

C_FILES += $(wildcard lib/usb/common/cdc/*.c)
C_FILES += $(wildcard lib/usb/common/core/*.c)
C_FILES += $(wildcard lib/usb/device/cdc-serial/*.c)
C_FILES += $(wildcard lib/usb/device/core/*.c)

C_FILES += lib/boards/$(BOARD)/board_memories.c
C_FILES += lib/boards/$(BOARD)/board_lowlevel.c
C_FILES += lib/boards/$(BOARD)/board_powermode.c

#steering files (I dont know much about makefiles so this probably needs changing)
C_FILES += $(wildcard src/steering/*.c)
#brake files will tidy this up soon
C_FILES += $(wildcard src/brake/*.c)

ASM_FILES =  lib/boards/$(BOARD)/board_cstartup.S
ASM_FILES += $(wildcard lib/utility/*.S)

OBJS =  $(addprefix $(OBJECTS_FOLDER)/,$(C_FILES:.c=.o))
OBJS += $(addprefix $(OBJECTS_FOLDER)/,$(ASM_FILES:.S=.o))


###############
##  Targets  ##
###############

all: libs hexes

test:
	@CFLAGS=-Werror TRACE_LEVEL=5 $(MAKE) -ks rebuild clean


LIB = $(OUTPUT_FOLDER)/libmario.a
libs: $(LIB)

splint:
	splint $(INCLUDES) src/*/main.c

hexes: $(patsubst src/%/main.c,$(OUTPUT_FOLDER)/%.bin,$(wildcard src/*/main.c))


rebuild: clean all


%.bin: %.elf
	$(OBJCOPY) -O binary $< $@


$(OUTPUT_FOLDER)/%.elf: $(OBJECTS_FOLDER)/src/%/main.o $(LIB)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LINK_SCRIPT) -o $@ $< $(LDFLAGS)


$(OBJECTS_FOLDER)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<


$(OBJECTS_FOLDER)/%.o: %.S
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -o $@ -c $<


$(OUTPUT_FOLDER)/libmario.a: $(OBJS)
	$(AR) $(ARFLAGS) $@ $? 2>/dev/null >/dev/null
	$(RANLIB) $@


# Temporarily creates a file to allow the debug task to
# check that OpenOCD has been started before starting GDB.
ocd: openocd.cfg
	@touch $(OCD_CHECK_FILE);\
		$(OPENOCD) -f $< ;\
		rm $(OCD_CHECK_FILE)
ocd%: openocd%.cfg
	@touch $(OCD_CHECK_FILE)$*;\
		$(OPENOCD) -f $<;\
		rm $(OCD_CHECK_FILE)$*


program-%: $(OUTPUT_FOLDER)/%.elf
	@if [ ! -e $(OCD_CHECK_FILE) ] ; \
		then ( \
			echo "\n"\
			  "==============================" "\n"\
				" OpenOCD not running          " "\n"\
				"                              " "\n"\
				" Use \`make ocd\` to start it." "\n"\
				"==============================" "\n"\
		) else ( \
			$(GDB) -batch -x program.gdb $< \
		) \
	fi


# Uses the check file to ensure that OpenOCD has
# been started before attempting to start up GDB.
debug-%: $(OUTPUT_FOLDER)/%.elf
	@if [ ! -e $(OCD_CHECK_FILE) ] ; \
		then ( \
			echo "\n"\
			  "==============================" "\n"\
				" OpenOCD not running          " "\n"\
				"                              " "\n"\
				" Use \`make ocd\` to start it." "\n"\
				"==============================" "\n"\
		) else ( \
			$(GDB) -x debug.gdb $< \
		) \
	fi


enable-flash-boot:
	@if [ ! -e $(OCD_CHECK_FILE) ] ; \
		then ( \
			echo "\n"\
			  "==============================" "\n"\
				" OpenOCD not running          " "\n"\
				"                              " "\n"\
				" Use \`make ocd\` to start it." "\n"\
				"==============================" "\n"\
		) else ( \
			$(GDB) -batch -x enable_flash_boot.gdb \
		) \
	fi


disable-flash-boot:
	@if [ ! -e $(OCD_CHECK_FILE) ] ; \
		then ( \
			echo "\n"\
			  "==============================" "\n"\
				" OpenOCD not running          " "\n"\
				"                              " "\n"\
				" Use \`make ocd\` to start it." "\n"\
				"==============================" "\n"\
		) else ( \
			$(GDB) -batch -x disable_flash_boot.gdb \
		) \
	fi


clean:
	rm -rf $(OUTPUT_FOLDER)/*
	rm -rf .dep/*

graphs-%: 
	mkdir -p .graphs $(GRAPH_OUTPUT)
	$(MAKEMAKE) --relpath --files src/$*/main.c $(LIBS) > .graphs/$*-files.d
	$(GRAPHDEPS) .graphs/$*-files.d --out $(GRAPH_OUTPUT)$*-file_dependencies.pdf
	$(MAKEMAKE) --relpath --modules src/$*/main.c $(LIBS) > .graphs/$*-modules.d
	$(GRAPHDEPS) .graphs/$*-modules.d --out $(GRAPH_OUTPUT)$*-modules_dependencies.pdf

graphs-makefile:
	mkdir -p .graphs $(GRAPH_OUTPUT)
	$(GRAPHDEPS) ./Makefile --out $(GRAPH_OUTPUT)makefile_dependencies.pdf
	$(GRAPHDEPS) ./Makefile --out $(GRAPH_OUTPUT)makefile_dependencies.pdf --showops

.PRECIOUS: $(OBJECTS_FOLDER)/%.o %.hex $(OUTPUT_FOLDER)/%.elf
.PHONY: all rebuild clean graphs

# Include the dependency files, should be the last of the makefile
-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)
