/*!
 *  \brief     strictissimo - Ligtweight json and json-schema library for C
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include <locale.h>
#include <json/json_schema.h>

static json *parse_file(const char *path)
{
    json_error error; // Error handle is optional
    json *node = json_parse_file(path, &error);

    if (node == NULL)
    {
        json_print_error(path, &error);
    }
    return node;
}

static void print_event(const char *title, const json *node)
{
    char *path = json_path(node);

    if (path != NULL)
    {
        fprintf(stderr, "%s %s\n", title, path);
        free(path);
    }
    if (json_is_scalar(node))
    {
        char *str = json_encode_value(node);

        if (str != NULL)
        {
            fprintf(stderr, "-> %s\n", str);
            free(str);
        }
    }
}

/**
 * json_validate() callback function
 *
 * Events:
 * 0) Warning: A keyword is irrelevant for validation
 * 1) Invalid: Doesn't validate against a schema rule
 * 2)   Error: Keyword with unexpected value (malformed schema)
 *
 * Return:
 *  0 to stop validating
 * !0 to continue
 *
 * Note:
 * Validation stops on event 2 (Error) even returning a non 0 value
 */
static int on_event(const json *node, const json *rule, int event, void *data)
{
    (void)data;

    const char *event_title[] = {"Warning", "Invalid", "Error"};

    fprintf(stderr, "\n[%s]\n", event_title[event]);
    print_event("Testing:", node);
    print_event("On rule:", rule);
    return 1; // Continue
}

static void validate_schema(const json *node, const char *path)
{
    json *schema = parse_file(path);

    if (schema != NULL)
    {
        puts("json:");
        json_print(node);
        puts("schema.json:");
        json_print(schema);
        if (json_validate(node, schema, on_event, NULL))
        {
            fprintf(stdout, "\n'%s' validated without errors\n", path);
        }
        else
        {
            fprintf(stderr, "\n'%s' doesn't validate\n", path);
        }
        json_free(schema);
    }
}

int main(void)
{
    setlocale(LC_CTYPE, "");

    json *node = parse_file("test.json");

    if (node != NULL)
    {
        validate_schema(node, "test.schema.json");
        json_free(node);
    }
    return 0;
}

