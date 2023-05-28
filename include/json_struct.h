/*!
 *  \brief     strictissimo - Ligtweight json and json-schema library for C
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#ifndef JSON_STRUCT_H
#define JSON_STRUCT_H

#include "json.h"

union json_value
{
    char *as_string;
    double as_number;
};

struct json
{
    json *parent, *child, *prev, *next;
    char *name;
    union json_value value;
    enum json_type type;
};

#endif /* JSON_STRUCT_H */

