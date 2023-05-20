/*!
 *  \brief     strictissimo - Ligtweight json and json-schema library for C
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

/*
Download, parse and display a json containing a joke from official-joke-api.appspot.com

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

static size_t string_read(void *ptr, size_t size, size_t nmemb, void *stream)
{
    struct string *str = stream;
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

static size_t url_read(struct string *str)
{
    curl_global_init(CURL_GLOBAL_DEFAULT);

    struct curl_slist *headers = NULL;

    headers = curl_slist_append(headers, "Accept: application/json");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "charset: utf-8");

    CURL *curl = curl_easy_init();
    CURLcode res = CURLE_OK;

    if (curl != NULL)
    {
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
        curl_easy_setopt(curl, CURLOPT_URL,
            "https://official-joke-api.appspot.com/random_joke");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, string_read);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, str);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    curl_slist_free_all(headers);
    curl_global_cleanup();
    return (res == CURLE_OK) ? str->len : 0;
}

int main(void)
{
    struct string str = {0};

    if (url_read(&str) == 0)
    {
        perror("url_read");
        exit(EXIT_FAILURE);
    }

    json_error error;
    json *node = json_parse(str.data, &error);

    if (node != NULL)
    {
        printf("\nQ: %s\nA: %s\n",
            json_string(json_find(node, "setup")),
            json_string(json_find(node, "punchline"))
        );
        json_free(node);
    }
    else
    {
        json_print_error(NULL, &error);
        puts(str.data);
    }
    free(str.data);
    return 0;
}

