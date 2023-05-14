/*!
 *  \brief     strictissimo - Ligtweight json and json-schema library for C
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdio.h>
#include <locale.h>
#include <json/json.h>

static void parse(const char *path)
{
    json_error error; // Error handle is optional
    json *node = json_parse_file(path, &error);

    if (node == NULL)
    {
        json_print_error(path, &error);
    }
    else
    {
        json_print(node);
        json_free(node);
    }
}

int main(void)
{
    setlocale(LC_CTYPE, "");
    parse("test.json");
    return 0;
}

