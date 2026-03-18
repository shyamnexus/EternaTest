#--------- ENVIRONMENT SETTING --------------------
INCLUDES	= -I$(NVT_HDAL_DIR)/include -I$(LIBRARY_DIR)/include -I$(NVT_VOS_DIR)/drivers/include -I./live555/include
WARNING		= -Wall -Wundef -Wsign-compare -Wno-missing-braces -Wstrict-prototypes
COMPILE_OPTS	= $(INCLUDES) -I. -O2 -fPIC -ffunction-sections -fdata-sections
CPPFLAGS	=
CFLAGS		= $(PLATFORM_CFLAGS) $(PRJCFG_CFLAGS)
C_FLAGS		= $(NVT_ASAN) $(COMPILE_OPTS) $(CPPFLAGS) $(CFLAGS) $(WARNING) -D_NETWORK_ON_CPU1_
LD_FLAGS	= $(NVT_ASAN) -L$(LIBRARY_DIR)/output -L$(NVT_HDAL_DIR)/output -L./live555/lib -lstdc++ -lpthread -lm -lrt -lliveMedia -lgroupsock -lBasicUsageEnvironment -lUsageEnvironment -lssl -lcrypto -lhdal
#--------- END OF ENVIRONMENT SETTING -------------
DEP_LIBRARIES := 
#--------- Compiling -------------------
BIN = nvtrtspd_ipc
SRC_C = nvtrtspd.c video_record.c detection.c nalu_header.c
SRC_CPP = custom_nvtlive555.cpp
HEADER = $(shell find . -name "*.h")

OBJ = $(SRC_C:.c=.o) $(SRC_CPP:.cpp=.o)

.PHONY: all clean install

ifeq ("$(wildcard *.c */*.c)","")
all:
	@echo ">>> Skip"
clean:
	@echo ">>> Skip"

else
all: $(BIN)

$(BIN): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LD_FLAGS)
	$(NM) -n $@ > $@.sym
	$(STRIP) $@
	$(OBJCOPY) -R .comment -R .note.ABI-tag -R .gnu.version $@

%.o: %.c $(HEADER)
	$(CC) $(C_FLAGS) -c $< -o $@

%.o: %.cpp $(HEADER)
	$(CXX) $(C_FLAGS) -c $< -o $@

clean:
	rm -vf $(BIN) $(OBJ) $(BIN).sym *.o *.a *.so*
endif

install:
	@echo ">>>>>>>>>>>>>>>>>>> $@ >>>>>>>>>>>>>>>>>>>"
	@mkdir -p $(APP_DIR)/output
	@cp -avf $(BIN) $(APP_DIR)/output
	@cp -avf ${BIN} ${ROOTFS_DIR}/rootfs/bin/
