/*!
 *  \brief     strictissimo - Ligtweight json and json-schema library for C
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include <string.h>
#include "json_map.h"

#define hash_name(name) hash_str((const unsigned char *)(name))

struct node
{
    struct node *next;
    json *data;
    char name[];
};

struct json_map
{
    struct node **list;
    json_map *next;
    size_t room;
    size_t size;
};

static const size_t primes[] =
{
    53,        97,         193,        389,
    769,       1543,       3079,       6151,
    12289,     24593,      49157,      98317, 
    196613,    393241,     786433,     1572869,
    3145739,   6291469,    12582917,   25165843, 
    50331653,  100663319,  201326611,  402653189,
    805306457, 1610612741, 3221225473, 4294967291
};

static unsigned long hash_str(const unsigned char * key)
{
    unsigned long hash = 5381;
    unsigned char chr;

    while ((chr = *key++))
    {
        hash = ((hash << 5) + hash) + chr;
    }
    return hash;
}

json_map *json_map_create(size_t size)
{
    enum {NPRIMES = sizeof primes / sizeof *primes};

    for (size_t iter = 0; iter < NPRIMES; iter++)
    {
        if (size < primes[iter])
        {
            size = primes[iter];
            break;
        }
    }

    json_map *map = calloc(1, sizeof *map);

    if (map != NULL)
    {
        map->list = calloc(size, sizeof *map->list);
        if (map->list == NULL)
        {
            free(map);
            return NULL;
        }
        map->room = size;
    }
    return map;
}

static struct node *create_node(const char *name, json *data)
{
    size_t size = strlen(name) + 1;
    struct node *node = malloc(sizeof *node + size);

    if (node != NULL)
    {
        node->next = NULL;
        node->data = data;
        memcpy(node->name, name, size);
    }
    return node;
}

static void reset(json_map *map)
{
    json_map *next = map->next;

    free(map->list);
    map->list = next->list;
    map->room = next->room;
    map->size = next->size;
    map->next = next->next;
    free(next);
}

static void move(json_map *map, struct node *node)
{
    unsigned long hash = hash_name(node->name);
    struct node **list = map->list + hash % map->room;

    node->next = *list;
    *list = node;
    map->size++;
}

static json_map *rehash(json_map *map, unsigned long hash)
{
    while (map->next != NULL)
    {
        struct node **list = map->list + hash % map->room;
        struct node *node = *list;

        *list = NULL;
        while (node != NULL)
        {
            struct node *next = node->next;

            move(map->next, node);
            map->size--;
            node = next;
        }
        if (map->size == 0)
        {
            reset(map);
        }
        else
        {
            map = map->next;
        }
    }
    return map;
}

json *json_map_insert(json_map *map, const char *name, json *data)
{
    if (map != NULL)
    {
        unsigned long hash = hash_name(name);

        map = rehash(map, hash);

        struct node **list = map->list + hash % map->room;
        struct node *node;

        while ((node = *list))
        {
            if (!strcmp(node->name, name))
            {
                return node->data;
            }
            list = &(node->next);
        }
        *list = create_node(name, data);
        if (*list == NULL)
        {
            return NULL;
        }
        // If more than 75% occupied then create a new table
        if (++map->size > map->room - map->room / 4)
        {
            map->next = json_map_create(map->room);
            if (map->next == NULL)
            {
                return NULL;
            }
        }
        return data;
    }
    return NULL;
}

json *json_map_delete(json_map *map, const char *name)
{
    unsigned long hash = hash_name(name);

    while (map != NULL)
    {
        struct node **list = map->list + hash % map->room;
        struct node *node = *list, *prev = NULL;

        if (node != NULL) do
        {
            if (!strcmp(node->name, name))
            {
                json *data = node->data;

                if (prev != NULL)
                {
                    prev->next = node->next;
                }
                else
                {
                    *list = node->next;
                }
                free(node);
                if ((--map->size == 0) && (map->next != NULL))
                {
                    reset(map);
                }
                return data;
            }
            else
            {
                prev = node;
                node = node->next;
            }
        } while (node != NULL);

        // Not found in this table, try in the next one
        map = map->next;
    }
    return NULL;
}

json *json_map_search(const json_map *map, const char *name)
{
    unsigned long hash = hash_name(name);

    while (map != NULL)
    {
        const struct node *node = map->list[hash % map->room];

        if (node != NULL) do
        {
            if (!strcmp(node->name, name))
            {
                return node->data;
            }
            node = node->next;
        } while (node != NULL);

        // Not found in this table, try in the next one
        map = map->next;
    }
    return NULL;
}

size_t json_map_size(const json_map *map)
{
    size_t size = 0;
    
    while (map != NULL)
    {
        size += map->size;
        map = map->next;
    }
    return size;
}

void json_map_destroy(json_map *map, void (*function)(json *))
{
    while (map != NULL)
    {
        for (size_t index = 0; map->size > 0; index++)
        {
            struct node *node = map->list[index];

            if (node != NULL) do
            {
                struct node *next = node->next;

                if (function != NULL)
                {
                    function(node->data);
                }
                free(node);
                node = next;
                map->size--;
            } while (node != NULL);
        }

        json_map *next = map->next;

        free(map->list);
        free(map);
        map = next;
    }
}

