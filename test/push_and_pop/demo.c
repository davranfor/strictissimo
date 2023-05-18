/*!
 *  \brief     strictissimo - Ligtweight json and json-schema library for C
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

/**
 * Testing push and pop family functions
 * -------------------------------------
 * 50 random insertions or deletions
 * Shows the result in a json object
 */

#include <stdlib.h>
#include <time.h>
#include <json/json.h>

enum {N = 50};

int main(void)
{
    srand((unsigned)time(NULL));

    json *root, *node;

    root = json_new_array("array");
    for (size_t i = 0; i < N; i++)
    {
        unsigned number = (unsigned)rand() % 90 + 10;
        unsigned offset = (unsigned)rand() % 5;
        unsigned random = (unsigned)rand() % 9;

        switch (random)
        {
            case 0:
                printf("json_push_front(%u)\n", number);
                json_push_front(root, json_new_real(NULL, number));
                break;
            case 1:
                printf("json_push_back(%u)\n", number);
                json_push_back(root, json_new_real(NULL, number));
                break;
            case 2:
                printf("json_push_before(#%u: %u)\n", offset, number);
                if ((node = json_at(root, offset)))
                {
                    json_push_before(node, json_new_real(NULL, number));
                }
                break;
            case 3:
                printf("json_push_after(#%u: %u)\n", offset, number);
                if ((node = json_at(root, offset)))
                {
                    json_push_after(node, json_new_real(NULL, number));
                }
                break;
            case 4:
                printf("json_push_at(#%u: %u)\n", offset, number);
                json_push_at(root, json_new_real(NULL, number), offset);
                break;
            case 5:
                printf("json_pop(#%u)\n", offset);
                json_free(json_pop(json_at(root, offset)));
                break;
            case 6:
                printf("json_pop_front()\n");
                json_free(json_pop_front(root));
                break;
            case 7:
                printf("json_pop_back()\n");
                json_free(json_pop_back(root));
                break;
            case 8:
                printf("json_pop_at(#%u)\n", offset);
                json_free(json_pop_at(root, offset));
                break;
            default:
                break;
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

