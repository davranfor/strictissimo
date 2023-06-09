/*!
 *  \brief     strictissimo - Ligtweight json and json-schema library for C
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "json_struct.h"
#include "json_macros.h"

/* Returns the type of an iterable by token */
static enum json_type token_type(int token)
{
    switch (token)
    {
        default : return JSON_UNDEFINED;
        case '{':
        case '}': return JSON_OBJECT;
        case '[':
        case ']': return JSON_ARRAY;
    }
}

/* Check whether a character is a token */
static int is_token(int c)
{
    return (c == '{') || (c == '}')
        || (c == '[') || (c == ']')
        || (c == ':') || (c == ',')
        || (c == '\0');
}

static int is_escape(const char *str)
{
    char c = *str;

    return (c == '\\') || (c == '/') || (c == '"') ||
           (c == 'b')  || (c == 'f') || (c == 'n') || (c == 'r') || (c == 't');
}

static int is_unicode(const char *str)
{
    return ((*str++) == 'u')
        && is_xdigit(*str++)
        && is_xdigit(*str++)
        && is_xdigit(*str++)
        && is_xdigit(*str);
}

/*
 * Converts Unicode to multibyte
 * Returns the length of the multibyte in bytes
 */
static int unicode_to_mb(const char *str, char *buf)
{
    char hex[5] = "";

    /* Copy UCN skipping the initial "u" */
    memcpy(hex, str + 1, 4);

    unsigned codepoint = (unsigned)strtoul(hex, NULL, 16);

    /* Copy "as is" if is not a valid json character */
    if (!is_char(codepoint))
    {
        memcpy(buf, str - 1, 6);
        return 6;
    }
    /* Convert to multibyte and return the length */
    if (codepoint <= 0x7f)
    {
        buf[0] = (char)codepoint;
        return 1;
    }
    else if (codepoint <= 0x7ff)
    {
        buf[0] = (char)(0xc0 | ((codepoint >> 6) & 0x1f));
        buf[1] = (char)(0x80 |  (codepoint & 0x3f));
        return 2;
    }
    else // if (codepoint <= 0xffff)
    {
        buf[0] = (char)(0xe0 | ((codepoint >> 12) & 0x0f));
        buf[1] = (char)(0x80 | ((codepoint >> 6) & 0x3f));
        buf[2] = (char)(0x80 |  (codepoint & 0x3f));
        return 3;
    }
}

/* scan() helpers */

static const char *scan_quoted(const char *str)
{
    while (!is_cntrl(*str))
    {
        if (*str == '"')
        {
            break;
        }
        if (*str == '\\')
        {
            if (is_escape(str + 1))
            {
                str += 2;
                continue;
            }
            if (is_unicode(str + 1))
            {
                str += 6;
                continue;
            }
            break;
        }
        str++;
    }
    return str;
}

static const char *scan_unquoted(const char *str)
{
    while (!is_space(*str) && !is_token(*str))
    {
        if (*str == '"')
        {
            break;
        }
        str++;
    }
    return str;
}

/* Returns a pointer to the next element or NULL on fail */
static const char *scan(const char **left, const char **right)
{
    const char *str = *left;

    /* Skip leading spaces */
    while (is_space(*str))
    {
        str++;
    }
    /* Adjust pointers to token */
    *left = *right = str;
    /* Return on first token */
    if (is_token(*str))
    {
        return str;
    }
    /* Handle name or string scalar */
    if (*str == '"')
    {
        str = scan_quoted(str + 1);
        if (*str != '"')
        {
            goto fail;
        }
        *right = str++;
    }
    else // ... handle other scalars
    {
        str = scan_unquoted(str + 1);
        if (*str == '"')
        {
            goto fail;
        }
        *right = str - 1;
    }
    /* Skip trailing spaces */
    while (is_space(*str))
    {
        str++;
    }
    /* Unexpected character */
    if (!is_token(*str))
    {
        goto fail;
    }
    /* Valid */
    return str;
fail:
    /* Adjust pointers to error */
    *left = *right = str;
    return NULL;
}

