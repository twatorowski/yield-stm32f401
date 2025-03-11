# --------------------------- TARGET NAME ---------------------------
# output name (final elf/bin will be named after this)
TARGET = yield-stm32f401xc

# -------------------------- LINKER SCRIPT --------------------------
# linker script
LD_SCRIPT = stm32f401xc.ld

# -------------------------- VERSION NUMBER -------------------------
# software version
SW_VER_MAJOR = 1
SW_VER_MINOR = 0
SW_VER_BUILD = 0

# hardware version
HW_VER_MAJOR = 1
HW_VER_MINOR = 0
HW_VER_BUILD = 0

# ----------------------------- SOURCES -----------------------------
# put all of your sources here (use / as path separator)
SRC += ./main.c ./vectors.c ./reset.c ./defhndl.c ./startup.c
SRC += ./debug.c ./coredump.c

# device drivers
SRC += ./dev/src/analog.c
SRC += ./dev/src/cpuclock.c
SRC += ./dev/src/dma.c
SRC += ./dev/src/fpu.c
SRC += ./dev/src/gpio.c
SRC += ./dev/src/led.c
SRC += ./dev/src/swi2c_dev.c
SRC += ./dev/src/swi2c.c
SRC += ./dev/src/systime.c
SRC += ./dev/src/spi.c
SRC += ./dev/src/spi_dev.c
SRC += ./dev/src/usart.c
SRC += ./dev/src/usart_dev.c
SRC += ./dev/src/usb.c
SRC += ./dev/src/usb_desc.c
SRC += ./dev/src/usb_core.c
SRC += ./dev/src/usb_vcp.c
SRC += ./dev/src/usb_eem.c
SRC += ./dev/src/seed.c

# flash file system
SRC += ./ffs/src/ffs.c
SRC += ./ffs/src/data_www.c

# tcp/ip network stack
SRC += ./net/tcpip/src/tcpip.c
SRC += ./net/tcpip/src/rxtx.c
SRC += ./net/tcpip/src/eth.c
SRC += ./net/tcpip/src/eth_addr.c
SRC += ./net/tcpip/src/arp.c
SRC += ./net/tcpip/src/arp_table.c
SRC += ./net/tcpip/src/ip.c
SRC += ./net/tcpip/src/ip_addr.c
SRC += ./net/tcpip/src/ip_checksum.c
SRC += ./net/tcpip/src/icmp.c
SRC += ./net/tcpip/src/icmp_checksum.c
SRC += ./net/tcpip/src/checksum.c
SRC += ./net/tcpip/src/udp.c
SRC += ./net/tcpip/src/udp_checksum.c
SRC += ./net/tcpip/src/udp_sock.c
SRC += ./net/tcpip/src/tcp.c
SRC += ./net/tcpip/src/tcp_checksum.c
SRC += ./net/tcpip/src/tcp_sock.c

# dhcp server
SRC += ./net/dhcp/src/frame.c
SRC += ./net/dhcp/src/server.c

# mdns server
SRC += ./net/mdns/src/frame.c
SRC += ./net/mdns/src/server.c

# uhttp server
SRC += ./net/uhttpsrv/src/uhttpsrv.c

# websockets
SRC += ./net/websocket/src/websocket.c

# operating system guts
SRC += ./sys/src/critical.c
SRC += ./sys/src/heap.c
SRC += ./sys/src/queue.c
SRC += ./sys/src/sem.c
SRC += ./sys/src/sleep.c
SRC += ./sys/src/time.c
SRC += ./sys/src/yield.c
SRC += ./sys/src/ev.c

# tests
SRC += ./test/src/ws.c

# utilities
SRC += ./util/src/string.c
SRC += ./util/src/stdio.c
SRC += ./util/src/strerr.c
SRC += ./util/src/sha1.c
SRC += ./util/src/sha2.c
SRC += ./util/src/base64.c
SRC += ./util/src/lfsr32.c
SRC += ./util/src/jenkins.c


# www related stuff
SRC += ./www/src/website.c
SRC += ./www/src/api.c
SRC += ./www/src/ws.c

