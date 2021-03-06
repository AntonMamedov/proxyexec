#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/ptrace.h>
#include <linux/kallsyms.h>

#include "../include/kln.h"

MODULE_LICENSE("Dual BSD/GPL");

typedef int (* syscall_wrapper)(struct pt_regs *);

unsigned long sys_call_table_addr;

#define EXECVELOG "[EXECVELOG]"


int enable_page_rw(void *ptr){
    unsigned int level;
    pte_t *pte = lookup_address((unsigned long) ptr, &level);
    if(pte->pte &~_PAGE_RW){
        pte->pte |=_PAGE_RW;
    }
    return 0;
}

int disable_page_rw(void *ptr){
    unsigned int level;
    pte_t *pte = lookup_address((unsigned long) ptr, &level);
    pte->pte = pte->pte &~_PAGE_RW;
    return 0;
}

syscall_wrapper original_execve;

int log_execve(struct pt_regs *regs) {
    printk(KERN_INFO EXECVELOG "execve was called (%s %p %p)",  (const char*)(regs->di), (void *)(regs->si), (void*)(regs->dx));
    return (*original_execve)(regs);
}

static int __init execve_log_init(void) {
    kln_p kln = get_kallsyms_lookup_name_ptr();

    printk(KERN_INFO EXECVELOG "proxyexec module has been loaded\n");

    sys_call_table_addr = kln("sys_call_table");

    printk(KERN_INFO EXECVELOG "sys_call_table %lx\n", sys_call_table_addr);

    enable_page_rw((void *)sys_call_table_addr);
    original_execve = ((syscall_wrapper *)sys_call_table_addr)[__NR_execve];
    if (!original_execve) return -1;
    ((syscall_wrapper *)sys_call_table_addr)[__NR_execve] = log_execve;
    disable_page_rw((void *)sys_call_table_addr);

    printk(KERN_INFO EXECVELOG "original_execve = %p", original_execve);
    return 0;
}

static void __exit execve_log_exit(void) {
    printk(KERN_INFO EXECVELOG "proxyexec module has been unloaded\n");

    enable_page_rw((void *)sys_call_table_addr);
    ((syscall_wrapper *)sys_call_table_addr)[__NR_execve] = original_execve;
    disable_page_rw((void *)sys_call_table_addr);
}

module_init(execve_log_init);
module_exit(execve_log_exit);