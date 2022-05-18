#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/ptrace.h>
#include <linux/socket.h>
#include <linux/kallsyms.h>

typedef unsigned long (*kln_p)(const char*);

#include <linux/kprobes.h>

#define KPROBE_PRE_HANDLER(fname) static int __kprobes fname(struct kprobe *p, struct pt_regs *regs)

long unsigned int kln_addr = 0;
unsigned long (*kln_pointer)(const char* name) = NULL;

static struct kprobe kp0, kp1;

KPROBE_PRE_HANDLER(handler_pre0) {
    kln_addr = (--regs->ip);

    return 0;
}

KPROBE_PRE_HANDLER(handler_pre1) {
    return 0;
}

static int do_register_kprobe(struct kprobe* kp, char* symbol_name, void* handler) {
    int ret;

    kp->symbol_name = symbol_name;
    kp->pre_handler = handler;

    ret = register_kprobe(kp);
    if (ret < 0) {
        pr_err("do_register_kprobe: failed to register for symbol %s, returning %d\n", symbol_name, ret);
        return ret;
    }

    pr_info("Planted krpobe for symbol %s at %p\n", symbol_name, kp->addr);

    return ret;
}

// this is the function that I have modified, as the name suggests it returns a pointer to the extracted kallsyms_lookup_name function
kln_p get_kln_p(void) {
    int status;

    status = do_register_kprobe(&kp0, "kallsyms_lookup_name", handler_pre0);

    if (status < 0) return NULL;

    status = do_register_kprobe(&kp1, "kallsyms_lookup_name", handler_pre1);

    if (status < 0) {
        // cleaning initial krpobe
        unregister_kprobe(&kp0);
        return NULL;
    }

    unregister_kprobe(&kp0);
    unregister_kprobe(&kp1);

    pr_info("kallsyms_lookup_name address = 0x%lx\n", kln_addr);

    kln_pointer = (unsigned long (*)(const char* name)) kln_addr;

    return kln_pointer;
}

MODULE_LICENSE("Dual BSD/GPL");

typedef int (* syscall_wrapper)(struct pt_regs *);

unsigned long sys_call_table_addr;

#define SOCKETLOG "[EXECVELOG]"


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
    printk(KERN_INFO SOCKETLOG "execve was called (%s %p %p)",  (const char*)(regs->di), (void *)(regs->si), (void*)(regs->dx));
    return (*original_execve)(regs);
}

static int __init execve_log_init(void) {
    kln_p kln = get_kln_p();

    printk(KERN_INFO SOCKETLOG "proxyexec module has been loaded\n");

    sys_call_table_addr = kln("sys_call_table");

    printk(KERN_INFO SOCKETLOG "sys_call_table %lx\n", sys_call_table_addr);

    enable_page_rw((void *)sys_call_table_addr);
    original_execve = ((syscall_wrapper *)sys_call_table_addr)[__NR_execve];
    if (!original_execve) return -1;
    ((syscall_wrapper *)sys_call_table_addr)[__NR_execve] = log_execve;
    disable_page_rw((void *)sys_call_table_addr);

    printk(KERN_INFO SOCKETLOG "original_execve = %p", original_execve);
    return 0;
}

static void __exit execve_log_exit(void) {
    printk(KERN_INFO SOCKETLOG "proxyexec module has been unloaded\n");

    enable_page_rw((void *)sys_call_table_addr);
    ((syscall_wrapper *)sys_call_table_addr)[__NR_execve] = original_execve;
    disable_page_rw((void *)sys_call_table_addr);
}

module_init(execve_log_init);
module_exit(execve_log_exit);