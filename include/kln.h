#ifndef PROXYEXEC_KLN_H
#define PROXYEXEC_KLN_H

typedef unsigned long (*kln_p)(const char*);
kln_p get_kln_pointer(void);

#endif //PROXYEXEC_KLN_H
