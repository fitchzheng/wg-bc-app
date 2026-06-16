cmake_minimum_required(VERSION 3.10)
project(plecs LANGUAGES C)

# 强制使用 64 位 GCC 工具链
set(CMAKE_C_COMPILER "C:/mingw64/bin/x86_64-w64-mingw32-gcc.exe")
set(CMAKE_CXX_COMPILER "C:/mingw64/bin/x86_64-w64-mingw32-g++.exe")

# 包含路径
include_directories(${CMAKE_SOURCE_DIR}/code/plecs)
include_directories(${CMAKE_SOURCE_DIR}/code/section)
include_directories(${CMAKE_SOURCE_DIR}/code/user/L1)
include_directories(${CMAKE_SOURCE_DIR}/code/user/L2)
include_directories(${CMAKE_SOURCE_DIR}/code/user/L3)
include_directories(${CMAKE_SOURCE_DIR}/code/user/L4/plecs)
include_directories("D:/Documents/Plexim/PLECS 4.7 (64 bit)/include/plecs/")

# 源文件
set(SOURCES
    code/plecs/plecs.c
    code/section/section.c
    code/user/L1/ctrl_app.c
    code/user/L1/data_com.c
    code/user/L1/fsm.c
    code/user/L1/protect.c
    code/user/L1/led.c
    code/user/L2/buck_boost.c
    code/user/L2/curr_lmt.c
    code/user/L2/l_loop_open.c
    code/user/L2/l_loop.c
    code/user/L2/volt_loop.c
    code/user/L2/open_loop.c
    code/user/L2/adc_check.c
    code/user/L2/fault.c
    code/user/L2/power_sw.c
    code/user/L2/led_ctrl.c
    code/user/L3/shell.c
    code/user/L3/adc.c
    code/user/L3/pid.c
    code/user/L3/gpio.c
    code/user/L3/temperature.c
    code/user/L3/wg_com_v2.c
    code/user/L4/plecs/bsp_pwm.c
    code/user/L4/plecs/bsp_adc.c
    code/user/L4/plecs/bsp_gpio.c
    code/user/L4/plecs/bsp_usart.c
)

# 添加全局宏定义
add_definitions(-DIS_PLECS)

# 生成共享库（DLL）
add_library(plecs SHARED ${SOURCES})

# 确保输出目录存在
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# 设置输出路径
set_target_properties(plecs PROPERTIES
    OUTPUT_NAME "plecs"
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# 生成 MAP 文件（可选）
if(WIN32)
    set_target_properties(plecs PROPERTIES
        LINK_FLAGS "-Wl,-Map,${CMAKE_BINARY_DIR}/bin/mydll.map"
    )
endif()
