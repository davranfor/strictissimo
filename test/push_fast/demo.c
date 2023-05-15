/*!
 *  \brief     strictissimo - Ligtweight json and json-schema library for C
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <locale.h>
#include <json/json.h>

int main(void)
{
    setlocale(LC_CTYPE, "");

    json *root, *tail = NULL;

    root = json_new_array(NULL);
    for (int i = 0; i < 25; i++)
    {
        // Push element at the end of the array. O(1) time complexity
        tail = json_push_fast(root, tail, json_new_integer(NULL, i));
    }
    json_print(root);
    json_free(root);
    return 0;
}

