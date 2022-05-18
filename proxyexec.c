#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kallsyms.h>
#include <linux/fs.h>
#include <linux/buffer_head.h>
#include <asm/segment.h>
#include <asm/uaccess.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Mamedov A.B.");
MODULE_VERSION("0.01");

//typedef int (*syscall_wrapper)(struct pt_regs *);

void **sys_call_table_addr = (void **)0xfffffffface00300;

int (*original_execve)(const char*, char *const[], char *const[]);

static int my_execve(const char* pathname, char *const argv[], char *const envp[])
{
    printk(KERN_INFO "%s\n", pathname);
    return original_execve(pathname, argv, envp);
}

static int enable_page_rw(void *ptr)
{
    unsigned int level;
    pte_t *pte = lookup_address((unsigned long) ptr, &level);
    if(pte->pte &~_PAGE_RW){
        pte->pte |=_PAGE_RW;
    }
    return 0;
}

static int disable_page_rw(void *ptr)
{
    unsigned int level;
    pte_t *pte = lookup_address((unsigned long) ptr, &level);
    pte->pte = pte->pte &~_PAGE_RW;
    return 0;
}


//syscall_wrapper original_exec;

static int __init proxyexec_init(void)
{
      enable_page_rw(sys_call_table_addr);
      original_execve = sys_call_table_addr[__NR_execve];
      if (!original_execve)
      {
          printk(KERN_INFO "NULL\n");
      }
      else {
          printk(KERN_INFO "exec: %p\n", original_execve);
          sys_call_table_addr[__NR_execve] = my_execve;
          disable_page_rw(sys_call_table_addr);
      }
    return 0;
}
//
static void __exit proxyexec_exit(void)
{
    enable_page_rw(sys_call_table_addr);
    sys_call_table_addr[__NR_execve] = original_execve;
    disable_page_rw(sys_call_table_addr);
}

module_init(proxyexec_init);
module_exit(proxyexec_exit);


