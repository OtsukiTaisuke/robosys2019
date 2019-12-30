#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#include <linux/device.h>
#include <linux/uaccess.h>    //ヘッダに追加
#include <linux/io.h>

#include <linux/timer.h>
#include <linux/delay.h>
#include <asm/delay.h>

MODULE_AUTHOR("Taisuke Ohtsuki");
MODULE_DESCRIPTION("driver for LED control");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

static dev_t dev;
static struct cdev cdv;
static struct class *cls = NULL;
static volatile u32 *gpio_base = NULL;  //アドレスをマッピングするための配列をグローバルで定義

static ssize_t led_write(struct file* filp, const char* buf, size_t count, loff_t* pos)
{

	char c;   //読み込んだ字を入れる変数
	if(copy_from_user(&c,buf,sizeof(char)))
		return -EFAULT;

	printk(KERN_INFO "led_write is called\n");
	int i;
	if(c == '0'){
		for(i=0;i<10;i++){
			gpio_base[7] = 1 << 25;
			msleep(50);
			gpio_base[10] = 1 << 25;
			msleep(50);
		}
		//gpio_base[10] = 1 << 25;
	}
	else if(c == '1'){
		for(i=0;i<10;i++){
			gpio_base[10] = 1 << 25;
			msleep(50);
			gpio_base[7] = 1 << 25;
			msleep(50);
		}
		//gpio_base[7] = 1 << 25;
	}
	return 1; //読み込んだ文字数を返す（この場合はダミーの1）
}
static ssize_t sushi_read(struct file* filp, char* buf, size_t count, loff_t* pos)
{
	int size = 0;
	char sushi[] = {0xF0,0x9F,0x8D,0xA3,0x0A}; //寿司の絵文字のバイナリ
	if(copy_to_user(buf+size,(const char *)sushi, sizeof(sushi))){
		printk( KERN_INFO "sushi : copy_to_user failed\n" );
		return -EFAULT;
	}
	size += sizeof(sushi);
	return size;
}

static struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.write = led_write
};

static void cat_dev_num(void){
	int retval;
	retval =  alloc_chrdev_region(&dev, 0, 1, "myled");
	if(retval < 0){
		printk(KERN_ERR "alloc_chrdev_region failed.\n");
		return retval;
	}
}
static void create_char_dev(void){
	int retval;
	// create char dev
	cdev_init(&cdv,&led_fops);
	retval = cdev_add(&cdv,dev,1);
	if(retval < 0){
		printk(KERN_ERR "cdev_add failed. major:%d, minor:%d",MAJOR(dev),MINOR(dev));
		return retval;
	}
}
static void create_class(void){
	// create class
	cls = class_create(THIS_MODULE,"myled");   //ここから追加
	if(IS_ERR(cls)){
		printk(KERN_ERR "class_create failed.");
		return PTR_ERR(cls);
	}
}
static int __init init_mod(void)
{
	printk(KERN_INFO "%s is loaded.\n",__FILE__);
	// cat dev  number
	gpio_base = ioremap_nocache(0x3f200000, 0xA0); //追加
	int retval;
	const u32 led = 25;
	const u32 index = led/10;//GPFSEL2
	const u32 shift = (led%10)*3;//15bit
	const u32 mask = ~(0x7 << shift);//11111111111111000111111111111111
	gpio_base[index] =(gpio_base[index]&mask)|(0x1<<shift);//001: output flag
	//11111111111111001111111111111111

	const u32 pwm = 18;
	const u32 index2 = pwm/10;
	const u32 shift2 = (pwm%10)*3;//15bit
	const u32 mask2 = ~(0x7 << shift);//11111111000111111111111111111111
	gpio_base[index2] =(gpio_base[index2]&mask2)|(0x2<<shift);//001: output flag

	//11111111010111111111111111111111

	cat_dev_num();
	printk(KERN_INFO "%s is loaded. major:%d\n",__FILE__,MAJOR(dev));
	create_char_dev();
	create_class();
	//write to class 
	device_create(cls, NULL, dev, NULL, "myled%d",MINOR(dev));
	return 0;
}

static void __exit cleanup_mod(void)
{
	//    printk(KERN_INFO "%s is unloaded.\n",__FILE__);
	cdev_del(&cdv);
	device_destroy(cls,dev);
	class_destroy(cls);
	unregister_chrdev_region(dev, 1);
	printk(KERN_INFO "%s is unloaded. major:%d\n",__FILE__,MAJOR(dev));
}


module_init(init_mod);     // マクロで関数を登録
module_exit(cleanup_mod);  // 同上