/* Allocates space for a name or a string value escaping special characters */
static char *copy(const char *str, size_t length)
{
    char *buf = malloc(length + 1);

    if (buf == NULL)
    {
        return NULL;
    }

    const char *end = str + length;
    char *ptr = buf;

    while (str < end)
    {
        if (*str == '\\')
        {
            switch (*++str)
            {
                default : *ptr++ = *str; break;
                case 'b': *ptr++ = '\b'; break;
                case 'f': *ptr++ = '\f'; break;
                case 'n': *ptr++ = '\n'; break;
                case 'r': *ptr++ = '\r'; break;
                case 't': *ptr++ = '\t'; break;
                case 'u':
                    ptr += unicode_to_mb(str, ptr);
                    str += 4;
                    break;
            }
        }
        else
        {
            *ptr++ = *str;
        }
        str++;
    }
    *ptr = '\0';
    return buf;
}

/* Convert and check whether a string is a valid number */
static double to_number(const char *left, const char *right, int *error)
{
    char *end;
    double number = strtod(left, &end);

    if (end <= right)
    {
        *error = 1;
        return 0;
    }
    /* Skip sign */
    if (left[0] == '-')
    {
        left++;
    }
    /* Do not allow padding 0s */
    if ((left[0] == '0') && is_digit(left[1]))
    {
        *error = 1;
        return 0;
    }
    /* Must start and end with a digit */ 
    if (!is_digit(*left) || !is_digit(*right))
    {
        *error = 1;
        return 0;
    }
    *error = 0;
    return number;
}

/* Check whether a string already tested as a valid number is a double */
static int is_double(const char *left, const char *right)
{
    while (left <= right)
    {
        if ((*left == '.') || (*left == 'E') || (*left == 'e'))
        {
            return 1;
        }
        left++;
    }
    return 0;
}

static char *set_name(json *node, const char *left, const char *right)
{
    size_t length = (size_t)(right - left + 1);

    /* Must start and end with quotes */
    if ((*left != '"') || (*right != '"'))
    {
        return NULL;
    }
    /* Allocate memory skipping quotes */
    node->name = copy(left + 1, length - 2);
    return node->name;
}

static int set_value(json *node, const char *left, const char *right)
{
    size_t length = (size_t)(right - left + 1);
    int error = 0;

    if ((*left == '"') && (*right == '"'))
    {
        node->type = JSON_STRING;
        if (!(node->value.string = copy(left + 1, length - 2)))
        {
            error = 1;
        }
    }
    else if ((length == 4) && (strncmp(left, "null", length) == 0))
    {
        node->type = JSON_NULL;
        node->value.number = 0;
    }
    else if ((length == 4) && (strncmp(left, "true", length) == 0))
    {
        node->type = JSON_BOOLEAN;
        node->value.number = 1;
    }
    else if ((length == 5) && (strncmp(left, "false", length) == 0))
    {
        node->type = JSON_BOOLEAN;
        node->value.number = 0;
    }
    else
    {
        double value = to_number(left, right, &error);

        if (!error)
        {
            node->type = is_double(left, right) ? JSON_DOUBLE : JSON_INTEGER;
            node->value.number = value;
        }
    }
    return !error;
}

static json *create_node(void)
{
    return calloc(1, sizeof(struct json));
}

/* parse() helpers - node must exist */

static json *create_child(json *parent)
{
    json *child = calloc(1, sizeof(struct json));

    if (child != NULL)
    {
        child->parent = parent;
        parent->child = child;
    }
    return child;
}

static json *delete_child(json *parent)
{
    free(parent->child);
    parent->child = NULL;
    return parent;
}

static json *create_next(json *node)
{
    json *next = calloc(1, sizeof(struct json));

    if (next != NULL)
    {
        next->parent = node->parent;
        next->prev = node;
        node->next = next;
    }
    return next;
}