# ----------------------------- INCLUDES ----------------------------
# put all used include directories here (use / as path separator)
INC_DIRS = .

# ---------------------------- LIBRARIES ----------------------------
# put all used libraries here starting with 'l[name]', like lm
LIBS = lm lc lgcc
# library search directories (use / as path separator)
LIB_DIRS = .

# ----------------------- OPTIMIZATION LEVEL ------------------------
# use '-O0' (no optimization) for debeugging or (-Os) for release
OPT_LEVEL = -O0

# ----------------------- OUTPUT DIRECTORIES ------------------------
# object files directory (use / as path separator)
OBJ_DIR = ../.objs
# final binaries directory (use / as path separator)
OUT_DIR = ./.outs

# --------------------- TOOLS && CONFIGURATION ----------------------
# *nix tools configuration
PATH_SEP = /
SHELL = sh
CP = cp
RM = rm -f
RMDIR = rm -r
ECHO = echo
MKDIR = mkdir -p

# ------------------------- BUILD TOOLS -----------------------------
# proper system path for selected build tools (leave empty if these are in
# system PATH)
TOOLCHAIN_PATH =

# buildtools for the gcc approach
CC = $(TOOLCHAIN_PATH)arm-none-eabi-gcc
AS = $(TOOLCHAIN_PATH)arm-none-eabi-gcc
LD = $(TOOLCHAIN_PATH)arm-none-eabi-ld
OBC = $(TOOLCHAIN_PATH)arm-none-eabi-objcopy
OBD = $(TOOLCHAIN_PATH)arm-none-eabi-objdump
NM = $(TOOLCHAIN_PATH)arm-none-eabi-nm
SIZE = $(TOOLCHAIN_PATH)arm-none-eabi-size


# ----------------------- ADDITIONAL TOOLS --------------------------
FFS_BUNDLER = python3 .tools/ffs_bundle.py

# bundling the websire
FFS_BUNDLER_WWW_INPUT_DIR = .www/
FFS_BUNDLER_WWW_OUTPUT = ffs/src/data_www.c
FFS_BUNDLER_WWW_ARGS = -r / -c -n ffs_fda_www

