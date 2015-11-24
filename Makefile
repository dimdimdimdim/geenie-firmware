# Copyright 2015 Dimitri L. <dimdimdimdim at gmx dot fr>
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License along
# with this program; if not, see <http://www.gnu.org/licenses/>.

CONF = test

TARGET_LANG ?= EN
DESTNAME ?= geenie

ifeq ($(OS),Windows_NT)
  CCPATH = C:/Program Files (x86)/GNU Tools ARM Embedded/4.8 2014q1/bin
  UTIL = C:/MinGW/msys/1.0/bin/$(1)
  ECHO = C:/MinGW/msys/1.0/bin/echo
  QEMU = "C:\Program Files (x86)\Qemu-windows-2.4.0.1\qemu-system-arm.exe"
  #QEMU = "D:\Elec\CodeSourcery\Qemu-windows-0.15.1\qemu-system-arm.exe" -serial mon:tcp:127.0.0.1:4444,server,nowait
  PERL = perl
  # this is for qemu under windows
  export SDL_STDIO_REDIRECT = no
else
  #CCPATH = /usr/local/gcc-arm-none-eabi-4_8-2014q1/bin
  CCPATH = /usr/local/gcc-arm-none-eabi-4_9-2015q3/bin
  UTIL = $(1)
  ECHO = echo
  #QEMU = /usr/src/qemu/build/debug/arm-softmmu/qemu-system-arm -serial mon:stdio
  #QEMU = ~/qemu/2.4.50-201511152059-dev/bin/qemu-system-gnuarmeclipse
  QEMU = ~/Work/qemu/install/debian64/qemu/bin/qemu-system-gnuarmeclipse
  PERL = perl
endif

ifeq ($(shell $(call UTIL,grep) DEF_LANG_$(TARGET_LANG) res/messages.cpp),)
  $(error Language $(TARGET_LANG) not defined)
endif

CPPFLAGS = -Isrc/system/kernel/archs/arm/include -Isrc/system/user/include -Isrc/system/env/include -DLANG_$(TARGET_LANG) -DKERNEL_CONF_NAME=$(CONF) -DCPU_NAME=$(TARGET_CPU) -DMCU_NAME=$(TARGET_MCU) -DBOARD_NAME=$(TARGET_BOARD)
ASFLAGS = -Wall -mcpu=$(TARGET_CPU) -fdata-sections -ffunction-sections
CFLAGS = -Wall -mcpu=$(TARGET_CPU) -mthumb -ffreestanding -fdata-sections -ffunction-sections $(CPPFLAGS)
CXXFLAGS = $(CFLAGS) -Wno-invalid-offsetof -std=c++11 -fno-rtti
LDFLAGS = -mcpu=$(TARGET_CPU) -nostdlib -Wl,--gc-sections
QEMU_CPU = $(TARGET_CPU)

ARCHDIR = src/system/kernel/archs

