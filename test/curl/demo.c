/*!
 *  \brief     strictissimo - Ligtweight json and json-schema library for C
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

/*
For this example you may need to install the development libraries for curl

On debian:
sudo apt install libcurl4-gnutls-dev

Compile and run with:
CFLAGS="-std=c11 -Wpedantic -Wall -Wextra -O2" LDLIBS="-lcurl -ljson" make demo && ./demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json/json.h>

struct string
{
    char *data;
    size_t len;
};

static void string_init(struct string *str)
{
    str->data = malloc(1);
    if (str->data == NULL)
    {
        perror("calloc");
        exit(EXIT_FAILURE);
    }
    str->len = 0;
}

static size_t string_read(void *ptr, size_t size, size_t nmemb, struct string *str)
{
    size_t len = str->len + size * nmemb;

    str->data = realloc(str->data, len + 1);
    if (str->data == NULL)
    {
        perror("realloc");
        exit(EXIT_FAILURE);
    }
    memcpy(str->data + str->len, ptr, size * nmemb);
    str->data[len] = '\0';
    str->len = len;
    return size * nmemb;
}

int main(void)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

    CURL *curl = curl_easy_init();

    if (curl != NULL)
    {
        struct string str;

        string_init(&str);

        curl_easy_setopt(curl, CURLOPT_URL, "https://official-joke-api.appspot.com/random_joke");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, string_read);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &str);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        curl_global_cleanup();

        json_error error;
        json *node = json_parse(str.data, &error);

        if (node != NULL)
        {
            const char *question = json_string(json_find(node, "setup"));
            const char *answer = json_string(json_find(node, "punchline"));

            if (question && answer)
            {
                printf("\nQ: %s\nA: %s\n", question, answer);
            }
            json_free(node);
        }
        else
        {
            json_print_error(NULL, &error);
            puts(str.data);
        }
        free(str.data);
    }
    return 0;
}