/* Parse document - returns an error position or NULL on success */
static const char *parse(json *node, const char *left)
{
    const char *right = NULL;
    const char *token;

    while (node != NULL)
    {
        if (!(token = scan(&left, &right)))
        {
            return left;
        }
        switch (*token)
        {
            case '{':
            case '[':
                /* Commas between groups are required: [[] []] */
                if (node->type != JSON_UNDEFINED)
                {
                    return left;
                }
                /* Contents before groups are not allowed: 1[] */
                if (left != token)
                {
                    return left;
                }
                /* Object properties must have a name */
                if (json_is_object(node->parent) && (node->name == NULL))
                {
                    return token;
                }
                node->type = token_type(*token);
                node = create_child(node);
                break;
            case ':':
                if (left == token)
                {
                    return left;
                }
                /* Only object properties can have a name */
                if (!json_is_object(node->parent) || (node->name != NULL))
                {
                    return token;
                }
                if (!set_name(node, left, right))
                {
                    return left;
                }
                break;
            case ',':
                if (node->parent == NULL)
                {
                    return token;
                }
                if (node->type == JSON_UNDEFINED)
                {
                    if (left == token)
                    {
                        return left;
                    }
                    if (json_is_object(node->parent) && (node->name == NULL))
                    {
                        return left;
                    }
                    if (!set_value(node, left, right))
                    {
                        return left;
                    }
                }
                else
                {
                    if (left != token)
                    {
                        return left;
                    }
                }
                node = create_next(node);
                break;
            case ']':
            case '}':
                if (json_type(node->parent) != token_type(*token))
                {
                    return token;
                }
                if (node->type == JSON_UNDEFINED)
                {
                    if (left == token)
                    {
                        /* Remove empty group: {} or [] */
                        if ((node->prev == NULL) && (node->name == NULL))
                        {
                            node = delete_child(node->parent);
                            break;
                        }
                        return left;
                    }
                    if (json_is_object(node->parent) && (node->name == NULL))
                    {
                        return left;
                    }
                    if (!set_value(node, left, right))
                    {
                        return left;
                    }
                }
                else
                {
                    if (left != token)
                    {
                        return left;
                    }
                }
                node = node->parent;
                break;
            case '\0':
                /* Bad closed document */
                if (node->parent != NULL)
                {
                    return left;
                }
                if (node->type == JSON_UNDEFINED)
                {
                    if (left == token)
                    {
                        return left;
                    }
                    if (!set_value(node, left, right))
                    {
                        return left;
                    }
                }
                else
                {
                    if (left != token)
                    {
                        return left;
                    }
                }
                /* Correct document */
                return NULL;
        }
        /* Keep going ... */
        left = token + 1;
    }
    return left;
}

static void clear_error(json_error *error)
{
    if (error != NULL)
    {
        error->line = error->column = 0;
    }
}

static void set_error(const char *str, const char *end, json_error *error)
{
    if (error != NULL)
    {
        error->line = error->column = 1;
    }
    else
    {
        return;
    }
    for (; str < end; str++)
    {
        if (*str == '\n')
        {
            error->line++;
            error->column = 1;
        }
        else if (is_utf8(*str))
        {
            error->column++;
        }
    }
}

json *json_parse(const char *str, json_error *error)
{
    clear_error(error);
    if (str == NULL)
    {
        return NULL;
    }

    json *node = create_node();

    if (node != NULL)
    {
        const char *end = parse(node, str);

        if (end != NULL)
        {
            set_error(str, end, error);
            json_free(node);
            return NULL;
        }
    }
    return node;
}

static char *read_file(FILE *file, size_t size)
{
    char *str = malloc(size + 1);

    if (str != NULL)
    {
        if (fread(str, 1, size, file) == size)
        {
            str[size] = '\0';
        }
        else
        {
            free(str);
            str = NULL;
        }
    }
    return str;
}

static char *read_file_from(const char *path)
{
    FILE *file = fopen(path, "rb");

    if (file == NULL)
    {
        return NULL;
    }

    char *str = NULL;

    if (fseek(file, 0L, SEEK_END) != -1)
    {
        long size = ftell(file);

        if ((size != -1) && (fseek(file, 0L, SEEK_SET) != -1))
        {
            str = read_file(file, (size_t)size);
        }
    }
    fclose(file);
    return str;
}

json *json_parse_file(const char *path, json_error *error)
{
    char *str = read_file_from(path);
    json *node = json_parse(str, error);

    free(str);
    return node;
}

void json_print_error(const char *path, const json_error *error)
{
    if ((error == NULL) || (error->line == 0))
    {
        fprintf(stderr, "json: %s\n%s\n",
            path ? path : "", strerror(errno)
        );
    }
    else
    {
        fprintf(stderr, "json: %s\nError at line %d, column %d\n",
            path ? path : "", error->line, error->column
        );
    }
}

