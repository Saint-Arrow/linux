this just a test:
device register is in kernel
but driver will load in the file system
so i make some change 

1:
./driver/char/Makefile
obj-$() +=itop4412_leds.o

2:make this module ko

3:put it in filesystem

4:run it from /home/start.sh

