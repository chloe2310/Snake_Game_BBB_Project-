#include <linux/module.h>           /* Defines functions such as module_init/module_exit */
#include <linux/gpio.h>             /* Defines functions such as gpio_request/gpio_free */
#include <linux/platform_device.h>  /* For platform devices */
#include <linux/gpio/consumer.h>    /* For GPIO Descriptor */
#include <linux/of.h>               /* For DT */  
#include <linux/interrupt.h>
#include <linux/fs.h>      /* Thu vien nay dinh nghia cac ham allocate major/minor number */
#include <linux/device.h>  /* Thu vien nay dinh nghia cac ham class_create/device_create */
#include <linux/cdev.h>    /* Thu vien nay dinh nghia cac ham kmalloc */
#include<linux/slab.h>     /* Thu vien nay dinh nghia cac ham cdev_init/cdev_add */
#include<linux/uaccess.h>  /* Thu vien nay dinh nghia cac ham copy_to_user/copy_from_user *//* Thu vien nay dinh nghia cac ham cdev_init/cdev_add */
#include <linux/poll.h>
static int my_pdrv_probe(struct platform_device *pdev);
static int my_pdrv_remove(struct platform_device *pdev);


static int __init button_init(void);
static void __exit button_exit(void);


static int      m_open(struct inode *inode, struct file *file);
static int      m_release(struct inode *inode, struct file *file);
static ssize_t  m_read(struct file *filp, char __user *user_buf, size_t size,loff_t * offset);
static unsigned int m_poll(struct file *file, poll_table *wait);

typedef struct {
    dev_t dev_num;
    struct class *m_class;
    struct cdev m_cdev;
    int size;
   // char kernel_buff[1];
    bool is_event_available; // Biến kiểm tra có sự kiện mới không
    wait_queue_head_t read_queue; // Hàng đợi cho các tiến trình đọc
} m_foo_dev;

char kernel_buff[1];

m_foo_dev mdev;

#define DRIVER_AUTHOR "Chloe"
#define DRIVER_DESC   "gpio for button"

#define LOW     0
#define HIGH    1

struct gpio_desc *gpio2_4;
struct gpio_desc *gpio2_5;
struct gpio_desc *gpio1_13;
struct gpio_desc *gpio1_12;
struct gpio_desc *gpio0_23;
int irq1, irq2, irq3, irq4, irq5;
int check ;

static irqreturn_t handler1(int irq, void *dev_id);
static irqreturn_t handler2(int irq, void *dev_id);
static irqreturn_t handler3(int irq, void *dev_id);
static irqreturn_t handler4(int irq, void *dev_id);
static irqreturn_t handler5(int irq, void *dev_id);

static const struct of_device_id gpiod_dt_ids[] = {
    { .compatible = "gpio-descriptor-based", },
    { /* sentinel */ }
};

/* platform driver */
static struct platform_driver mypdrv = {
    .probe = my_pdrv_probe,
    .remove = my_pdrv_remove,
    .driver = {
        .name = "descriptor-based",
        .of_match_table = of_match_ptr(gpiod_dt_ids),
        .owner = THIS_MODULE,
    },
};

//MODULE_DEVICE_TABLE(of, gpiod_dt_ids);

static struct file_operations fops =
{
    .owner      = THIS_MODULE,
    .read       = m_read,
  //  .write      = m_write,
    .open       = m_open,
    .poll       = m_poll,
    .release    = m_release,
};

