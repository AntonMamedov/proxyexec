#include "hash_table.h"

#include <string.h>

#define LOAD_FACTOR 4



ssize_t
proxy_addr_store_init (proxy_addr_store_t *dest, size_t size)
{
    dest->table
            = malloc (sizeof (proxy_addr_stroe_node_t *) * size);
    if (dest->table == NULL)
        return -1;
    dest->size = size;
    dest->size = dest->size % 2 == 0 ? dest->size + 1 : dest->size;
    dest->current_size = 0;
    size_t i;
    for (i = 0; i < dest->size; i++)
        dest->table[i] = NULL;
    return (ssize_t)dest->size;
}

static void
proxy_addr_store_node_release (proxy_addr_stroe_node_t *dst)
{
    free (dst);
    free (dst);
}

void
proxy_addr_store_release (proxy_addr_store_t *dst)
{
    size_t i;
    for (i = 0; i < dst->size; i++)
    {
        if (dst->table[i] != NULL)
            proxy_addr_store_node_release (dst->table[i]);
    }
    free (dst->table);
}

static size_t
hash_impl (const char *str, size_t table_size, size_t key, size_t str_len)
{
    unsigned long hash_result = 0;
    size_t i;
    for (i = 0; i < str_len; i++)
        hash_result = (key * hash_result + str[i]);
    hash_result = (hash_result * 2 + 1) % table_size;
    return hash_result;
}

static size_t
hash1 (const char *str, size_t table_size, size_t str_len)
{
    return hash_impl (str, table_size, table_size + 1, str_len);
}

static size_t
hash2 (const char *str, size_t table_size, size_t str_len)
{
    return hash_impl (str, table_size, table_size - 1, str_len);
}

static int
proxy_addr_store_fill (proxy_addr_stroe_node_t *node, const char *str,
                       size_t str_len)
{
    node->key = malloc (sizeof (char) * str_len + 1);
    if (node->key == NULL)
        return -1;
    strcpy (node->key, str);
    node->state = TAKEN;
    return 0;
}

static proxy_addr_stroe_node_t *
new_proxy_addr_store_node (const char *str, size_t str_len)
{
    proxy_addr_stroe_node_t *node
            = malloc (sizeof (proxy_addr_stroe_node_t));
    if (node == NULL)
        return NULL;
    if (proxy_addr_store_fill (node, str, str_len) < 0)
    {
        free (node);
        return NULL;
    }
    return node;
}

static size_t
proxy_addr_store_get_index (const char *key, size_t size, size_t str_len,
                            size_t *h1, size_t i)
{
    size_t index = 0;
    if (i == 0)
        index = *h1 = hash1 (key, size, str_len);
    else
        index = *h1 + (hash2 (key, size, str_len) * i) % size;

    return index;
}

static ssize_t
proxy_addr_store_insert_impl (const char *key, proxy_addr_stroe_node_t **table,
                              size_t size)
{
    size_t str_len = strlen (key);

    size_t i;
    size_t h1;
    for (i = 0; i < size; i++)
    {
        size_t index = proxy_addr_store_get_index (key, size, str_len, &h1, i);
        store_node_state_t state
                = table[index] == NULL ? EMPTY : table[index]->state;
        switch (state)
        {
            case EMPTY:
                table[index] = new_proxy_addr_store_node (key, str_len);
                if (table[index] == NULL)
                    return -1;
                return (ssize_t)index;
            case DELETED:
                proxy_addr_store_fill (table[index], key, str_len);
                if (proxy_addr_store_fill (table[index], key, str_len) < 0)
                    return -1;
                return (ssize_t)index;
            case TAKEN:
                break;
        }
    }

    return -1;
}

static ssize_t
proxy_addr_store_extend (proxy_addr_store_t *dest)
{
    size_t new_size = dest->size * 2;
    proxy_addr_stroe_node_t **new_table
            = malloc (sizeof (proxy_addr_stroe_node_t *) * new_size);
    if (new_table == NULL)
        return -1;

    size_t i;
    for (i = 0; i < new_size; i++)
    {
        new_table[i] = NULL;
    }

    for (i = 0; i < dest->size; i++)
    {
        store_node_state_t state
                = dest->table[i] == NULL ? EMPTY : dest->table[i]->state;
        switch (state)
        {
            case EMPTY:
                break;
            case DELETED:
                proxy_addr_store_node_release (dest->table[i]);
                break;
            case TAKEN:
            {
                size_t h1;
                ssize_t j;
                for (j = 0; j < new_size;)
                {
                    size_t index = proxy_addr_store_get_index (
                            dest->table[i]->key, new_size,
                            strlen (dest->table[i]->key), &h1, j);
                    state = new_table[index] == NULL ? EMPTY
                                                     : new_table[index]->state;
                    switch (state)
                    {
                        case EMPTY:
                            new_table[index] = dest->table[i];
                            j = -1;
                            break;
                        case DELETED:
                        {
                            proxy_addr_store_t tmp = { new_table, new_size, 0 };
                            proxy_addr_store_release (&tmp);
                            return -1;
                        }
                        case TAKEN:
                            break;
                    }
                    if (j < 0)
                        break;
                }
                break;
            }
        }
    }
    free (dest->table);
    dest->table = new_table;
    dest->size = new_size;
    return (ssize_t)new_size;
}

ssize_t
proxy_addr_store_insert (const char *str, proxy_addr_store_t *dst)
{
    if (dst->current_size >= dst->size / LOAD_FACTOR)
    {
        if (proxy_addr_store_extend (dst) < 0)
            return -1;
    }
    dst->current_size++;
    return proxy_addr_store_insert_impl (str, dst->table, dst->size);
}

ssize_t
proxy_addr_store_search (const char *key, const proxy_addr_store_t *store)
{
    size_t str_len = strlen (key);
    size_t i;
    size_t h1;
    for (i = 0; i < store->size;)
    {
        size_t index
                = proxy_addr_store_get_index (key, store->size, str_len, &h1, i);
        store_node_state_t state
                = store->table[index] == NULL ? EMPTY : store->table[index]->state;
        switch (state)
        {
            case EMPTY:
                return -1;
            case DELETED:
                break;
            case TAKEN:
                if (strcmp (store->table[index]->key, key) != 0)
                    break;
                return (ssize_t)index;
        }
    }
    return -1;
}

void
proxy_addr_store_delete (proxy_addr_store_t *dst, const char *key)
{
    size_t index = proxy_addr_store_search (key, dst);
    if (index < 0)
        return;
    dst->table[index]->state = DELETED;
    free (dst->table[index]->key);
}