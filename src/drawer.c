#include "../includes/lem_ipc.h"

struct drawer_info
{
    const void *field;
    const void *shared_memory;
    const t_state *state;
    size_t player_count;
    size_t team_count;
    size_t is_started;
    int mouse_x;
    int mouse_y;
    mlx_image_t *players;
    mlx_image_t *teams;
    mlx_image_t *started;
    mlx_image_t *play_button;
    mlx_image_t *exit_button;
    mlx_image_t *image;
    mlx_t *mlx;
};

static inline uint32_t get_color(const int id)
{
    if (id == WALL)
        return 0xEADDCAFF;
    if (id == 0)
        return 0x000000FF;
    if (id < 8) // 1st team: 4-7
        return 0xE63946FF;
    if (id < 13) // 2nd team: 9-12
        return 0x1D3557FF;
    if (id < 20) // 3rd team: 16-19
        return 0x2A9D8FFF;
    if (id < 29) // 4th team: 25-28
        return 0xF4A261FF;

    return 0x000000FF;
}

static inline void exit_mlx(mlx_t *mlx, struct drawer_info *info)
{
    if (info->players)
	    mlx_delete_image(mlx, info->players);
    if (info->teams)
            mlx_delete_image(mlx, info->teams);
    if (info->started)
            mlx_delete_image(mlx, info->started);
    if (info->play_button)
	    mlx_delete_image(mlx, info->play_button);
    if (info->exit_button)
	    mlx_delete_image(mlx, info->exit_button);
    if (info->image)
            mlx_delete_image(mlx, info->image);

    mlx_terminate(mlx);
    cleanup_resources(info->state);
}

static mlx_image_t* draw_image(struct drawer_info *info, const char *path, const int x, const int y)
{
    mlx_texture_t *png = mlx_load_png(path);
    if (!png)
        exit_mlx(info->mlx, info);

    mlx_image_t *img = mlx_texture_to_image(info->mlx, png);
    if (!img)
        exit_mlx(info->mlx, info);

    mlx_delete_texture(png);

    if (mlx_image_to_window(info->mlx, img, x, y) < 0)
        exit_mlx(info->mlx, info);
    
    return img;
}

static mlx_image_t *draw_string(struct drawer_info *info, const char *string, size_t number, const int x, const int y)
{
    char buffer1[strlen(string) + 1 + 4];
    sprintf(buffer1, "%s: %ld", string, number);

    mlx_image_t *image = mlx_put_string(info->mlx, buffer1, x, y);
    if (!image)
        exit_mlx(info->mlx, info);

    return image;
}

static void draw_tile(const struct drawer_info *info, const int x, const int y, const uint32_t color)
{
    uint32_t tmp;
    short height = 0;
    short width;

    while (height < 32)
    {
        width = 0;
        while (width < 32)
        {
            tmp = color;
            if (color != 0x000000FF && (height == 0 || height == 31 || width == 0 || width == 31))
                tmp = 0xFFFDD011;

            mlx_put_pixel(info->image, x + width, y + height, tmp);
            ++width;
        }
        ++height;
    }
}

static void game_hook(void *param)
{
    struct drawer_info *info = param;
    char player_id;

    register short j;
    register short i = 0;
    
    update_semaphore(0, -1, info->state->semaphores_id); // enter smph
    
    while (i < FIELD_HEIGHT)
    {
        j = 0;
        while (j < FIELD_WIDTH)
        {
            player_id = *((char *)info->field + j + i * FIELD_WIDTH);
            draw_tile(info, j * 32, i * 32, get_color((int)player_id));
            ++j;
        }
        ++i;
    }

    if (*(size_t *)info->shared_memory != info->player_count)
    {
        mlx_delete_image(info->mlx, info->players);
        info->player_count = *(size_t *)info->shared_memory;
        info->players = draw_string(info, "Players", info->player_count, 12, 32 * FIELD_HEIGHT + 10);
    }

    if (*((size_t *)info->shared_memory + 1) != info->team_count)
    {
        mlx_delete_image(info->mlx, info->teams);
        info->team_count = *((size_t *)info->shared_memory + 1);
        info->teams = draw_string(info, "Teams", info->team_count, 12, 32 * FIELD_HEIGHT + 40);
    }

    if (*((size_t *)info->shared_memory + 2) != info->is_started)
    {
        mlx_delete_image(info->mlx, info->started);
        info->is_started = *((size_t *)info->shared_memory + 2);
        info->started = draw_string(info, "Started", info->is_started, 12, 32 * FIELD_HEIGHT + 70);
    }

    update_semaphore(0, 1, info->state->semaphores_id); // exit smp
}

