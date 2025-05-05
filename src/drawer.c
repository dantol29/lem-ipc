#include "../includes/lem_ipc.h"

struct drawer_info
{
    const void *field;
    const void *shared_memory;
    size_t player_count;
    size_t team_count;
    int mouse_x;
    int mouse_y;
    mlx_image_t *players;
    mlx_image_t *teams;
    mlx_image_t *image;
    mlx_t *mlx;
};

static inline uint32_t get_color(const int id)
{
    if (id == WALL)
        return 0xEADDCAFF;
    if (id == 0)
        return 0x000000FF;
    if (id < 4) // 1st team: 0-3
        return 0xE63946FF;
    if (id < 8) // 2nd team: 4-7
        return 0x1D3557FF;
    if (id < 13) // 3rd team: 9-12
        return 0x2A9D8FFF;
    if (id < 20) // 4th team: 16-19
        return 0xF4A261FF;

    return 0x000000FF;
}

static inline void exit_mlx(mlx_t *mlx)
{
    mlx_terminate(mlx);
    cleanup_resources();
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
        info->player_count = *(size_t *)info->shared_memory;
        char buffer[23];
        sprintf(buffer, "Players: %ld", info->player_count);

        if (info->players)
            mlx_delete_image(info->mlx, info->players);
        info->players = mlx_put_string(info->mlx, buffer, 12, 32 * FIELD_HEIGHT + 10);
    }

    if (*((size_t *)info->shared_memory + 1) != info->team_count)
    {
        info->team_count = *((size_t *)info->shared_memory + 1);
        char buffer[23];
        sprintf(buffer, "Teams: %ld", info->team_count);

        if (info->teams)
            mlx_delete_image(info->mlx, info->teams);
        info->teams = mlx_put_string(info->mlx, buffer, 12, 32 * FIELD_HEIGHT + 40);
    }
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
        if (x == 30 && y == 1)
        {
            mlx_terminate(info->mlx);
            exit_error("User clicked on exit", CLEANUP);
        }
        return;
    }

    update_semaphore(0, -1); // enter smph

    char *value = (char *)info->field + y * FIELD_WIDTH + x;
    if (*value == WALL)
        *value = 0;
    else if (*value == 0)
        *value = WALL;

    update_semaphore(0, 1); // exit smph
}

static mlx_image_t *init_images(mlx_t *mlx, struct drawer_info *info)
{
    mlx_image_t *img = mlx_new_image(mlx, 1024, 1000);
    if (!img)
        exit_mlx(mlx);

    if (mlx_image_to_window(mlx, img, 0, 0) < 0)
        exit_mlx(mlx);

    mlx_texture_t *exit = mlx_load_png("textures/exit.png");
    if (!exit)
        exit_mlx(mlx);

    mlx_image_t *img2 = mlx_texture_to_image(mlx, exit);
    if (!img)
        exit_mlx(mlx);

    if (mlx_image_to_window(mlx, img2, FIELD_WIDTH * 32 + 35, 10) < 0)
        exit_mlx(mlx);

    mlx_texture_t *cursor = mlx_load_png("textures/hammer.png");
    if (!cursor)
        exit_mlx(mlx);

    mlx_set_cursor(mlx, mlx_create_cursor(cursor));

    char buffer1[23];
    sprintf(buffer1, "Teams: %ld", info->team_count);

    info->teams = mlx_put_string(info->mlx, buffer1, 12, 32 * FIELD_HEIGHT + 40);
    if (!info->teams)
        exit_mlx(mlx);

    char buffer2[23];
    sprintf(buffer2, "Players: %ld", info->player_count);

    info->players = mlx_put_string(info->mlx, buffer2, 12, 32 * FIELD_HEIGHT + 10);
    if (!info->players)
        exit_mlx(mlx);

    return img;
}

int display_shared_memory(const void *shared_memory)
{
    mlx_t *mlx = mlx_init(1024, 1000, "lem-ipc", true);
    if (!mlx)
        exit_error("Could not open the window", CLEANUP);

    struct drawer_info info;
    info.mlx = mlx;
    info.shared_memory = shared_memory;
    info.player_count = *(size_t *)shared_memory;
    info.team_count = *((size_t *)shared_memory + 1);
    info.field = (size_t *)shared_memory + 2; // skip player and team count

    mlx_image_t *img = init_images(mlx, &info);

    info.image = img;

    mlx_cursor_hook(mlx, cursor_hook, (void *)&info);
    mlx_mouse_hook(mlx, mouse_hook, (void *)&info);
    mlx_loop_hook(mlx, game_hook, (void *)&info);

    mlx_loop(mlx);

    return 0;
}