find = $(foreach d, $(wildcard $1/*), $(call find, $d, $2) $(filter $2, $d))
SRC := $(filter-out $(ARCHDIR)/%, $(call find, src, %.c %.cpp %.S)) $(call find, res, %.c %.cpp)

include mk/config.$(CONF).mk
include mk/board.$(TARGET_BOARD).mk
include mk/mcu.$(TARGET_MCU).mk
include mk/cpu.$(TARGET_CPU).mk

ifeq ($(ENABLE_DEBUG),1)
CFLAGS += -ggdb
CPPFLAGS += -DDEBUG
BUILD_MODE = debug
else
CFLAGS += -Os
BUILD_MODE = release
endif
DEPS_FILES = $(patsubst %,output/arm/$(CONF)/deps/%.d,$(SRC)) output/arm/$(CONF)/deps/system.ld.d
OBJECT_FILES = $(patsubst %,output/arm/$(CONF)/%.o,$(basename $(SRC)))

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),clean-objs)
ifeq ($(MAKE_RESTARTS),)
  $(info configuration : $(CONF))
  $(info board         : $(TARGET_BOARD))
  $(info mcu           : $(TARGET_MCU) ($(TARGET_CPU)))
  $(info mode          : $(BUILD_MODE))
endif
endif
endif

# number of lines to print in the "blame" rule (for overriding in the command line)
n=20

bin: output/arm/$(CONF)/$(DESTNAME).bin
	@"$(CCPATH)/arm-none-eabi-size" output/arm/$(CONF)/$(DESTNAME).elf
	@$(ECHO) "Done."

elf: output/arm/$(CONF)/$(DESTNAME).elf
	@"$(CCPATH)/arm-none-eabi-size" output/arm/$(CONF)/$(DESTNAME).elf
	@$(ECHO) "Done."

blame: output/arm/$(CONF)/$(DESTNAME).elf
	@$(ECHO) Biggest objects in text section:
	@"$(CCPATH)/arm-none-eabi-nm" -C -td --reverse-sort --size-sort output/arm/$(CONF)/$(DESTNAME).elf | $(call UTIL,grep) " [tT] " | $(call UTIL,head) -n$(n) | $(call UTIL,sed) -e 's/^/\t/'
	@$(ECHO) Biggest objects in bss section:
	@"$(CCPATH)/arm-none-eabi-nm" -C -td --reverse-sort --size-sort output/arm/$(CONF)/$(DESTNAME).elf | $(call UTIL,grep) " [bB] " | $(call UTIL,head) -n$(n) | $(call UTIL,sed) -e 's/^/\t/'
	@$(ECHO) Biggest objects in data section:
	@"$(CCPATH)/arm-none-eabi-nm" -C -td --reverse-sort --size-sort output/arm/$(CONF)/$(DESTNAME).elf | $(call UTIL,grep) " [dD] " | $(call UTIL,head) -n$(n) | $(call UTIL,sed) -e 's/^/\t/'
	
nodeps:
	@$(ECHO) "Dependencies checking bypassed"
	@$(call UTIL,touch) -c $(DEPS_FILES)

ifeq ($(patsubst qemu-%,qemu-,$(TARGET_BOARD)),qemu-)
	
test: elf
	@$(ECHO) "Starting qemu..."
	@$(QEMU) -mcu $(TARGET_MCU) -nographic -semihosting -monitor telnet:127.0.0.1:51235,server,nowait -kernel output/arm/$(CONF)/$(DESTNAME).elf

test-gdb: elf
	@(sleep .5; gnome-terminal -x "$(CCPATH)/arm-none-eabi-gdb" -ex "target remote 127.0.0.1:51234" output/arm/$(CONF)/$(DESTNAME).elf)&
	@$(ECHO) "Starting qemu..."
	@$(QEMU) -mcu $(TARGET_MCU) -nographic -semihosting -gdb tcp:127.0.0.1:51234 -monitor telnet:127.0.0.1:51235,server,nowait -S -kernel output/arm/$(CONF)/$(DESTNAME).elf

endif
	
clean:
	@$(call UTIL,rm) -rf output/arm/$(CONF)/deps
	@$(call UTIL,rm) -rf output/arm/$(CONF)

clean-objs:
	@$(call UTIL,rm) -rf output/arm/$(CONF)

ifneq ($(firstword $(MAKECMDGOALS)),nodeps)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),clean-objs)
-include $(DEPS_FILES)
endif
endif
endif

#	@$(PERL) makedeps.pl $@ "output/arm/$(CONF)/$*.o" "$(CCPATH)/arm-none-eabi-gcc" $(CFLAGS) -DGENERATE_DEPS -MM $<
output/arm/$(CONF)/deps/%.c.d: %.c
	@$(ECHO) "Checking deps for $<"
	@$(call UTIL,mkdir) -p $(dir $@)
	@"$(CCPATH)/arm-none-eabi-gcc" $(CFLAGS) -DGENERATE_DEPS -MM -MP -MT output/arm/$(CONF)/$*.o -MT $@ $< > $@

output/arm/$(CONF)/deps/%.cpp.d: %.cpp
	@$(ECHO) "Checking deps for $<"
	@$(call UTIL,mkdir) -p $(dir $@)
	@"$(CCPATH)/arm-none-eabi-gcc" $(CFLAGS) -DGENERATE_DEPS -MM -MP -MT output/arm/$(CONF)/$*.o -MT $@ $< > $@

output/arm/$(CONF)/deps/%.S.d: %.S
	@$(ECHO) "Checking deps for $<"
	@$(call UTIL,mkdir) -p $(dir $@)
	@"$(CCPATH)/arm-none-eabi-gcc" $(CFLAGS) -DGENERATE_DEPS -MM -MP -MT output/arm/$(CONF)/$*.o -MT $@ $< > $@

output/arm/$(CONF)/deps/system.ld.d: $(LDFILE)
	@$(ECHO) "Checking deps for $<"
	@$(call UTIL,mkdir) -p $(dir $@)
	@"$(CCPATH)/arm-none-eabi-cpp" $(CFLAGS) -DGENERATE_DEPS -MM -MP -MT output/arm/$(CONF)/system.ld -MT $@ $< > $@
	
output/arm/$(CONF)/%.o: %.c
	@$(ECHO) "Compiling $<"
	@$(call UTIL,mkdir) -p $(dir $@)
	@"$(CCPATH)/arm-none-eabi-gcc" $(CFLAGS) -c $< -o $@

output/arm/$(CONF)/%.o: %.cpp
	@$(ECHO) "Compiling $<"
	@$(call UTIL,mkdir) -p $(dir $@)
	@"$(CCPATH)/arm-none-eabi-gcc" $(CXXFLAGS) -c $< -o $@	
	
output/arm/$(CONF)/%.o: %.S
	@$(ECHO) "Compiling $<"
	@$(call UTIL,mkdir) -p $(dir $@)
	@"$(CCPATH)/arm-none-eabi-gcc" $(ASFLAGS) -c $< -o $@	
	
output/arm/$(CONF)/system.ld: $(LDFILE)
	@$(ECHO) "Generating linker script"
	@$(call UTIL,mkdir) -p $(dir $@)
	@"$(CCPATH)/arm-none-eabi-cpp" $(CPPFLAGS) -C -P $(LDFILE) > $@

output/arm/$(CONF)/$(DESTNAME).elf: output/arm/$(CONF)/system.ld $(OBJECT_FILES)
	@$(ECHO) "Linking $@"
	@"$(CCPATH)/arm-none-eabi-gcc" -Toutput/arm/$(CONF)/system.ld $(OBJECT_FILES) -lgcc -lsupc++ -lm $(LDFLAGS) -o $@

output/arm/$(CONF)/$(DESTNAME).bin: output/arm/$(CONF)/$(DESTNAME).elf
	@$(ECHO) "Generating firmware binary"
	@"$(CCPATH)/arm-none-eabi-objcopy" -O binary $^ $@

.PHONY: bin elf blame nodeps test test-gdb clean clean-objs
