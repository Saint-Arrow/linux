#include "../my_driver.h"

#include "mach/gpio.h"
#include "plat/gpio-cfg.h"
#include <mach/regs-gpio.h>
#include <asm/io.h>

unsigned int  phy_addr,virt_addr;
unsigned int  *GPL2CON,*GPL2DAT,*GPL2PUD;
#define BASE_PHY_ADDR 0x11000100
void led_init(void)
{
	phy_addr=BASE_PHY_ADDR;
	virt_addr=(unsigned int)ioremap(phy_addr,0x10);
	GPL2CON=(unsigned int  *)virt_addr+0x00;
	GPL2DAT = (unsigned int *)(virt_addr+0x04);
	GPL2PUD	=(unsigned int *)(virt_addr+0x08);
	//set output
	*GPL2CON &= 0xFFFFFF00;
	*GPL2CON |= 0X00000001;
	//PULL UP
	*GPL2PUD |=0x0003;
}
void led_on(void)
{
	*GPL2DAT |= 0X01;
}
void led_off(void)
{
	*GPL2DAT &= ~0X01;
}


static int gpio_iomap_init(void)
{
	int ret=0;
	printk(KERN_EMERG "my_leds driver enter\n");
    led_init();
	led_on();
	return ret;
}
static int gpio_iomap_exit(void)
{
	led_off();
	return 0;
}
module_init(gpio_iomap_init);
module_exit(gpio_iomap_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("ARROW");

