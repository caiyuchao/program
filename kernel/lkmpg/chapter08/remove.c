#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/list.h>
#include<linux/cpumask.h>

static int __init mymod_init(void)
{
	struct module *mod,*relate;
	int cpu;

	// 打印本模块的模块名和模块状态
	printk(KERN_ALERT"[insmod mymod] name:%s state:%d\n",THIS_MODULE->name,THIS_MODULE->state);

	// 遍历模块列表，查找target模块
	list_for_each_entry(mod,THIS_MODULE->list.prev,list)
	{
		if(strcmp(mod->name,"syscall")==0) {

			// 打印target的模块名、模块状态、引用计数
			printk(KERN_ALERT"name:%s state:%d refcnt:%lu ",mod->name,mod->state,module_refcount(mod));

			// 打印出所有依赖target的模块名
			if(!list_empty(&mod->source_list)) {
				list_for_each_entry(relate,&mod->source_list,source_list)
					printk(KERN_ALERT"%s ",relate->name);
			} else
				printk(KERN_ALERT"used by NULL\n");

			// 把target的引用计数置为0
			//for_each_possible_cpu(cpu)
			//	local_set(__module_ref_addr(mod,cpu),0);
			if(module_refcount(mod) > 0)
				module_put(mod);

			// 再看看target的名称、状态、引用计数
			printk(KERN_ALERT"name:%s state:%d refcnt:%lu\n",mod->name,mod->state,module_refcount(mod));
		}
	}
	return 0;
}

static void __exit mymod_exit(void)
{
	printk(KERN_ALERT"[rmmod mymod] name:%s state:%d\n",THIS_MODULE->name,THIS_MODULE->state);
}

module_init(mymod_init);
module_exit(mymod_exit);
