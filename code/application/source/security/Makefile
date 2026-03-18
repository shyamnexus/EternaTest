
include $(NVT_PRJCFG_MODEL_CFG)
#--------- ENVIRONMENT SETTING --------------------
INCLUDES	= 
WARNING		= 
COMPILE_OPTS	= 
CPPFLAGS	= 
CFLAGS		= 
C_FLAGS		= $(COMPILE_OPTS) $(CPPFLAGS) $(CFLAGS) $(WARNING)
LD_FLAGS	= 
#--------- END OF ENVIRONMENT SETTING -------------
DEP_LIBRARIES := 
#--------- Compiling -------------------
BIN = canary
SRC = canary.c
BIN2 = nx
SRC2 = sample.c
BIN3 = pie
SRC3 = sample.c


OBJ = $(SRC:.c=.o)
OBJ2 = $(SRC2:.c=.o)
OBJ3 = $(SRC3:.c=.o)

.PHONY: all clean install

ifeq ("$(wildcard *.c */*.c)","")
all:
	@echo ">>> Skip"
clean:
	@echo ">>> Skip"
else

all: $(BIN) $(BIN2) $(BIN3)

$(BIN): $(OBJ)
	$(CC) -fstack-protector-all -g -o canary canary.c

$(BIN2): $(OBJ2)
	$(CC) -Wa,--noexecstack -o $@ $(OBJ2) $(LD_FLAGS)

$(BIN3): $(OBJ3)
	$(CC) -fpie -pie -o pie sample.c


%.o:%.c
	$(CC) $(C_FLAGS) -c $< -o $@

clean:
	rm -vf $(BIN) $(OBJ) $(BIN).sym *.o *.a *.so*
	rm -vf $(BIN2) $(OBJ2) $(BIN2).sym *.o *.a *.so*
	rm -vf $(BIN3) $(OBJ3) $(BIN3).sym *.o *.a *.so*

endif
install:
	@echo ">>>>>>>>>>>>>>>>>>> $@ >>>>>>>>>>>>>>>>>>>"
	@cp -avf $(BIN) $(BIN2) $(BIN3) $(ROOTFS_DIR)/rootfs/usr/bin/

