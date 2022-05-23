#ifndef LINUX_KERNEL_MODULE_WITH_CLION_IDE_SUPPORT_CMAKE_HASH_TABLE_H
#define LINUX_KERNEL_MODULE_WITH_CLION_IDE_SUPPORT_CMAKE_HASH_TABLE_H
#include <stdint.h>
#include <stdlib.h>

typedef enum {
    EMPTY = 0,
    DELETED,
    TAKEN
} store_node_state_t;

struct proxy_addr_store_node
{
    char* key;
    store_node_state_t state;
};

typedef struct proxy_addr_store_node proxy_addr_stroe_node_t;
typedef struct proxy_addr_store
{
    proxy_addr_stroe_node_t** table;
    size_t size;
    size_t current_size;
} proxy_addr_store_t;

ssize_t proxy_addr_store_init(proxy_addr_store_t* dst, size_t size);
ssize_t proxy_addr_store_insert(const char* str, proxy_addr_store_t* dst);
ssize_t proxy_addr_store_search(const char* key, const proxy_addr_store_t* store);
void proxy_addr_store_release(proxy_addr_store_t* dst);
void proxy_addr_store_delete(proxy_addr_store_t* dst, const char* key);

#endif //LINUX_KERNEL_MODULE_WITH_CLION_IDE_SUPPORT_CMAKE_HASH_TABLE_H