static int my_pdrv_probe(struct platform_device *pdev)
{
    pr_info("Probe function\n");
    struct device *dev = &pdev->dev;
    gpio0_23 =  gpiod_get(dev, "button23", GPIOD_IN);
    gpio1_13 = gpiod_get(dev, "button45", GPIOD_IN);
    gpio1_12=  gpiod_get(dev, "button44", GPIOD_IN);
    gpio2_4=  gpiod_get(dev, "button68", GPIOD_IN);
    gpio2_5=  gpiod_get(dev, "button69", GPIOD_IN);

    gpiod_set_debounce(gpio0_23, 200);
    gpiod_set_debounce(gpio1_12, 200);
    gpiod_set_debounce(gpio1_13, 200);
    gpiod_set_debounce(gpio2_4, 200);
    gpiod_set_debounce(gpio2_5, 200);

    irq1 = gpiod_to_irq(gpio0_23);
    irq2 = gpiod_to_irq(gpio1_12);
    irq3 = gpiod_to_irq(gpio1_13);
    irq4 = gpiod_to_irq(gpio2_4);
    irq5 = gpiod_to_irq(gpio2_5);

    check = request_irq(irq1,handler1, IRQF_TRIGGER_FALLING, "button_irq1", NULL);
    if (check) {
    pr_err("Failed to request IRQ for Button GPIO\n");
    gpiod_put(gpio0_23);
    return check;
    }

    check = request_irq(irq2,handler2, IRQF_TRIGGER_FALLING, "button_irq2", NULL);
    if (check) {
    pr_err("Failed to request IRQ for Button GPIO\n");
    gpiod_put(gpio1_12);
    return check;
    }

    check = request_irq(irq3,handler3, IRQF_TRIGGER_FALLING, "button_irq3", NULL);
    if (check) {
    pr_err("Failed to request IRQ for Button GPIO\n");
    gpiod_put(gpio1_13);
    return check;
    }

    check = request_irq(irq4,handler4, IRQF_TRIGGER_FALLING, "button_irq4", NULL);
    if (check) {
    pr_err("Failed to request IRQ for Button GPIO\n");
    gpiod_put(gpio2_4);
    return check;
    }

    check = request_irq(irq5,handler5, IRQF_TRIGGER_FALLING, "button_irq5", NULL);
    if (check) {
    pr_err("Failed to request IRQ for Button GPIO\n");
    gpiod_put(gpio2_5);
    return check;
    }

    return 0;
}

static int my_pdrv_remove(struct platform_device *pdev)
{
    pr_info("remove function\n");

    free_irq(irq1, NULL);
    gpiod_put(gpio0_23);

    free_irq(irq2, NULL);
    gpiod_put(gpio1_12);

    free_irq(irq3, NULL);
    gpiod_put(gpio1_13);
    free_irq(irq4, NULL);
    gpiod_put(gpio2_4);

    free_irq(irq5, NULL);
    gpiod_put(gpio2_5);
    pr_info("End remove\n");

    return 0;
}

static irqreturn_t handler1(int irq, void *dev_id)
{
    bool pressed = gpiod_get_value(gpio0_23);
    
    if (pressed)
        pr_info("Button1 Pressed\n");
    else
        pr_info("Button1 Released\n");
    
    kernel_buff[0] = '1';
    mdev.is_event_available = true;
    wake_up_interruptible(&mdev.read_queue);
    return IRQ_HANDLED;
}

static irqreturn_t handler2(int irq, void *dev_id)
{
    bool pressed = gpiod_get_value(gpio1_12);
    
    if (pressed)
        pr_info("Button2 Pressed\n");
    else
        pr_info("Button2 Released\n");

    kernel_buff[0] = '2';
    mdev.is_event_available = true;
    wake_up_interruptible(&mdev.read_queue);
    return IRQ_HANDLED;
}

static irqreturn_t handler3(int irq, void *dev_id)
{
    bool pressed = gpiod_get_value(gpio1_13);
    
    if (pressed)
        pr_info("Button3 Pressed\n");
    else
        pr_info("Button3 Released\n");
    
    kernel_buff[0] = '3';
    mdev.is_event_available = true;
    wake_up_interruptible(&mdev.read_queue);
    return IRQ_HANDLED;
}

static irqreturn_t handler4(int irq, void *dev_id)
{
    bool pressed = gpiod_get_value(gpio2_4);
    
    if (pressed)
        pr_info("Button4 Pressed\n");
    else
        pr_info("Button4 Released\n");

    kernel_buff[0] = '4';
    mdev.is_event_available = true;
    wake_up_interruptible(&mdev.read_queue);
    return IRQ_HANDLED;
}

