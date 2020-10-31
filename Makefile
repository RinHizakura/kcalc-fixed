KDIR=/lib/modules/$(shell uname -r)/build

obj-m += calc.o
calc-objs += main.o expression.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

GIT_HOOKS := .git/hooks/applied

all: $(GIT_HOOKS) eval client
	make -C $(KDIR) M=$(PWD) modules

$(GIT_HOOKS):
	scripts/install-git-hooks
	echo

load: calc.ko
	sudo insmod calc.ko
	sudo chmod 0666 /dev/calc

unload:
	sudo rmmod calc

check: all
	scripts/test.sh

eval: eval.c
	$(CC) -o $@ $< -std=gnu11

client: client.c
	$(CC) -o $@ $< -lm -std=gnu11

bench: all
	$(MAKE) load
	./client
	$(MAKE) unload

clean:
	make -C $(KDIR) M=$(PWD) clean
	$(RM) eval
