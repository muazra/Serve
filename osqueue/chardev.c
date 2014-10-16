#include "chardev.h"

/* Globals localized to file (by use of static */
static int Major;		/* assigned to device driver */
static char msg[BUF_LEN];	/* a stored message */

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release
};

static struct qnode {
	unsigned int fd;
	struct node *next;
};
struct qnode *head;

static int device_open(struct inode *inode, struct file *file)
{
	try_module_get(THIS_MODULE);
	head = (struct qnode*)kmalloc(sizeof(struct qnode), GFP_KERNEL);
	head -> fd = NULL;
	head -> next = NULL;
	return 0;
}

static int device_release(struct inode *inode, struct file *file)
{
	module_put(THIS_MODULE);
	while(head != NULL){
		struct qnode *temp = head;
	        kfree(temp);
		head = head->next;
	}
	return 0;
}

/* Called when a process writes to dev file: echo "hi" > /dev/hello */
static ssize_t device_write(struct file *filp, const char *buff,
			    size_t len, loff_t * off)
{
	int copy_len = len;
	unsigned long amnt_copied = 0;
	struct qnode *node = (struct qnode*)kmalloc(sizeof(struct qnode), GFP_KERNEL);

	/* NOTE: copy_from_user returns the amount of bytes _not_ copied */
	amnt_copied = copy_from_user((void*)&node->fd, (unsigned int*)buff, copy_len);
	if (copy_len == amnt_copied)
		return -EINVAL;

	node->next = head;
	head = node;

	return copy_len - amnt_copied;
}

static ssize_t device_read(struct file *filp, char *buffer, size_t len,
			   loff_t * offset)
{
	unsigned long amnt_copied;
	int copy_len = len;

	/* are we at the end of the buffer? */
	if ((head == NULL) || (head->fd == NULL))
		return 0;

	/* NOTE: copy_to_user returns the amount of bytes _not_ copied */
	amnt_copied = copy_to_user((unsigned int*)buffer, (void*)&head->fd, copy_len);
	if (copy_len == amnt_copied)
		return -EINVAL;

	struct qnode *temp = head;
	head = head->next;
	kfree(temp);

	return copy_len - amnt_copied;
}

int init_module(void)
{
	Major = register_chrdev(0, DEVICE_NAME, &fops);

	if (Major < 0) {
	   printk(KERN_ALERT "Failed to register char device.\n");
	   return Major;
	}

	memset(msg, '+', BUF_LEN);
	//printk(KERN_INFO "chardev is assigned to major number %d.\n", Major);

	return 0;
}
void cleanup_module(void)
{
	int ret = unregister_chrdev(Major, DEVICE_NAME);
	if (ret < 0)
		printk(KERN_ALERT "Error in unregister_chrdev: %d\n", ret);
}
