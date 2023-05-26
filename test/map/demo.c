/*!
 *  \brief     strictissimo - Ligtweight json and json-schema library for C
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include <stdlib.h>
#include <time.h>
#include <json/json.h>
#include <json/json_map.h>

static json_map *map;

static void clean(void)
{
    puts("\nDestroying ...");
    json_map_destroy(map, json_free);
}

int main(void)
{
    enum {N = 100};

    atexit(clean);
    srand((unsigned)time(NULL));

    // Map can be created with any size including 0
    if (!(map = json_map_create(N)))
    {
        perror("json_map_create");
        exit(EXIT_FAILURE);
    }

    char str[32];

    puts("Inserting ...");
    for (int i = 0; i < N; i++)
    {
        int r = rand() % N;

        snprintf(str, sizeof str, "Item #%d", r);

        json *node = json_new_string(NULL, str);
        json *temp = node;

        if ((node = json_map_insert(map, str, node)))
        {
            if (temp != node)
            {
                printf("%s repeated\n", json_string(node));
                json_delete(temp);
            }
            else
            {
                //printf("%s inserted\n", json_string(node));
            }
        }
        else
        {
            perror("json_map_insert");
            exit(EXIT_FAILURE);
        }
    }

    printf("\nSize of map = %zu\n", json_map_size(map));

    puts("\nSearching ...");
    for (int i = 0; i < N / 10; i++)
    {
        int r = rand() % N;

        snprintf(str, sizeof str, "Item #%d", r);

        json *node = json_map_search(map, str);

        if (node != NULL)
        {
            printf("%s located\n", json_string(node));
        }
        else
        {
            printf("%s not found\n", str);
        }
    }

    puts("\nDeleting ...");
    for (int i = 0; i < N / 10; i++)
    {
        int r = rand() % N;

        snprintf(str, sizeof str, "Item #%d", r);

        json *node = json_map_delete(map, str);

        if (node != NULL)
        {
            printf("%s deleted\n", json_string(node));
            json_delete(node);
        }
        else
        {
            printf("%s not found\n", str);
        }
    }

    printf("\nSize of map = %zu\n", json_map_size(map));

    return 0;
}

