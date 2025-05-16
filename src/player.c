#include "../includes/lem_ipc.h"

static inline int is_enemy(const char pos, const char team)
{
    return pos != WALL && pos != 0 && (pos < team * team || pos > team * team + TEAM_SIZE - 1);
}

static inline int get_enemy_team(const char enemy)
{
    int i = 2;
    while (i < enemy)
    {
        if (i * i > enemy)
            return i - 1;
        ++i;
    }

    return INT16_MAX;
}

static void clean_player_data(const void *shared_memory, t_field *info, const t_state *state)
{
    *(size_t *)shared_memory -= 1;
    *((char *)info->field + info->player_pos) = 0;
    update_semaphore(0, 1, state->semaphores_id); // exit smph
}

// Manhattan Distance
static int get_closest_enemy(const t_field *info)
{
    int32_t enemy_distance = INT32_MAX;
    int32_t distance;
    int enemy_pos = NOT_FOUND;
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
                return ENEMY_CLOSE; // enemy is already close

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

static int is_surrounded(const t_field *info)
{
    const void *player = (char *)info->field + info->player_pos;
    int enemies[TEAM_COUNT + 2] = {0, 0, 0, 0, 0, 0};

    if (info->player_x > 0 && is_enemy(*((char *)player - 1), info->team))
        enemies[get_enemy_team(*((char *)player - 1))] += 1; // left

    if (info->player_x < FIELD_WIDTH - 1 && is_enemy(*((char *)player + 1), info->team))
        enemies[get_enemy_team(*((char *)player + 1))] += 1; // right

    if (info->player_y > 0 && is_enemy(*((char *)player - FIELD_WIDTH), info->team))
        enemies[get_enemy_team(*((char *)player - FIELD_WIDTH))] += 1; // up

    if (info->player_y < FIELD_HEIGHT - 1 && is_enemy(*((char *)player + FIELD_WIDTH), info->team))
        enemies[get_enemy_team(*((char *)player + FIELD_WIDTH))] += 1; // down

    if (info->player_x > 0 && info->player_y > 0 && is_enemy(*((char *)player - FIELD_WIDTH - 1), info->team))
        enemies[get_enemy_team(*((char *)player - FIELD_WIDTH - 1))] += 1; // diagonal upper-left

    if (info->player_x > 0 && info->player_y < FIELD_HEIGHT - 1 && is_enemy(*((char *)player + FIELD_WIDTH - 1), info->team))
        enemies[get_enemy_team(*((char *)player + FIELD_WIDTH - 1))] += 1; // diagonal lower-left

    if (info->player_x < FIELD_WIDTH - 1 && info->player_y > 0 && is_enemy(*((char *)player - FIELD_WIDTH + 1), info->team))
        enemies[get_enemy_team(*((char *)player - FIELD_WIDTH + 1))] += 1; // diagonal upper-right

    if (info->player_x < FIELD_WIDTH - 1 && info->player_y < FIELD_HEIGHT - 1 && is_enemy(*((char *)player + FIELD_WIDTH + 1), info->team))
        enemies[get_enemy_team(*((char *)player + 1 + FIELD_WIDTH))] += 1; // diagonal lower-right

    int i = 2; // teams numbers start from 2
    while (i < TEAM_COUNT + 2)
    {
        if (enemies[i] > 1)
            return 1;
        ++i;
    }

    return 0;
}

static void update_position(t_field *info, const int enemy_pos)
{
    t_node *node = a_star(info, enemy_pos);
    if (node)
    {
        *((char *)info->field + info->player_pos) = 0;
        *((char *)info->field + node->y * FIELD_WIDTH + node->x) = info->player_id;
        info->player_pos = node->y * FIELD_WIDTH + node->x;
    }
}

int place_player(const t_state *state, const char team, const void *shared_memory)
{
    void *field = (size_t *)shared_memory + 3;

    const int team1_positions[] = {0, 2, FIELD_WIDTH * 2, FIELD_WIDTH * 2 + 2};
    const int team2_positions[] = {FIELD_WIDTH - 3, FIELD_WIDTH - 1, FIELD_WIDTH * 2 - 3, FIELD_WIDTH * 2 - 1};
    const int team3_positions[] = {FIELD_WIDTH * FIELD_HEIGHT - FIELD_WIDTH, FIELD_WIDTH * FIELD_HEIGHT - FIELD_WIDTH * 2,
                                   FIELD_WIDTH * FIELD_HEIGHT - FIELD_WIDTH + 2, FIELD_WIDTH * FIELD_HEIGHT - FIELD_WIDTH * 2 + 2};
    const int team4_positions[] = {FIELD_HEIGHT * FIELD_WIDTH - 1, FIELD_HEIGHT * FIELD_WIDTH - 3,
                                   FIELD_HEIGHT * FIELD_WIDTH - FIELD_WIDTH - 1, FIELD_HEIGHT * FIELD_WIDTH - FIELD_WIDTH - 3};

    const int *positions = NULL;
    int position = -1;

    if (team == 2)
        positions = team1_positions;
    else if (team == 3)
        positions = team2_positions;
    else if (team == 4)
        positions = team3_positions;
    else if (team == 5)
        positions = team4_positions;
    else
        return position;

    update_semaphore(0, -1, state->semaphores_id); // enter smph

    int i = 0;
    while (i < TEAM_SIZE)
    {
        if (*((char *)field + positions[i]) == 0)
        {
            *((char *)field + positions[i]) = i + team * team; // player_id
            *(size_t *)shared_memory += 1;                     // player_count

            if (i == 0)
                *((size_t *)shared_memory + 1) += 1; // team_count

            position = positions[i];
            break;
        }
        ++i;
    }

    update_semaphore(0, 1, state->semaphores_id); // exit smph

    return position;
}

void player_loop(const t_state *state, const void *shared_memory, t_field *info)
{
    t_msg msg;
    int enemy_pos;
    size_t *is_started = (size_t *)shared_memory + 2;

    while (1)
    {
        update_semaphore(0, -1, state->semaphores_id); // enter smph

        if (*is_started)
        {
            info->player_y = info->player_pos / FIELD_WIDTH;
            info->player_x = info->player_pos - info->player_y * FIELD_WIDTH;

            if (is_surrounded(info))
                return clean_player_data(shared_memory, info, state);

            int status = dequeue(&msg, state->message_queue_id); // mates need help surrounding the enemy
            if (status)
            {
                enemy_pos = get_player_pos(info, msg.enemy_id);
                if (enemy_pos != -1)
                    update_position(info, enemy_pos);
            }
            else
            {
                enemy_pos = get_closest_enemy(info);
                if (enemy_pos == NOT_FOUND) // Congrats! You won!
                    return clean_player_data(shared_memory, info, state);

                if (enemy_pos != ENEMY_CLOSE)
                {
                    update_position(info, enemy_pos);
                    if (info->player_id > info->team * info->team)                            // more than 1 player on the team
                        enqueue(*((char *)info->field + enemy_pos), state->message_queue_id); // pass enemy_id
                }
            }
        }

        update_semaphore(0, 1, state->semaphores_id); // exit smph
        usleep(PLAYER_DELAY);
    }
}
