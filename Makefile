ASM = nasm
ASM_FLAGS = -felf64 -g -Ifortheart/
LINKER = ld

all: bin/forthwith_machine

bin/forthwith_machine: obj/forthwith_machine.o
	mkdir -p bin
	$(LINKER) -o bin/forthwith_machine -o bin/forthwith_machine obj/forthwith_machine.o

obj/forthwith_machine.o: fortheart/forthwith_machine.asm fortheart/macro.inc fortheart/outer_interpreter.inc fortheart/base.inc
	mkdir -p obj
	$(ASM) $(ASM_FLAGS) fortheart/forthwith_machine.asm -o obj/forthwith_machine.o

clean:
	rm -rf bin obj

.PHONY: clean
