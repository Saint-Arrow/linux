CC=arm-hisiv500-linux-gcc
usb_reset:main.c
	$(CC) $(CFLAGS) -lpthread -lm -o $@ $^
	cp $@ ~/nfs
