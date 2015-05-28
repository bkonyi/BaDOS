#
# Makefile for busy-wait IO library
#
CC  = gcc
AS	= as
AR	= ar
LD  = ld
CFLAGS  = -O3 -c -fPIC -Wall -Werror -mcpu=arm920t -msoft-float
# -g: include hooks for gdb
# -c: only compile
# -mcpu=arm920t: generate code for the 920t architecture
# -fpic: emit position-independent code
# -Wall: report all warnings
# -msoft-float: use software for floating point

ASFLAGS	= -mcpu=arm920t -mapcs-32
# -mapcs-32: always create a complete stack frame

ARFLAGS = rcs

#define the executable
EXEC = bados.elf

LDFLAGS = -init main -N -Map $(EXEC:.elf=.map) -T orex.ld 

#define the source files
SRCS = $(shell find $(SOURCEDIR) -name '*.c') 

# define any directories containing header files
#
INCLUDES = -I. -I../include $(foreach dir, $(wildcard ../include/**), -I$(dir)) 

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS = -L/u/wbcowan/gnuarm-4.0.2/lib/gcc/arm-elf/4.0.2

# define any libraries to link into executable:
#   if I want to link in libraries (libx.so or libx.a) I use the -llibname 
#   option, something like (this will link in libmylib.so and libm.so:
LIBS = -lgcc

# define the C object files 
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS:.c=.o) context_switch.o user/send_sys_call.o

# define the assembly files
ASSM = $(SRCS:.c=.s) context_switch.s user/send_sys_call.s

all: $(EXEC)
	rm -f /u/cs452/tftp/ARM/$(EXEC) 
	cp $(EXEC) /u/cs452/tftp/ARM/$(EXEC)
	chmod 777 /u/cs452/tftp/ARM/$(EXEC)
# Main target
$(EXEC): $(ASSM) $(OBJS)
	$(LD) $(LDFLAGS) $(LFLAGS) -o $@ $(OBJS) $(LIBS)

# To obtain assembly files
%.s: %.c
	$(CC) -S $(CFLAGS) $(INCLUDES) -o $@ $<

# To obtain object files
%.o: %.s
	$(AS) $(ASFLAGS) -o $@ $<

# To remove generated files
clean:
	rm -f $(EXEC) $(EXEC:.elf=.map) $(OBJS) $(SRCS:.c=.s)