# list of files
FFS_BUNDLER_WWW_FILES = $(wildcard $(FFS_BUNDLER_WWW_INPUT_DIR)/*.*)

# ------------------------ PREPARE PATHS ----------------------------
# string versions of the 'versions'
SW_VER = $(SW_VER_MAJOR).$(SW_VER_MINOR).$(SW_VER_BUILD)
HW_VER = $(HW_VER_MAJOR).$(HW_VER_MINOR).$(HW_VER_BUILD)

# output directory with valid path separator
OUT_DIR_PATH = $(subst /,$(PATH_SEP),$(OUT_DIR))
OBJ_DIR_PATH = $(subst /,$(PATH_SEP),$(OBJ_DIR))

# target name with directory
TARGET_PATH = $(OUT_DIR_PATH)$(PATH_SEP)$(TARGET)
# output file name with version information
TARGET_VER = $(TARGET)_$(HW_VER)_$(SW_VER)
# target file name with path and version information
TARGET_VER_PATH = $(OUT_DIR_PATH)$(PATH_SEP)$(TARGET_VER)

# sources converted to objs with valid path separator
OBJ = $(subst /,$(PATH_SEP), $(SRC:%.c=$(OBJ_DIR_PATH)$(PATH_SEP)%.o))

# --------------------------- BUILD FLAGS ---------------------------
# optimization level and C standard
CC_FLAGS += $(OPT_LEVEL) --std=c2x
# target architecture flags
CC_FLAGS += -mcpu=cortex-m4 -march=armv7e-m -mthumb $(OPT_LEVEL)
CC_FLAGS += -mfloat-abi=hard -mfpu=fpv4-sp-d16

# warning levels
CC_FLAGS += -g3 -fdata-sections -ffunction-sections
CC_FLAGS += -Wall -Wno-format -Wno-overlength-strings
CC_FLAGS += -pedantic-errors -Wno-implicit-fallthrough
# use defines such as M_PI from math.h
CC_FLAGS += -D _USE_MATH_DEFINES
CC_FLAGS += $(addprefix -I,$(INC_DIRS))
# version information (software)
CC_FLAGS += -DSW_VER_MAJOR=$(SW_VER_MAJOR)
CC_FLAGS += -DSW_VER_MINOR=$(SW_VER_MINOR)
CC_FLAGS += -DSW_VER_BUILD=$(SW_VER_BUILD)
# version information (hardware)
CC_FLAGS += -DHW_VER_MAJOR=$(HW_VER_MAJOR)
CC_FLAGS += -DHW_VER_MINOR=$(HW_VER_MINOR)
CC_FLAGS += -DHW_VER_MINOR=$(HW_VER_BUILD)

# linker flags
LD_FLAGS  = -T$(LD_SCRIPT)
LD_FLAGS += $(addprefix -,$(LIBS)) $(addprefix -L,$(LIB_DIRS))
LD_FLAGS +=  -Wl,-Map=$(TARGET_PATH).map
LD_FLAGS += -Wl,--gc-sections,-nostdlib

# object dump flags
OBD_FLAGS  = -j ".flash_code" -j ".ram_code" -S

# object copy flags
OBC_FLAGS  = -O binary

# -------------------------- BUILD PROCESS --------------------------
# generate elf and bin and all other files
all: bundle_www $(TARGET_PATH).elf $(TARGET_PATH).lst $(TARGET_PATH).sym \
	 $(TARGET_PATH).bin size

# compile all sources
$(OBJ_DIR_PATH)$(PATH_SEP)%.o : %.c
	@ $(ECHO) --------------------- Compiling $< ---------------------
	-@ $(MKDIR) $(dir $@)
	$(CC) -c $(CC_FLAGS) $< -o $@

# link all thogether and generate elf file from objs and *.map file
$(TARGET_PATH).elf: $(OBJ)
	@ $(ECHO) ---------------------    Linking   ---------------------
	-@ $(MKDIR) $(dir $@)
	$(CC) $(CC_FLAGS) $(OBJ) --output $@ $(LD_FLAGS)

# geneate listing
$(TARGET_PATH).lst: $(TARGET_PATH).elf
	@ $(ECHO) ---------------------    Listing   ---------------------
	$(OBD) $(OBD_FLAGS) $< > $@

# generate symbol list
$(TARGET_PATH).sym: $(TARGET_PATH).elf
	@ $(ECHO) --------------------- Symbol map ---------------------
	$(NM) -n -o $(TARGET_PATH).elf > $(TARGET_PATH).sym

# generate bin file
$(TARGET_PATH).bin: $(TARGET_PATH).elf
	@ $(ECHO) --------------------   Converting   --------------------
	$(OBC) $(OBC_FLAGS) $< $@
	$(CP) $(TARGET_PATH).bin $(TARGET_VER_PATH).bin

# show size information
size: $(TARGET_PATH).elf
	@ $(ECHO) --------------------- Section size ---------------------
	$(SIZE) $(TARGET_PATH).elf

# clean build products
clean:
	- $(RM) $(OBJ)
	- $(RM) $(TARGET_PATH).elf $(TARGET_PATH).bin $(TARGET_VER_PATH).bin
	- $(RM) $(TARGET_PATH).map $(TARGET_PATH).sym $(TARGET_PATH).lst
	- $(RMDIR) $(OBJ_DIR_PATH) $(OUT_DIR_PATH)

# build using docker
build_docker:
	docker run --name $(TARGET) --rm -v $(CURDIR)/:/$(TARGET) \
	--network=host -w /$(TARGET) twatorowski/gcc-arm-none-eabi make all

# bundle the www data
bundle_www:
	$(FFS_BUNDLER) $(FFS_BUNDLER_WWW_ARGS) $(FFS_BUNDLER_WWW_INPUT_DIR) \
	$(FFS_BUNDLER_WWW_OUTPUT)