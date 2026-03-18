CFLAGS = -fsanitize=address -fsanitize-recover=address -fno-omit-frame-pointer -Wno-format-security -fno-stack-protector -fno-var-tracking -static-libasan -g

BIN = asan
SRC= asan.c

all: $(BIN)

asan:
	$(CXX) $(CFLAGS) $(SRC) -o $(BIN)

clean:
	rm -f $(BIN)

install:
	@echo ">>>>>>>>>>>>>>>>>>> $@ >>>>>>>>>>>>>>>>>>>"
	@mkdir -p $(APP_DIR)/output
	@cp -avf $(BIN) $(APP_DIR)/output
	@cp -avf ${BIN} $(ROOTFS_DIR)/rootfs/usr/bin
