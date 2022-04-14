MAKEFLAGS += --no-print-directory --no-builtin-rules --silent

# DIR_GUARD makes destination directory for each target if it doesn't already exist
DIR_GUARD=@mkdir -p $(@D)
%/:
	$(DIR_GUARD)

# Build tools
TOOLCHAIN := arm-none-eabi
AR := $(TOOLCHAIN)-ar
CPPC := $(TOOLCHAIN)-g++

# C standard
CPPSTD := c++2a

DEVICES_CPPFLAGS := \
	-fno-common -fmessage-length=0 -Wall -Wextra -fno-exceptions -ffunction-sections \
	-fdata-sections -fomit-frame-pointer -MMD -MP -Wno-unused -Wno-shift-negative-value -fshort-enums \
	-Wno-packed-bitfield-compat \
	-ffast-math -fno-math-errno -fsingle-precision-constant \
	-std=$(CPPSTD)

# Define the requirements that this submodule needs in order to compile
# These requirements should be satisified in the top level firmware MakeFile
ifneq ($(MAKECMDGOALS),clean)

SRC := $(wildcard src/*.cpp)
OBJ := $(patsubst src/%.cpp, build/obj/%.o, $(SRC))

INCLUDES := $(addprefix -I, inc)

.DEFAULT_GOAL := build/liblogging.a
build/liblogging.a: $(OBJ)
	$(DIR_GUARD)
	@$(AR) rcs $@ $(OBJ)
	@if tty -s; then tput setaf 2; echo "LOGGING complete"; tput sgr0; fi

build/obj/prereq: Makefile
	$(DIR_GUARD)
	@touch $@

build/obj/%.o: src/%.cpp build/obj/prereq
	$(DIR_GUARD)
	@$(CPPC) $(INCLUDES) $(CPPFLAGS) $(DEVICES_CPPFLAGS) -c $< -o $@

endif # MAKECMDGOALS != clean

.PHONY: clean
clean:
	@rm -rf build
