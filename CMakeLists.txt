cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2_w)
# Pull in SDK (must be before project)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

# Pull in FreeRTOS
include($ENV{FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake)

project(app C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_executable(blink 
	main.c
	hw_config.c
	debug_uart.c

	rtc.c
)



pico_generate_pio_header(blink ${CMAKE_CURRENT_LIST_DIR}/debug_uart.pio)

target_compile_definitions(blink PRIVATE
  PICO_FLASH_ASSUME_CORE1_SAFE=1
)

target_include_directories(blink 
	PRIVATE 
	${CMAKE_CURRENT_LIST_DIR}
	${CMAKE_CURRENT_LIST_DIR}/libraries
)

add_subdirectory(libraries/FreeRTOS+FAT+CLI)
add_subdirectory(libraries/ds3231)
#add_subdirectory(libraries/Lab-Project-FreeRTOS-FAT)


# pull in common dependencies
target_link_libraries(blink 
	pico_stdlib
	#pico_cyw43_arch_lwip_threadsafe_background
	FreeRTOS-Kernel 
	FreeRTOS-Kernel-Heap4

	FreeRTOS+FAT+CLI
	pico_time

	pico_ds3231


)  

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(blink)
