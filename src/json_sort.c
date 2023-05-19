/*!
 *  \brief     strictissimo - Ligtweight json and json-schema library for C
 *  \author    David Ranieri <davranfor@gmail.com>
 *  \copyright GNU Public License.
 */

#include "json_struct.h"

static json *split(json *head)
{
    json *fast = head;
    json *slow = head;

    while ((fast->next != NULL) && (fast->next->next != NULL))
    {
        fast = fast->next->next;
        slow = slow->next;
    }

    json *temp = slow->next;

    slow->next = NULL;
    return temp;
}

static json *merge(json *a, json *b, json_compare compare)
{
    if (a == NULL)
    {
        return b;
    }
    if (b == NULL)
    {
        return a;
    }

    json *pa = a;

    while (b != NULL)
    {
        json *pb = b;

        b = pb->next;
        pb->next = NULL;
        if (compare(a, pb) > 0)
        {
            pb->next = a;
            a = pa = pb;
        }
        else while (1)
        {
            if (pa->next == NULL)
            {
                pa->next = pb;
                pa = pb;
                break;
            }
            if (compare(pa->next, pb) > 0)
            {
                pb->next = pa->next;
                pa->next = pb;
                break;
            }
            pa = pa->next;
        }
    }
    return a;
}

static json *sort(json *head, json_compare compare)
{
    if ((head == NULL) || (head->next == NULL))
    {
        return head;
    }

    json *mid = split(head);

    head = sort(head, compare);
    mid = sort(mid, compare);
    return merge(head, mid, compare);
}

json *json_sort(json *root, json_compare compare)
{
    if ((root != NULL) && (root->child != NULL))
    {
        json *node = sort(root->child, compare);

        root->child = node;
        // Reconnect prev nodes
        node->prev = NULL;
        while (node->next != NULL)
        {
            node->next->prev = node;
            node = node->next;
        }
    }
    return root;
}

json *json_reverse(json *root)
{
    if ((root != NULL) && (root->child != NULL))
    {
        json *node = root->child, *prev = NULL;

        while (node != NULL)
        {
            prev = node->prev;
            node->prev = node->next;
            node->next = prev;
            node = node->prev;
        }
        if (prev != NULL)
        {
            root->child = prev->prev;
        }
    }
    return root;
}

