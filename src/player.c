#include "../includes/lem_ipc.h"

static inline int is_enemy(const char pos, const char team)
{
    return pos != team && pos != WALL && pos != 0;
}

// Manhattan Distance
static int get_closest_enemy(t_field *info)
{
    int32_t enemy_distance = INT32_MAX;
    int32_t distance;
    int enemy_pos = -1;
    int enemy_x;
    int enemy_y;
    int i = 0;

    while (i < FIELD_WIDTH * FIELD_HEIGHT)
    {
        if (is_enemy(*((char *)info->field + i), info->team))
        {
            enemy_y = i / FIELD_WIDTH;
            enemy_x = i - enemy_y * FIELD_WIDTH;

            distance = abs(info->player_x - enemy_x) + abs(info->player_y - enemy_y);
            if (distance == 1)
                return -1; // enemy is already close

            if (distance < enemy_distance)
            {
                enemy_pos = i;
                enemy_distance = distance;
            }
        }
        ++i;
    }

    return enemy_pos;
}

static int is_surrounded(t_field *info)
{
    const void *player = (char *)info->field + info->player_pos;
    int count = 0;

    if (info->player_x > 0 && is_enemy(*((char *)player - 1), info->team))
        ++count; // left

    if (info->player_x < FIELD_WIDTH - 1 && is_enemy(*((char *)player + 1), info->team))
        ++count; // right

    if (info->player_y > 0 && is_enemy(*((char *)player - FIELD_WIDTH), info->team))
        ++count; // up

    if (info->player_y < FIELD_HEIGHT - 1 && is_enemy(*((char *)player + FIELD_WIDTH), info->team))
        ++count; // down

    if (info->player_x > 0 && info->player_y > 0 && is_enemy(*((char *)player - FIELD_WIDTH - 1), info->team))
        ++count; // diagonal upper-left

    if (info->player_x > 0 && info->player_y < FIELD_HEIGHT - 1 && is_enemy(*((char *)player + FIELD_WIDTH - 1), info->team))
        ++count; // diagonal lower-left

    if (info->player_x < FIELD_WIDTH - 1 && info->player_y > 0 && is_enemy(*((char *)player - FIELD_WIDTH + 1), info->team))
        ++count; // diagonal upper-right

    if (info->player_x < FIELD_WIDTH - 1 && info->player_y < FIELD_HEIGHT - 1 && is_enemy(*((char *)player + FIELD_WIDTH + 1), info->team))
        ++count; // diagonal lower-right

    return count > 1 ? 1 : 0;
}

static int update_position(t_field *info, int enemy_pos)
{
    t_node *node = a_star(info, enemy_pos);
    if (node)
    {
        *((char *)info->field + info->player_pos) = 0;
        *((char *)info->field + node->y * FIELD_WIDTH + node->x) = info->team;

        info->player_pos = node->y * FIELD_WIDTH + node->x;
    }

    return info->player_pos;
}

int place_player(const char team, const void *shared_memory)
{
    void *field = (size_t *)shared_memory + 2;

    const int team1_positions[] = {0, 2, FIELD_WIDTH * 2, FIELD_WIDTH * 2 + 2};
    const int team2_positions[] = {FIELD_WIDTH - 3, FIELD_WIDTH - 1, FIELD_WIDTH * 2 - 3, FIELD_WIDTH * 2 - 1};
    const int team3_positions[] = {FIELD_WIDTH * FIELD_HEIGHT - FIELD_WIDTH, FIELD_WIDTH * FIELD_HEIGHT - FIELD_WIDTH * 2,
                                   FIELD_WIDTH * FIELD_HEIGHT - FIELD_WIDTH + 2, FIELD_WIDTH * FIELD_HEIGHT - FIELD_WIDTH * 2 + 2};
    const int team4_positions[] = {FIELD_HEIGHT * FIELD_WIDTH - 1, FIELD_HEIGHT * FIELD_WIDTH - 3,
                                   FIELD_HEIGHT * FIELD_WIDTH - FIELD_WIDTH - 1, FIELD_HEIGHT * FIELD_WIDTH - FIELD_WIDTH - 3};

    const int *positions = NULL;
    int position = -1;

    if (team == 1)
        positions = team1_positions;
    else if (team == 2)
        positions = team2_positions;
    else if (team == 3)
        positions = team3_positions;
    else if (team == 4)
        positions = team4_positions;
    else
        return position;

    update_semaphore(0, -1); // enter smph

    int i = 0;
    while (i < TEAM_SIZE)
    {
        if (*((char *)field + positions[i]) == 0)
        {
            *((char *)field + positions[i]) = team;
            *(size_t *)shared_memory += 1; // player_count

            if (i == 0)
                *((size_t *)shared_memory + 1) += 1; // team_count

            position = positions[i];
            break;
        }
        ++i;
    }

    update_semaphore(0, 1); // exit smph

    return position;
}

void player_loop(const void *shared_memory, t_field *info)
{
    while (1)
    {
        info->player_y = info->player_pos / FIELD_WIDTH;
        info->player_x = info->player_pos - info->player_y * FIELD_WIDTH;

        update_semaphore(0, -1); // enter smph

        if (*((size_t *)shared_memory + 1) > 1)
        {
            if (is_surrounded(info))
            {
                *(size_t *)shared_memory -= 1;
                *((char *)info->field + info->player_pos) = 0;
                update_semaphore(0, 1); // exit smph
                return;
            }

            // check if someone needs help surrounding enemy
            // int enemy_pos = 0;
            // if (1)
            //     info->player_pos = update_position(&info, enemy_pos);
            // else
            // {
            int enemy_pos = get_closest_enemy(info);
            if (enemy_pos != -1)
                info->player_pos = update_position(info, enemy_pos);
            // }
        }

        update_semaphore(0, 1); // exit smph
        usleep(300000);
    }
}
