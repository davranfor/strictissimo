/*!
 *  \brief     strictissimo - Ligtweight json and json-schema library for C
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "json_struct.h"
#include "json_macros.h"

static size_t string_size(const char *str)
{
    const char *ptr = str;

    while (json_valid_char(*str))
    {
        str++;
    }
    return (size_t)(*str == '\0' ? str - ptr + 1 : 0);
}

static char *string_format(const char *fmt, va_list args)
{
    va_list copy;

    va_copy(copy, args);

    size_t len = (size_t)vsnprintf(NULL, 0, fmt, args);
    char *str = malloc(len + 1);

    if (str != NULL)
    {
        vsprintf(str, fmt, copy);
        if (string_size(str) == 0)
        {
            free(str);
            str = NULL;
        }
    }
    va_end(copy);
    return str;
}

static char *copy_string(const char *str)
{
    size_t size = string_size(str);
    char *ptr = NULL;

    if ((size > 0) && (ptr = malloc(size)))
    {
        memcpy(ptr, str, size);
    }
    return ptr;
}

static char *copy_integer(long long number)
{
    size_t size = 1 + (size_t)snprintf(NULL, 0, "%lld", number);
    char *str = malloc(size);

    if (str != NULL)
    {
        snprintf(str, size, "%lld", number);
    }
    return str;
}

static char *copy_real(unsigned long long number)
{
    size_t size = 1 + (size_t)snprintf(NULL, 0, "%llu", number);
    char *str = malloc(size);

    if (str != NULL)
    {
        snprintf(str, size, "%llu", number);
    }
    return str;
}

static char *copy_double(double number, int decimals)
{
    size_t size = 1 + (size_t)snprintf(NULL, 0, "%.*f", decimals, number);
    char *str = malloc(size);

    if (str != NULL)
    {
        snprintf(str, size, "%.*f", decimals, number);
    }
    return str;
}

static json *new_type(enum json_type type, const char *key, char *value)
{
    if ((value == NULL) && (type != JSON_OBJECT) && (type != JSON_ARRAY))
    {
        return NULL;
    }

    char *name = NULL;

    if ((key != NULL) && !(name = copy_string(key)))
    {
        free(value);
        return NULL;
    }

    json *node = calloc(1, sizeof *node);

    if (node != NULL)
    {
        node->type = type;
        node->name = name;
        node->value = value;
    }
    else
    {
        free(name);
        free(value);
    }
    return node;
}

json *json_new_object(const char *name)
{
    return new_type(JSON_OBJECT, name, NULL);
}

json *json_new_array(const char *name)
{
    return new_type(JSON_ARRAY, name, NULL);
}

json *json_new_format(const char *name, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);

    char *str = string_format(fmt, args);

    va_end(args);
    if (str == NULL)
    {
        return NULL;
    }
    return new_type(JSON_STRING, name, str);
}

json *json_new_string(const char *name, const char *value)
{
    char *str = copy_string(value);

    if (str == NULL)
    {
        return NULL;
    }
    return new_type(JSON_STRING, name, str);
}

json *json_new_integer(const char *name, long long value)
{
    char *str = copy_integer(value);

    if (str == NULL)
    {
        return NULL;
    }
    return new_type(JSON_INTEGER, name, str);
}

json *json_new_real(const char *name, unsigned long long value)
{
    char *str = copy_real(value);

    if (str == NULL)
    {
        return NULL;
    }
    return new_type(JSON_INTEGER, name, str);
}

json *json_new_double(const char *name, double value, int decimals)
{
    char *str = copy_double(value, decimals);

    if (str == NULL)
    {
        return NULL;
    }

    enum json_type type = decimals ? JSON_DOUBLE : JSON_INTEGER;

    return new_type(type, name, str);
}

json *json_new_boolean(const char *name, int value)
{
    char *str = copy_string(value ? "true" : "false");

    if (str == NULL)
    {
        return NULL;
    }
    return new_type(JSON_BOOLEAN, name, str);
}

json *json_new_null(const char *name)
{
    char *str = copy_string("null");

    if (str == NULL)
    {
        return NULL;
    }
    return new_type(JSON_NULL, name, str);
}

const char *json_set_name(json *node, const char *name)
{
    if (node == NULL)
    {
        return NULL;
    }
    if ((node->parent != NULL) && (!node->name & !name))
    {
        return NULL;
    }

    char *str = NULL;

    if ((name != NULL) && !(str = copy_string(name)))
    {
        return NULL;
    }
    free(node->name);
    node->name = str;
    return str;
}

/* set helper */
static const char *set_value(json *node, enum json_type type, char *value)
{
    if (value != NULL)
    {
        node->type = type;
        free(node->value);
        node->value = value;
    }
    return value;
}

const char *json_set_string(json *node, const char *value)
{
    if ((node == NULL) || (node->value == NULL))
    {
        return NULL;
    }
    return set_value(node, JSON_STRING, copy_string(value));
}

const char *json_set_integer(json *node, long long value)
{
    if ((node == NULL) || (node->value == NULL))
    {
        return NULL;
    }
    return set_value(node, JSON_INTEGER, copy_integer(value));
}

const char *json_set_real(json *node, unsigned long long value)
{
    if ((node == NULL) || (node->value == NULL))
    {
        return NULL;
    }
    return set_value(node, JSON_INTEGER, copy_real(value));
}

