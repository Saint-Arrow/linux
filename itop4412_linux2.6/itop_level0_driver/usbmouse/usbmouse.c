#include "../my_driver.h"
/* for apple IDs */
#ifdef CONFIG_USB_HID_MODULE
#include "../hid-ids.h"
#endif


/*
 * Version Information
 */
#define DRIVER_VERSION "v1.6"
#define DRIVER_AUTHOR "Vojtech Pavlik <vojtech@ucw.cz>"
#define DRIVER_DESC "USB HID Boot Protocol mouse driver"
#define DRIVER_LICENSE "GPL"

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE(DRIVER_LICENSE);
struct usb_mouse {
	char name[128];
	char phys[64];
	struct usb_device *usbdev;
	struct input_dev *dev;
	struct urb *irq;

	signed char *data;
	dma_addr_t data_dma;
};
int pipe, maxp;
struct usb_mouse *mouse;
int count;
static void check_usb_data(struct urb *urb)
{
	printk("mouse %d\n",count++);
	usb_submit_urb(urb, GFP_ATOMIC);
}
static int usb_mouse_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(intf);
	struct usb_endpoint_descriptor *endpoint;

	int error = -ENOMEM;
	printk("usb_mouse_probe");
	endpoint=&(intf->cur_altsetting->endpoint[0].desc);
	pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);
	maxp = usb_maxpacket(dev, pipe, usb_pipeout(pipe));
	mouse = kzalloc(sizeof(struct usb_mouse), GFP_KERNEL); 
	mouse->data=usb_alloc_coherent(dev,8,GFP_ATOMIC,&mouse->data_dma);
	if (!mouse->data)
		goto fail1;
	mouse->irq=usb_alloc_urb(0,GFP_KERNEL);
	if (!mouse->irq)
		goto fail2;
	usb_fill_int_urb(mouse->irq,dev,pipe,mouse->data,\
					(maxp>8?8:maxp),\
					check_usb_data,NULL,endpoint->bInterval);
	mouse->irq->transfer_dma=mouse->data_dma;
	mouse->irq->transfer_flags |=URB_NO_TRANSFER_DMA_MAP;
	error=usb_submit_urb(mouse->irq,GFP_KERNEL);
	if(error)
	{
		return error;	
	}
	return 0;
fail3:	
	usb_free_urb(mouse->irq);
fail2:	
	usb_free_coherent(dev, 8, mouse->data, mouse->data_dma);
fail1:	
	//input_free_device(input_dev);
	kfree(mouse);
	return error;	
}

static void usb_mouse_disconnect(struct usb_interface *intf)
{
	printk("usb_mouse_disconnect");
	usb_kill_urb(mouse->irq);
	usb_free_urb(mouse->irq);
	usb_free_coherent(interface_to_usbdev(intf), 8, mouse->data, mouse->data_dma);
	kfree(mouse);
}

static struct usb_device_id usb_mouse_id_table [] = {
        { USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
                USB_INTERFACE_PROTOCOL_MOUSE) },
        { }     /* Terminating entry */
};

MODULE_DEVICE_TABLE (usb, usb_mouse_id_table);

static struct usb_driver usb_mouse_driver = {
        .name           = "usbmouse",
        .probe          = usb_mouse_probe,
        .disconnect     = usb_mouse_disconnect,
        .id_table       = usb_mouse_id_table,
};

static int __init usb_mouse_init(void)
{
        return usb_register(&usb_mouse_driver);
}

static void __exit usb_mouse_exit(void)
{
        usb_deregister(&usb_mouse_driver);
}


module_init(usb_mouse_init);
module_exit(usb_mouse_exit);