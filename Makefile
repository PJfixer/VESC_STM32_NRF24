######################################################################
#  Project Makefile
######################################################################

BINARY		= main
SRCFILES	= main.c usart1.c usart3.c miniprintf.c vesc/crc.c vesc/buffer.c vesc/vesc.c rtos/heap_4.c rtos/list.c rtos/port.c rtos/queue.c rtos/tasks.c rtos/opencm3.c
LDSCRIPT	= stm32f103c8t6.ld

# DEPS		= 	# Any additional dependencies for your build
# CLOBBER	+= 	# Any additional files to be removed with "make clobber"

include ../../Makefile.incl
include ../Makefile.rtos

pjea_flash:
	openocd -f /usr/share/openocd/scripts/interface/stlink-v2.cfg -f  /usr/share/openocd/scripts/target/stm32f1x.cfg -c init -c "reset halt" -c "flash write_image erase main.bin 0x08000000" -c "reset run" -c exit
	
pjea_reset:
	openocd -f /usr/share/openocd/scripts/interface/stlink-v2.cfg -f  /usr/share/openocd/scripts/target/stm32f1x.cfg -c init -c "reset run" -c exit
######################################################################
#  NOTES:
#	1. remove any modules you don't need from SRCFILES
#	2. "make clean" will remove *.o etc., but leaves *.elf, *.bin
#	3. "make clobber" will "clean" and remove *.elf, *.bin etc.
#	4. "make flash" will perform:
#	   st-flash write main.bin 0x8000000
######################################################################