static irqreturn_t handler5(int irq, void *dev_id)
{
    bool pressed = gpiod_get_value(gpio2_5);
    
    if (pressed)
        pr_info("Button5 Pressed\n");
    else
        pr_info("Button5 Released\n");

    kernel_buff[0] = '5';
    mdev.is_event_available = true;
    wake_up_interruptible(&mdev.read_queue);
    return IRQ_HANDLED;
}

/* This function will be called when we open the Device file */
static int m_open(struct inode *inode, struct file *file)
{
    pr_info("System call open() called...!!!\n");
    return 0;
}

/* This function will be called when we close the Device file */
static int m_release(struct inode *inode, struct file *file)
{
    pr_info("System call close() called...!!!\n");
    return 0;
}

/* This function will be called when we read the Device file */
static ssize_t m_read(struct file *filp, char __user *user_buf, size_t size, loff_t *offset)
{
    ssize_t result = 0;

    // wait until button press
    wait_event_interruptible(mdev.read_queue, mdev.is_event_available);

    // check data avaiable
    if (!mdev.is_event_available) {
        return 0;
    }

    // Copy data from kernel buff to user buff
    if (copy_to_user(user_buf, kernel_buff, 1)) {
        // Trả về lỗi nếu không copy được
        result = -EFAULT;
    } else {
        // read data done
        mdev.is_event_available = false; // Reset lại cờ sự kiện
        result = 1; // Trả về số byte đã copy
    }

    return result;

}


static unsigned int m_poll(struct file *file, poll_table *wait)
{
    unsigned int reval_mask = 0;
    poll_wait(file, &mdev.read_queue, wait);
    if (mdev.is_event_available)
    {
        reval_mask |= POLLIN | POLLRDNORM; // Dữ liệu đọc sẵn có
    }
    //pr_info("Nodata send\n");
    return reval_mask;
}

static int __init button_init(void)
{
    if (alloc_chrdev_region(&mdev.dev_num, 0, 1, "my-cdev"))
    {
        pr_err("ERROR: Failed to allocate character device number\n");
        return -1;
    }

    pr_info("Initialization started\n");

    pr_info("Allocated device number: Major = %d, Minor = %d\n",
            MAJOR(mdev.dev_num), MINOR(mdev.dev_num));

    if ((mdev.m_class = class_create(THIS_MODULE, "my_class")) == NULL) {
        pr_err("ERROR: Failed to create device class\n");
        goto rm_dev_num;
    }
    pr_info("Device class created successfully\n");

    if (device_create(mdev.m_class, NULL, mdev.dev_num, NULL, "my_button_device") == NULL)
    {
        pr_err("ERROR: Failed to create device node\n");
        goto rm_class;
    }
    pr_info("Device node created successfully\n");

    cdev_init(&mdev.m_cdev, &fops);
    pr_info("Character device initialized\n");

    if (cdev_add(&mdev.m_cdev, mdev.dev_num, 1) < 0)
    {
        pr_err("ERROR: Failed to add character device\n");
        goto rm_dev_num;
    }
    pr_info("Character device added successfully\n");

    platform_driver_register(&mypdrv);
    pr_info("Platform driver registered\n");

    init_waitqueue_head(&mdev.read_queue);
    mdev.is_event_available = false;

    pr_info("Initialization completed successfully\n");

    return 0;

rm_dev_num:
    unregister_chrdev_region(mdev.dev_num, 1);
rm_class:
    class_destroy(mdev.m_class);

    return -1;
}


static void __exit button_exit(void)
{
    pr_info("Exit started\n");

    platform_driver_unregister(&mypdrv);
    pr_info("Platform driver unregistered\n");

    cdev_del(&mdev.m_cdev);
    pr_info("Character device removed\n");

    device_destroy(mdev.m_class, mdev.dev_num);
    pr_info("Device node destroyed\n");

    class_destroy(mdev.m_class);
    pr_info("Device class destroyed\n");

    unregister_chrdev_region(mdev.dev_num, 1);
    pr_info("Character device number unregistered\n");

    pr_info("Exit completed\n");
}


module_init(button_init);
module_exit(button_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chloe");
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_VERSION("1.0");