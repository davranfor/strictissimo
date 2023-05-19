/*!
 *  \brief     strictissimo - Ligtweight json and json-schema library for C
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include <time.h>
#include <json/json.h>

/* Sort numbers in ascending order */
static int compare(const json *a, const json *b)
{
    if (json_is_number(a) && json_is_number(b))
    {
        return json_number(a) > json_number(b);
    }
    return json_type(a) > json_type(b);
}

int main(void)
{
    srand((unsigned)time(NULL));

    json *root, *tail = NULL;

    root = json_new_array(NULL);
    for (int i = 0; i < 25; i++)
    {
        int r = rand() % 100;

        tail = json_push_fast(root, tail, json_new_integer(NULL, r));
    }
    puts("Unsorted:");
    json_print(root);
    puts("Reversed:");
    json_reverse(root);
    json_print(root);
    puts("Sorted:");
    json_sort(root, compare);
    json_print(root);
    json_free(root);
    return 0;
}

