#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#define __NO_VERSION__

#include "../include/kln.h"

#define KPROBE_PRE_HANDLER(fname) static int __kprobes fname(struct kprobe *p, struct pt_regs *regs)

long unsigned int kallsyms_lookup_name_addr = 0;
unsigned long (*kallsyms_lookup_name_pointer)(const char* name) = NULL;

static struct kprobe kp0, kp1;

KPROBE_PRE_HANDLER(handler_pre0) {
    kallsyms_lookup_name_addr = (--regs->ip);

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
kln_p get_kallsyms_lookup_name_ptr(void) {
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

    pr_info("kallsyms_lookup_name address = 0x%lx\n", kallsyms_lookup_name_addr);

    kallsyms_lookup_name_pointer = (unsigned long (*)(const char* name)) kallsyms_lookup_name_addr;

    return kallsyms_lookup_name_pointer;
}