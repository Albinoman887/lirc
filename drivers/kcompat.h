/*      $Id: kcompat.h,v 5.21 2005/12/04 16:11:18 lirc Exp $      */

#ifndef _KCOMPAT_H
#define _KCOMPAT_H

#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)

#include <linux/device.h>

#define LIRC_HAVE_DEVFS
#define LIRC_HAVE_DEVFS_26
#define LIRC_HAVE_SYSFS

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13)

typedef struct class_simple lirc_class_t;

static inline lirc_class_t *class_create(struct module *owner, char *name)
{
	return class_simple_create(owner, name);
}

static inline void class_destroy(lirc_class_t *cls)
{
	class_simple_destroy(cls);
}

#define lirc_class_device_create(cs, parent, dev, device, fmt, args...) \
	class_simple_device_add(cs, dev, device, fmt, ## args)

static inline void class_device_destroy(lirc_class_t *cls, dev_t devt)
{
	class_simple_device_remove(devt);
}

#else /* >= 2.6.13 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)

#define lirc_class_device_create(cs, parent, dev, device, fmt, args...) \
	class_device_create(cs, dev, device, fmt, ## args)

#else /* >= 2.6.15 */

#define lirc_class_device_create class_device_create
#define LIRC_DEVFS_PREFIX

#endif

typedef struct class lirc_class_t;

#endif

#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
#define LIRC_HAVE_DEVFS
#define LIRC_HAVE_DEVFS_24
#endif

#ifndef LIRC_DEVFS_PREFIX
#define LIRC_DEVFS_PREFIX "usb/"
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 3, 0)
#include <linux/timer.h>
#include <linux/interrupt.h>
static inline void del_timer_sync(struct timer_list * timerlist)
{
	start_bh_atomic();
	del_timer(timerlist);
	end_bh_atomic();
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 0)
#ifdef daemonize
#undef daemonize
#endif
#define daemonize(name) do {                                           \
                                                                       \
	lock_kernel();                                                 \
	                                                               \
	exit_mm(current);                                              \
	exit_files(current);                                           \
	exit_fs(current);                                              \
	current->session = 1;                                          \
	current->pgrp = 1;                                             \
	current->euid = 0;                                             \
	current->tty = NULL;                                           \
	sigfillset(&current->blocked);                                 \
	                                                               \
	strcpy(current->comm, name);                                   \
	                                                               \
	unlock_kernel();                                               \
                                                                       \
} while (0)

/* Not sure when this was introduced, sometime during 2.5.X */
#define MODULE_PARM_int(x) MODULE_PARM(x, "i")
#define MODULE_PARM_bool(x) MODULE_PARM(x, "i")
#define MODULE_PARM_long(x) MODULE_PARM(x, "l")
#define module_param(x,y,z) MODULE_PARM_##y(x)
#else
#include <linux/moduleparam.h>
#endif /* Linux < 2.6.0 */

#ifdef LIRC_HAVE_DEVFS_24
#ifdef register_chrdev
#undef register_chrdev
#endif
#define register_chrdev devfs_register_chrdev
#ifdef unregister_chrdev
#undef unregister_chrdev
#endif
#define unregister_chrdev devfs_unregister_chrdev
#endif /* DEVFS 2.4 */

#ifndef LIRC_HAVE_SYSFS
#define class_destroy(x) do { } while(0)
#define class_create(x,y) NULL
#define class_device_destroy(x,y) do { } while(0)
#define lirc_class_device_create(x, y, z, xx, yy, zz) 0
#define IS_ERR(x) 0
typedef struct class_simple 
{
	int notused;
} lirc_class_t;	
#endif /* No SYSFS */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0)
#define KERNEL_2_5

/*
 * We still are using MOD_INC_USE_COUNT/MOD_DEC_USE_COUNT in the set_use_inc 
 * function of all modules for 2.4 kernel compatibility.
 * 
 * For 2.6 kernels reference counting is done in lirc_dev by 
 * try_module_get()/module_put() because the old approach is racy.
 * 
 */
#ifdef MOD_INC_USE_COUNT
#undef MOD_INC_USE_COUNT
#endif
#define MOD_INC_USE_COUNT

#ifdef MOD_DEC_USE_COUNT
#undef MOD_DEC_USE_COUNT
#endif
#define MOD_DEC_USE_COUNT

#ifdef EXPORT_NO_SYMBOLS
#undef EXPORT_NO_SYMBOLS
#endif
#define EXPORT_NO_SYMBOLS

#else  /* Kernel < 2.5.0 */

static inline int try_module_get(struct module *module)
{
	return 1;
}

static inline void module_put(struct module *module)
{
}

#endif /* Kernel >= 2.5.0 */

#ifndef MODULE_LICENSE
#define MODULE_LICENSE(x)
#endif

#ifndef MODULE_PARM_DESC
#define MODULE_PARM_DESC(x,y)
#endif

#ifndef MODULE_ALIAS_CHARDEV_MAJOR
#define MODULE_ALIAS_CHARDEV_MAJOR(x)
#endif

#ifndef MODULE_DEVICE_TABLE
#define MODULE_DEVICE_TABLE(x,y)
#endif

#include <linux/interrupt.h>
#ifndef IRQ_RETVAL
typedef void irqreturn_t;
#define IRQ_NONE
#define IRQ_HANDLED
#define IRQ_RETVAL(x)
#endif

#ifndef MOD_IN_USE
#ifdef CONFIG_MODULE_UNLOAD
#define MOD_IN_USE module_refcount(THIS_MODULE)
#else
#error "LIRC modules currently require"
#error "  'Loadable module support  --->  Module unloading'"
#error "to be enabled in the kernel"
#endif
#endif

#if !defined(local_irq_save)
#define local_irq_save(flags) do{ save_flags(flags);cli(); } while(0)
#endif
#if !defined(local_irq_restore)
#define local_irq_restore(flags) do{ restore_flags(flags); } while(0)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,22)
static inline char *pci_name(struct pci_dev *pdev)
{
	return pdev->slot_name;
}
#endif//Kernel<2.4.22

/*************************** I2C specific *****************************/
#include <linux/i2c.h>

#ifndef I2C_CLIENT_END
#error "********************************************************"
#error " Sorry, this driver needs the new I2C stack.            "
#error " You can get it at http://www2.lm-sensors.nu/~lm78/.    "
#error "********************************************************"
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 0)

#undef i2c_get_clientdata
#define i2c_get_clientdata(client) ((client)->data)


#undef i2c_set_clientdata
#define i2c_set_clientdata(client_ptr, new_data) do { \
	(client_ptr)->data = new_data; \
} while (0)


#endif

/* removed in 2.6.14 */
#ifndef I2C_ALGO_BIT
#   define I2C_ALGO_BIT 0
#endif

/*************************** USB specific *****************************/
#include <linux/usb.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 8)
static inline int usb_kill_urb(struct urb *urb)
{
	return usb_unlink_urb(urb);
}
#endif

/* removed in 2.6.14 */
#ifndef URB_ASYNC_UNLINK
#define URB_ASYNC_UNLINK 0  
#endif

#endif /* _KCOMPAT_H */
