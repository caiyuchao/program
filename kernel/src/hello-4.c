/*
 * The following license idents are currently accepted as indicating free
 * software modules
 *
 *  "GPL"               [GNU Public License v2 or later]
 *  "GPL v2"            [GNU Public License v2]
 *  "GPL and additional rights" [GNU Public License v2 rights and more]
 *  "Dual BSD/GPL"          [GNU Public License v2
 *                   or BSD license choice]
 *  "Dual MPL/GPL"          [GNU Public License v2
 *                   or Mozilla license choice]
 *
 * The following other idents are available
 *
 *  "Proprietary"           [Non free products]
 *
 * There are dual licensed components, but when running with Linux it is the
 * GPL that is relevant so this is a non issue. Similarly LGPL linked with GPL
 * is a GPL combined work.
 *
 * This exists for several reasons
 * 1.   So modinfo can show license info for users wanting to vet their setup 
 *  is free
 * 2.   So the community can ignore bug reports including proprietary modules
 * 3.   So vendors can do likewise based on their own policies
 */
/*  
 *  hello-4.c - Demonstrates module documentation.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#define DRIVER_AUTHOR "Peter Jay Salzman <p@dirac.org>"
#define DRIVER_DESC   "A sample driver"

static int __init init_hello_4(void)
{
    printk(KERN_ALERT "Hello, world 4\n");
    return 0;
}

static void __exit cleanup_hello_4(void)
{
    printk(KERN_ALERT "Goodbye, world 4\n");
}

module_init(init_hello_4);
module_exit(cleanup_hello_4);

/*  
 *  You can use strings, like this:
 */

/* 
 * Get rid of taint message by declaring code as GPL. 
 */
MODULE_LICENSE("GPL");

/*
 * Or with defines, like this:
 */
MODULE_AUTHOR(DRIVER_AUTHOR);   /* Who wrote this module? */
MODULE_DESCRIPTION(DRIVER_DESC);    /* What does this module do */

/*  
 *  This module uses /dev/testdevice.  The MODULE_SUPPORTED_DEVICE macro might
 *  be used in the future to help automatic configuration of modules, but is 
 *  currently unused other than for documentation purposes.
 */
MODULE_SUPPORTED_DEVICE("testdevice");

