# GCC toolchain prefix
#SET(TOOLCHAIN_PREFIX "/home/iwasz/x-tools/arm-unknown-eabi")
SET(TOOLCHAIN_PREFIX "/home/iwasz/local/share/arm-unknown-eabi")
SET(TARGET_TRIPLET "arm-unknown-eabi")

SET(TOOLCHAIN_BIN_DIR ${TOOLCHAIN_PREFIX}/bin)
SET(TOOLCHAIN_INC_DIR ${TOOLCHAIN_PREFIX}/${TARGET_TRIPLET}/include)
SET(TOOLCHAIN_LIB_DIR ${TOOLCHAIN_PREFIX}/${TARGET_TRIPLET}/lib)

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_SYSTEM_PROCESSOR arm)

SET(CMAKE_C_COMPILER ${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-gcc)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-g++)
SET(CMAKE_ASM_COMPILER ${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-as)
SET(CMAKE_OBJCOPY ${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-objcopy)
SET(CMAKE_OBJDUMP ${TOOLCHAIN_BIN_DIR}/${TARGET_TRIPLET}-objdump)

SET(CMAKE_C_FLAGS_DEBUG "-O0 -g -ggdb -gstabs+" CACHE INTERNAL "c debug compiler flags")
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb -g -gstabs+" CACHE INTERNAL "cxx debug compiler flags")
SET(CMAKE_ASM_FLAGS_DEBUG "-g -gstabs+" CACHE INTERNAL "asm debug compiler flags")

SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS_DEBUG} CACHE INTERNAL "c compiler flags")
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS_DEBUG} CACHE INTERNAL "cxx compiler flags")

# -flto powoduje problemy z funkcjami syscalls
SET(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE INTERNAL "c release compiler flags")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG" CACHE INTERNAL "cxx release compiler flags")
SET(CMAKE_ASM_FLAGS_RELEASE "" CACHE INTERNAL "asm release compiler flags")

SET(CMAKE_EXE_LINKER_FLAGS "-T${CMAKE_CURRENT_BINARY_DIR}/stm32f4.ld " CACHE INTERNAL "exe link flags")
SET(CMAKE_MODULE_LINKER_FLAGS "-L${TOOLCHAIN_LIB_DIR}" CACHE INTERNAL "module link flags")
SET(CMAKE_SHARED_LINKER_FLAGS "-L${TOOLCHAIN_LIB_DIR}" CACHE INTERNAL "shared link flags")

SET(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_PREFIX}/${TARGET_TRIPLET} CACHE INTERNAL "cross root directory")
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH CACHE INTERNAL "")
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY CACHE INTERNAL "")
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY CACHE INTERNAL "")
SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
SET(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

INCLUDE_DIRECTORIES(
    ../3rdparty/CMSIS/Include
    ../3rdparty/CMSIS/ST/STM32F4xx/Include
    ../3rdparty/STM32F4xx_StdPeriph_Driver/inc
    ../3rdparty/STM32_USB_OTG_Driver/inc
    ../3rdparty/STM32_USB_Device_Library/Core/inc
    ../3rdparty/STM32_USB_Device_Library/Class/audio/inc
    ../3rdparty/STM32_USB_Device_Library/Class/cdc/inc
    ../3rdparty/STM32_USB_Device_Library/Class/dfu/inc
    ../3rdparty/STM32_USB_Device_Library/Class/hid/inc
    ../3rdparty/STM32_USB_Device_Library/Class/msc/inc        
    ../3rdparty/STM32_USB_HOST_Library/Core/inc
    ../3rdparty/STM32_USB_HOST_Library/Class/HID/inc
    ../3rdparty/STM32_USB_HOST_Library/Class/MSC/inc
)

enable_language(ASM-ATT)

