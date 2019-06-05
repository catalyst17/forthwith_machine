ASM = nasm
ASM_FLAGS = -felf64 -f -Ifortheart/
LINKER = ld

all: bin/forthwith_machine

bin/forthwith_machine: obj/forthwith_machine.o
    mkdir -p bin
    $(LINKER) -0 bin/forthwith_machine -o bin/forthwith_machine obj/forthwith_machine.o

obj/forthwith_machine.o: fortheart/forthwith_machine.asm fortheart/macro.inc fortheart/interpreter.inc fortheart/forth-utils.inc
    mkdir -p obj
    $(ASM) $(ASM_FLAGS) fortheart/forthwith_machine.asm -o obj/forthwith_machine.o

clean:
    rm -rf bin obj

.PHONY: clean
