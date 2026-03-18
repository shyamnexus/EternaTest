# DO NOT USE SAPCE AT LEFT SIDE AND RIGHT SIDE OF EQUAL.
# e.g DO NOT USE 'NVT_FPGA = OFF', but rather 'NVT_FPGA=OFF'
export NVT_FPGA=OFF
export NVT_EMULATION=OFF
export NVT_RUN_CORE2=OFF
export SDK_CODENAME=ns02302
export RTOS_NVTTIMER=ON

# this setting only for RTOS.
# cortex-a53
# cortex-a9
# cortex-a53x64
export RTOS_CPU_TYPE=cortex-a53x64

# this setting only for Linux
# cortex-a53
# cortex-a9
# cortex-a53x64
export LINUX_CPU_TYPE=cortex-a53x64

# this setting for build IPC or NVR SDK
# NVT_IPC_SDK
# NVT_NVR_SDK
export NVT_SDK_TYPE=NVT_IPC_SDK

# this setting only for RTOS.
# amazon-freertos
# freertos
export RTOS_TYPE=freertos

# remove mark to enable asan for all libs and apps
# to disable it, remark it and type 'unset NVT_ASAN' on your shell
#export NVT_ASAN="-fsanitize=address -fsanitize-recover=address  -fno-omit-frame-pointer -static-libasan"
