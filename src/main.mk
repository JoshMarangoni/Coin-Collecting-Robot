SOURCES =                   \
	./coin.c                \
	./frequency.c           \
	./main.c                \
	./motor.c               \
	./perimeter.c           \
	./period.c              \
	./serial.c              \
	./tune.c                \

INCLUDES =                  \
	-I../include/

SHELL=cmd
CC = xc32-gcc
OBJCPY = xc32-bin2hex
ARCH = -mprocessor=32MX130F064B
PORTN=$(shell type COMPORT.inc)

OBJS = $(SOURCES:.c=.o)

main.elf: $(OBJ)
	$(CC) $(ARCH) -o main.elf $(OBJS) -mips16 -DXPRJ_default=default -legacy-libc -Wl,-Map=main.map
	$(OBJCPY) main.elf $^
	@echo Success!
   
main.o: %.c
	$(CC) -g -x c -mips16 -Os -c $(ARCH) -MMD -o -DXPRJ_default=default -legacy-libc $@ $^

compile-all:
	$(CC) $(INCLUDES) -c $(SOURCES) -g -mips16 -Os $(ARCH) -MMD -DXPRJ_default=default -legacy-libc

clean:
	@del *.o *.d *.map *.elf *.hex 2>NUL

FlashLoad:
	@Taskkill /IM putty.exe /F 2>NUL | wait 500
	pro32 -p -v main.hex
	putty.exe -serial $(PORTN) -sercfg 115200,8,n,1,N -v

putty:
	@Taskkill /IM putty.exe /F 2>NUL | wait 500
	putty.exe -serial $(PORTN) -sercfg 115200,8,n,1,N -v

dummy: main.hex main.map
	@echo ;-)
	
explorer:
	explorer .
