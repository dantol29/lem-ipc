#include "../includes/lem_ipc.h"

int place_player(char team, void *shared_memory)
{
    void *field = (size_t *)shared_memory + 2;

    // Top-left corner
    const int team1_positions[] = {
        0 * FIELD_WIDTH + 0, 0 * FIELD_WIDTH + 2,
        2 * FIELD_WIDTH + 0, 2 * FIELD_WIDTH + 2};

    // Top-right corner
    const int team2_positions[] = {
        0 * FIELD_WIDTH + (FIELD_WIDTH - 2), 0 * FIELD_WIDTH + (FIELD_WIDTH - 1),
        2 * FIELD_WIDTH + (FIELD_WIDTH - 2), 2 * FIELD_WIDTH + (FIELD_WIDTH - 1)};

    // Bottom-left corner
    const int team3_positions[] = {
        (FIELD_HEIGHT - 3) * FIELD_WIDTH + 0, (FIELD_HEIGHT - 3) * FIELD_WIDTH + 2,
        (FIELD_HEIGHT - 1) * FIELD_WIDTH + 0, (FIELD_HEIGHT - 1) * FIELD_WIDTH + 2};

    // Bottom-right corner
    const int team4_positions[] = {
        (FIELD_HEIGHT - 3) * FIELD_WIDTH + (FIELD_WIDTH - 2), (FIELD_HEIGHT - 3) * FIELD_WIDTH + (FIELD_WIDTH - 1),
        (FIELD_HEIGHT - 1) * FIELD_WIDTH + (FIELD_WIDTH - 2), (FIELD_HEIGHT - 1) * FIELD_WIDTH + (FIELD_WIDTH - 1)};

    const int *positions = NULL;

    if (team == 1)
        positions = team1_positions;
    else if (team == 2)
        positions = team2_positions;
    else if (team == 3)
        positions = team3_positions;
    else if (team == 4)
        positions = team4_positions;
    else
        return -1;

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

            update_semaphore(0, 1); // exit smph
            return positions[i];
        }
        ++i;
    }

    update_semaphore(0, 1); // exit smph
    return -1;
}

// Manhattan Distance
static int get_closest_enemy(const void *field, char team, int player_x, int player_y, int *enemy_dist)
{
    int32_t enemy_distance = INT32_MAX;
    int32_t distance;
    int enemy_pos = -1;
    int enemy_x;
    int enemy_y;
    int i = 0;

    while (i < FIELD_WIDTH * FIELD_HEIGHT)
    {
        if (*((char *)field + i) != 0 && *((char *)field + i) != team && *((char *)field + i) != WALL)
        {
            enemy_y = i / FIELD_WIDTH;
            enemy_x = i - enemy_y * FIELD_WIDTH;

            distance = abs(player_x - enemy_x) + abs(player_y - enemy_y);
            if (distance == 1)
                return -1; // enemy is already close
            else if (distance < enemy_distance)
            {
                enemy_pos = i;
                enemy_distance = distance;
            }
        }
        ++i;
    }

    *enemy_dist = enemy_distance;
    return enemy_pos;
}

void player_loop(void *shared_memory, const char team, int player_pos)
{
    const void *field = (size_t *)shared_memory + 2;
    int enemy_distance;

    while (1)
    {
        const int player_y = player_pos / FIELD_WIDTH;
        const int player_x = player_pos - player_y * FIELD_WIDTH;

        update_semaphore(0, -1); // enter smph

        if (*((size_t *)shared_memory + 1) > 1)
        {
            int enemy_pos = get_closest_enemy(field, team, player_x, player_y, &enemy_distance);
            if (enemy_pos != -1)
            {
                t_node *node = a_star(field, player_x, player_y, enemy_pos);
                if (node)
                {
                    *((char *)field + player_pos) = 0;
                    *((char *)field + node->y * FIELD_WIDTH + node->x) = team;

                    player_pos = node->y * FIELD_WIDTH + node->x;
                }
            }
            else
            {
                *(size_t *)shared_memory -= 1;
                update_semaphore(0, 1); // exit smph
                return;
            }
        }

        update_semaphore(0, 1); // exit smph
        usleep(900000);
    }
}