const char *json_set_double(json *node, double value, int decimals)
{
    if ((node == NULL) || (node->value == NULL))
    {
        return NULL;
    }

    enum json_type type = decimals ? JSON_DOUBLE : JSON_INTEGER;

    return set_value(node, type, copy_double(value, decimals));
}

const char *json_set_boolean(json *node, int value)
{
    if ((node == NULL) || (node->value == NULL))
    {
        return NULL;
    }
    return set_value(node, JSON_BOOLEAN, copy_string(value ? "true" : "false"));
}

const char *json_set_null(json *node)
{
    if ((node == NULL) || (node->value == NULL))
    {
        return NULL;
    }
    return set_value(node, JSON_NULL, copy_string("null"));
}

/* push helper */
static int not_pushable(const json *parent, const json *child)
{
    if ((parent == NULL) || (parent->value != NULL)
      || (child == NULL) || (child->parent != NULL))
    {
        return 1;
    }
    // parent being object and child without name
    // or
    // parent being array and child with name 
    return ((parent->type == JSON_OBJECT) ^ (child->name != NULL));
}

json *json_push_fast(json *parent, json *where, json *child)
{
    if (where == NULL)
    {
        return json_push_back(parent, child);
    }
    else
    {
        return json_push_after(where, child);
    }
}

json *json_push_front(json *parent, json *child)
{
    if (not_pushable(parent, child))
    {
        return NULL;
    }
    if (parent->child != NULL)
    {
        child->next = parent->child;
        parent->child->prev = child;
    }
    child->parent = parent;
    parent->child = child;
    return child;
}

json *json_push_back(json *parent, json *child)
{
    if (not_pushable(parent, child))
    {
        return NULL;
    }
    if (parent->child == NULL)
    {
        parent->child = child;
    }
    else
    {
        json *node = parent->child;

        while (node->next != NULL)
        {
            node = node->next;
        }
        node->next = child;
        child->prev = node;
    }
    child->parent = parent;
    return child;
}

json *json_push_before(json *where, json *child)
{
    json *parent = json_parent(where);

    if (not_pushable(parent, child))
    {
        return NULL;
    }
    if (parent->child == where)
    {
        parent->child = child;
    }
    else
    {
        child->prev = where->prev;
        where->prev->next = child;
    }
    child->parent = parent;
    child->next = where;
    where->prev = child;
    return child;
}

json *json_push_after(json *where, json *child)
{
    json *parent = json_parent(where);

    if (not_pushable(parent, child))
    {
        return NULL;
    }
    if (where->next != NULL)
    {
        child->next = where->next;
        where->next->prev = child;
    }
    child->parent = parent;
    child->prev = where;
    where->next = child;
    return child;
}

json *json_push_at(json *parent, json *child, size_t item)
{
    if (not_pushable(parent, child))
    {
        return NULL;
    }
    if (parent->child == NULL)
    {
        parent->child = child;
    }
    else
    {
        json *node = parent->child;

        while ((item > 0) && (node->next != NULL))
        {
            node = node->next;
            item--;
        }
        if (parent->child == node)
        {
            parent->child = child;
        }
        if (node->prev != NULL)
        {
            child->prev = node->prev;
            node->prev->next = child;
        }
        child->next = node;
        node->prev = child;
    }
    child->parent = parent;
    return child;
}

json *json_pop(json *child)
{
    json *parent = json_parent(child);

    if (parent == NULL)
    {
        return NULL;
    }
    if (parent->child == child)
    {
        parent->child = child->next;
    }
    else
    {
        child->prev->next = child->next;
    }
    if (child->next != NULL)
    {
        child->next->prev = child->prev;
    }
    child->parent = NULL;
    child->prev = NULL;
    child->next = NULL; 
    return child;
}

json *json_pop_front(json *parent)
{
    json *child = json_child(parent);

    if (child == NULL)
    {
        return NULL;
    }
    parent->child = child->next;
    if (child->next != NULL)
    {

        child->next->prev = NULL;
        child->next = NULL;
    }
    child->parent = NULL;
    return child;
}

json *json_pop_back(json *parent)
{
    json *child = json_child(parent);

    if (child == NULL)
    {
        return NULL;
    }
    while (child->next != NULL)
    {
        child = child->next;
    }
    if (child->prev != NULL)
    {
        child->prev->next = NULL;
        child->prev = NULL;
    }
    else
    {
        parent->child = NULL;
    }
    child->parent = NULL;
    return child;
}

json *json_pop_at(json *parent, size_t item)
{
    json *child = json_child(parent);

    if (child == NULL)
    {
        return NULL;
    }
    while ((item > 0) && (child != NULL))
    {
        child = child->next;
        item--;
    }
    if (child == NULL)
    {
        return NULL;
    }
    if (parent->child == child)
    {
        parent->child = child->next;
    }
    if (child->prev != NULL)
    {
        child->prev->next = child->next;
    }
    if (child->next != NULL)
    {
        child->next->prev = child->prev;
    }
    child->parent = NULL;
    child->prev = NULL;
    child->next = NULL;
    return child;
}

void json_free(json *node)
{
    json *parent = node ? node->parent : NULL;
    json *next;

    while (node != parent)
    {
        next = node->child;
        node->child = NULL;
        if (next == NULL)
        {
            if (node->next != NULL)
            {
                next = node->next;
            }
            else
            {
                next = node->parent;
            }
            free(node->name);
            free(node->value);
            free(node);
        }
        node = next;
    }
}

