#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "hash_table.h"


int main() {
    proxy_addr_store_t store;

    ssize_t status = proxy_addr_store_init(&store, 9);
    if (status < 0)
    {
        return -1;
    }

    ssize_t index = proxy_addr_store_insert("asdas", &store);
    printf("%s\n", store.table[index]->key);
    index = proxy_addr_store_insert("asdasadass", &store);
    printf("%s\n", store.table[index]->key);
    index = proxy_addr_store_insert("asdas", &store);

    index = proxy_addr_store_search("asdas", &store);
    return 0;
}