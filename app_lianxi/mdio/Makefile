objects = mdio.o

#CC=arm-hisiv500-linux-gcc
# CC=arm-himix200-linux-gcc
CC ?= gcc
mdio : $(objects)
	$(CC) -g -o $@ $(objects)
	- cp $@ ~/nfs


clean :
	rm -rf  $(objects)
