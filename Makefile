#
# Smart City Digital Twin
# QNX 8.0 - Raspberry Pi 4
#

# Build artifact
ARTIFACT = smart_city_digital_twin

# Build architecture
# Possible values: x86, armv7le, aarch64le, etc.
PLATFORM ?= aarch64le

# Build profile
# Possible values: release, debug, profile, coverage
BUILD_PROFILE ?= debug

CONFIG_NAME ?= $(PLATFORM)-$(BUILD_PROFILE)

OUTPUT_DIR = build/$(CONFIG_NAME)

TARGET = $(OUTPUT_DIR)/$(ARTIFACT)


#
# Compiler definitions
#

CC = qcc -Vgcc_nto$(PLATFORM)

CXX = q++ -Vgcc_nto$(PLATFORM)_cxx

LD = $(CC)


#
# User defined include/preprocessor flags and libraries
#

INCLUDES += -Iinclude

# Additional libraries can be added here if required.
# LIBS += -L/path/to/my/lib/$(PLATFORM)/usr/lib -lmylib
# LIBS += -L../mylib/$(OUTPUT_DIR) -lmylib


#
# Compiler flags for build profiles
#

CCFLAGS_release += -O2

CCFLAGS_debug += -g -O0 -fno-builtin

CCFLAGS_coverage += -g -O0 -ftest-coverage -fprofile-arcs

LDFLAGS_coverage += -ftest-coverage -fprofile-arcs

CCFLAGS_profile += -g -O0 -finstrument-functions

LIBS_profile += -lprofilingS


#
# Generic compiler flags
#

CCFLAGS_all += -Wall -fmessage-length=0

CCFLAGS_all += $(CCFLAGS_$(BUILD_PROFILE))

LDFLAGS_all += $(LDFLAGS_$(BUILD_PROFILE))

LIBS_all += $(LIBS_$(BUILD_PROFILE))


#
# Dependency generation
#

DEPS = -Wp,-MMD,$(@:%.o=%.d),-MT,$@


#
# Macro to recursively find source files
#

rwildcard = $(wildcard $1/*.$2) \
            $(foreach d,$(wildcard $1/*),$(call rwildcard,$d,$2))


#
# Source list
#

SRCS = $(call rwildcard,src,c)


#
# Object files list
#

OBJS = $(addprefix $(OUTPUT_DIR)/,$(addsuffix .o,$(basename $(SRCS))))


#
# Compilation rule
#

$(OUTPUT_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) -c $(DEPS) -o $@ $(INCLUDES) $(CCFLAGS_all) $(CCFLAGS) $<


#
# Linking rule
#

$(TARGET): $(OBJS)
	$(LD) -o $(TARGET) $(LDFLAGS_all) $(LDFLAGS) $(OBJS) $(LIBS_all) $(LIBS)


#
# Default compilation and linking
#

all: $(TARGET)


#
# Clean
#

clean:
	rm -fr $(OUTPUT_DIR)


#
# Rebuild
#

rebuild: clean all


#
# Include generated dependencies
#

-include $(OBJS:%.o=%.d)