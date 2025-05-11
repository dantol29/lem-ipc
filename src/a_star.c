#include "../includes/lem_ipc.h"

static void free_list(t_node *list, t_node *except)
{
    if (!list)
        return;

    t_node *prev;

    while (list->next)
    {
        prev = list;
        list = list->next;
        if (prev != except)
            free(prev);
    }

    if (list != except)
        free(list);
}

static void add_node(t_node **list, t_node *new_node)
{
    if (new_node == NULL)
        return;

    new_node->next = NULL;

    if (*list == NULL) // list is empty
    {
        *list = new_node;
        return;
    }

    t_node *current = *list;

    while (current->next)
        current = current->next;

    current->next = new_node;
}

static t_node *pop_node(t_node **list)
{
    if (!list || !*list)
        return NULL;

    t_node *current = *list;
    t_node *smallest = current;
    t_node *prev = NULL;
    int smallest_f = current->f;

    while (current->next)
    {
        if (current->next->f < smallest_f)
        {
            smallest_f = current->next->f;
            smallest = current->next;
            prev = current;
        }
        current = current->next;
    }

    if (prev)
        prev->next = smallest->next;
    else
        *list = smallest->next; // head is the smallets

    smallest->next = NULL;
    return smallest;
}

static int find_node(t_node **list, const t_node *node)
{
    if (!list || !*list || !node)
        return 0;

    t_node *current = *list;

    while (current)
    {
        if (current->x == node->x && current->y == node->y && current->f <= node->f)
            return 1;

        current = current->next;
    }

    return 0;
}

// g - movement cost from the starting point to a square
// h - movement cost from a square to the target
// f = g + h
static int manage_node(
    const void *field, t_node **open_list, t_node **closed_list, t_node *q, const int enemy_x, const int enemy_y, const int x, const int y)
{
    if (enemy_x == x && enemy_y == y)
        return 1;

    if (*((char *)field + y * FIELD_WIDTH + x) != 0)
        return 0;

    t_node *new_node = malloc(sizeof(t_node));
    if (!new_node)
        return 0;

    new_node->x = x;
    new_node->y = y;
    new_node->parent = q;

    int h = abs(x - enemy_x) + abs(y - enemy_y);
    new_node->g = q->g + 1;
    new_node->f = h + new_node->g;

    if (!find_node(open_list, new_node) && !find_node(closed_list, new_node))
        add_node(open_list, new_node);
    else
        free(new_node);

    return 0;
}

static int calculate_moves(
    const void *field, t_node **open_list, t_node **closed_list, t_node *q, const int enemy_x, const int enemy_y)
{
    if (q->x > 0) // left
        if (manage_node(field, open_list, closed_list, q, enemy_x, enemy_y, q->x - 1, q->y))
            return 1;
    if (q->x < FIELD_WIDTH - 1) // right
        if (manage_node(field, open_list, closed_list, q, enemy_x, enemy_y, q->x + 1, q->y))
            return 1;
    if (q->y > 0) // up
        if (manage_node(field, open_list, closed_list, q, enemy_x, enemy_y, q->x, q->y - 1))
            return 1;
    if (q->y < FIELD_HEIGHT - 1) // down
        if (manage_node(field, open_list, closed_list, q, enemy_x, enemy_y, q->x, q->y + 1))
            return 1;

    return 0;
}

t_node *a_star(const t_field *info, const int enemy_pos)
{
    const int enemy_y = enemy_pos / FIELD_WIDTH;
    const int enemy_x = enemy_pos - enemy_y * FIELD_WIDTH;

    t_node *closed_list = NULL;
    t_node *open_list = malloc(sizeof(t_node));
    if (!open_list)
        return NULL;

    open_list->next = NULL;
    open_list->parent = NULL;
    open_list->x = info->player_x;
    open_list->y = info->player_y;
    open_list->f = 0;
    open_list->g = 0;

    while (1)
    {
        t_node *q = pop_node(&open_list);
        if (!q)
            return NULL;

        if (calculate_moves(info->field, &open_list, &closed_list, q, enemy_x, enemy_y))
        {
            while (q->parent && q->parent->parent) // find second node
                q = q->parent;

            free_list(closed_list, q);
            free_list(open_list, NULL);
            return q;
        }

        add_node(&closed_list, q);
    }

    return NULL;
}
