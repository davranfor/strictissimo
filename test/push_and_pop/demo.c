/*!
 *  \brief     strictissimo - Ligtweight json and json-schema library for C
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include <time.h>
#include <locale.h>
#include <json/json.h>

enum {N = 50};

/**
 * Testing push and pop family functions
 * -------------------------------------
 * 50 random insertions or deletions
 * Shows the result in a json object
 */

int main(void)
{
    setlocale(LC_CTYPE, "");
    srand((unsigned)time(NULL));

    json *root;

    root = json_new_array("array");
    for (int i = 0; i < N; i++)
    {
        int rnd = rand() % 18;
        int idx = rand() % 10;

        if (rnd < 2)
        {
            printf("json_push_front(%d)\n", idx);
            json_push_front(root, json_new_integer(NULL, idx));
        }
        else if (rnd < 4)
        {
            printf("json_push_back(%d)\n", idx);
            json_push_back(root, json_new_integer(NULL, idx));
        }
        else if (rnd < 6)
        {
            json *node = json_new_integer(NULL, idx);

            printf("json_push_before(push %d before item %d)\n", idx, idx);
            if (!json_push_before(json_at(root, (size_t)idx), node))
            {
                json_free(node);
            }
        }
        else if (rnd < 8)
        {
            json *node = json_new_integer(NULL, idx);

            printf("json_push_after(push %d after item %d)\n", idx, idx);
            if (!json_push_after(json_at(root, (size_t)idx), node))
            {
                json_free(node);
            }
        }
        else if (rnd < 10)
        {
            json *node = json_new_integer(NULL, idx);

            printf("json_push_at(push %d at item %d)\n", idx, idx);
            if (!json_push_at(root, node, (size_t)idx))
            {
                json_free(node);
            }
        }
        else if (rnd < 12)
        {
            printf("json_pop(%d)\n", idx);
            json_free(json_pop(json_at(root, (size_t)idx)));
        }
        else if (rnd < 14)
        {
            printf("json_pop_front()\n");
            json_free(json_pop_front(root));
        }
        else if (rnd < 16)
        {
            printf("json_pop_back()\n");
            json_free(json_pop_back(root));
        }
        else if (rnd < 18)
        {
            printf("json_pop_at(%d)\n", idx);
            json_free(json_pop_at(root, (size_t)idx));
        }
    }
    puts("");
    if (json_push_front(json_new_object(NULL), root))
    {
        json_push_after(root, json_new_real("size", json_size(root)));
        root = json_parent(root);
    }
    json_print(root);
    json_free(root);
    return 0;
}

