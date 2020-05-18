objects = mdio.o
#CC=gcc
#CC=arm-hisiv500-linux-gcc
CC=arm-himix200-linux-gcc
mdio : $(objects)
	$(CC) -g -o $@ $(objects)
	cp $@ ~/nfs


clean :
	rm -rf  $(objects)
