cmake_minimum_required(VERSION 2.8)
SET (CMAKE_VERBOSE_MAKEFILE OFF) 

PROJECT (sdio-test)
include (stm32f4-crosstool.cmake)
ADD_DEFINITIONS(-DUSE_STDPERIPH_DRIVER)
ADD_DEFINITIONS(-DSTM32F40XX)

INCLUDE_DIRECTORIES("../src/")
AUX_SOURCE_DIRECTORY ("../src/" APP_SOURCES)

# According to : http://www.atollic.com/index.php/truestudio/building/tsbuildercompiler, TrueSTUDIO uses GCC, so I assume this startup code is OK for any GCC.
LIST (APPEND APP_SOURCES "../3rdparty/CMSIS/ST/STM32F4xx/Source/Templates/TrueSTUDIO/startup_stm32f40xx.s")
LIST (APPEND APP_SOURCES "../3rdparty/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_rcc.c")
LIST (APPEND APP_SOURCES "../3rdparty/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_gpio.c")
LIST (APPEND APP_SOURCES "../3rdparty/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_usart.c")
#LIST (APPEND APP_SOURCES "../3rdparty/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_exti.c")
#LIST (APPEND APP_SOURCES "../3rdparty/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_syscfg.c")
#LIST (APPEND APP_SOURCES "../3rdparty/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_sdio.c")
#LIST (APPEND APP_SOURCES "../3rdparty/STM32F4xx_StdPeriph_Driver/src/stm32f4xx_dma.c")
#LIST (APPEND APP_SOURCES "../3rdparty/STM32F4xx_StdPeriph_Driver/src/misc.c")

ADD_EXECUTABLE(${CMAKE_PROJECT_NAME}.elf ${APP_SOURCES})

ADD_CUSTOM_TARGET(${CMAKE_PROJECT_NAME}.hex ALL DEPENDS ${CMAKE_PROJECT_NAME}.elf COMMAND ${CMAKE_OBJCOPY} -Oihex ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.hex)
ADD_CUSTOM_TARGET(${CMAKE_PROJECT_NAME}.bin ALL DEPENDS ${CMAKE_PROJECT_NAME}.elf COMMAND ${CMAKE_OBJCOPY} -Obinary ${CMAKE_PROJECT_NAME}.elf ${CMAKE_PROJECT_NAME}.bin)

# st-flash write stm32-cmake-test.bin 0x8000000
