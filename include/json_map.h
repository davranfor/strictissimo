/*!
 *  \brief     strictissimo - Ligtweight json and json-schema library for C
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef JSON_MAP_H
#define JSON_MAP_H

typedef struct json json;
typedef struct json_map json_map;

json_map *json_map_create(size_t);
json *json_map_insert(json_map *, const char *, json *);
json *json_map_delete(json_map *, const char *);
json *json_map_search(const json_map *, const char *);
size_t json_map_size(const json_map *);
void json_map_destroy(json_map *, void (*)(json *));

#endif /* JSON_MAP_H */