static void cursor_hook(double xpos, double ypos, void *param)
{
    struct drawer_info *info = param;

    info->mouse_x = xpos;
    info->mouse_y = ypos;
}

static void mouse_hook(mouse_key_t button, action_t action, modifier_key_t mods, void *param)
{
    (void)mods;
    struct drawer_info *info = param;

    if (button != 0 || action == 0)
        return;

    int x = (info->mouse_x + 10) / 32;
    int y = (info->mouse_y + 10) / 32;

    if (x >= FIELD_WIDTH || y >= FIELD_HEIGHT)
    {
        if (x == 30 && y == 1) // EXIT
        {
            mlx_terminate(info->mlx);
            exit_error(info->state, "User clicked on exit", CLEANUP);
        }
        else if (x == 30 && y == 3) // START
        {
            update_semaphore(0, -1, info->state->semaphores_id); // enter smph

            if (*((size_t *)info->shared_memory + 2) == 1)
                *((size_t *)info->shared_memory + 2) = 0;
            else
                *((size_t *)info->shared_memory + 2) = 1;

            update_semaphore(0, 1, info->state->semaphores_id); // exit smp
        }
        return;
    }

    update_semaphore(0, -1, info->state->semaphores_id); // enter smph

    char *value = (char *)info->field + y * FIELD_WIDTH + x;
    if (*value == WALL)
        *value = 0;
    else if (*value == 0)
        *value = WALL;

    update_semaphore(0, 1, info->state->semaphores_id); // exit smph
}

static mlx_image_t *init_images(struct drawer_info *info)
{
    mlx_image_t *img = mlx_new_image(info->mlx, 1024, 1000);
    if (!img)
        exit_mlx(info->mlx, info);

    if (mlx_image_to_window(info->mlx, img, 0, 0) < 0)
        exit_mlx(info->mlx, info);

    mlx_texture_t *cursor = mlx_load_png("textures/hammer.png");
    if (!cursor)
        exit_mlx(info->mlx, info);

    mlx_set_cursor(info->mlx, mlx_create_cursor(cursor));

    mlx_delete_texture(cursor);

    info->exit_button = draw_image(info, "textures/exit.png", FIELD_WIDTH * 32 + 35, 10);
    info->play_button = draw_image(info, "textures/play.png", FIELD_WIDTH * 32 + 30, 90);

    info->players = draw_string(info, "Players", info->player_count, 12, 32 * FIELD_HEIGHT + 10);
    info->teams = draw_string(info, "Teams", info->team_count, 12, 32 * FIELD_HEIGHT + 40);
    info->started = draw_string(info, "Started", info->is_started, 12, 32 * FIELD_HEIGHT + 70);

    return img;
}

int display_shared_memory(const t_state *state, const void *shared_memory)
{
    mlx_t *mlx = mlx_init(1024, 1000, "lem-ipc", true);
    if (!mlx)
        exit_error(state, "Could not open the window", CLEANUP);

    update_semaphore(0, -1, state->semaphores_id); // enter smph

    struct drawer_info info;
    info.mlx = mlx;
    info.state = state;
    info.shared_memory = shared_memory;
    info.player_count = *(size_t *)shared_memory;
    info.team_count = *((size_t *)shared_memory + 1);
    info.is_started = *((size_t *)shared_memory + 2);
    info.field = (size_t *)shared_memory + 3; // skip player, team count and is_started
    info.players = NULL;
    info.teams = NULL;
    info.started = NULL;
    info.exit_button = NULL;
    info.play_button = NULL;

    mlx_image_t *img = init_images(&info);

    update_semaphore(0, 1, state->semaphores_id); // exit smp

    info.image = img;

    mlx_cursor_hook(mlx, cursor_hook, (void *)&info);
    mlx_mouse_hook(mlx, mouse_hook, (void *)&info);
    mlx_loop_hook(mlx, game_hook, (void *)&info);

    mlx_loop(mlx);

    return 0;
}
