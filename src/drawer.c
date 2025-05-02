#include "../includes/lem_ipc.h"

struct drawer_info
{
    void *field;
    void *shared_memory;
    size_t player_count;
    size_t team_count;
    int mouse_x;
    int mouse_y;
    mlx_image_t *image;
    mlx_t *mlx;
};

static inline uint32_t get_color(const int team)
{
    switch (team)
    {
    case 1:
        return 0xE63946FF;
    case 2:
        return 0x1D3557FF;
    case 3:
        return 0x2A9D8FFF;
    case 4:
        return 0xF4A261FF;
    case WALL:
        return 0xEADDCAFF;
    default:
        return 0x000000FF;
    }
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

static void ft_hook(void *param)
{
    struct drawer_info *info = param;
    char team;

    register short j;
    register short i = 0;
    while (i < FIELD_HEIGHT)
    {
        j = 0;
        while (j < FIELD_WIDTH)
        {
            team = *((char *)info->field + j + i * FIELD_WIDTH);
            draw_tile(info, j * 32, i * 32, get_color((int)team));
            ++j;
        }
        ++i;
    }

    if (*(size_t *)info->shared_memory != info->player_count)
    {
        info->player_count = *(size_t *)info->shared_memory;
        char buffer[23];
        sprintf(buffer, "Players: %ld", info->player_count);

        mlx_put_string(info->mlx, buffer, 12, 32 * FIELD_HEIGHT + 10);
    }

    if (*((size_t *)info->shared_memory + 1) != info->team_count)
    {
        info->team_count = *((size_t *)info->shared_memory + 1);
        char buffer[23];
        sprintf(buffer, "Teams: %ld", info->team_count);

        mlx_put_string(info->mlx, buffer, 12, 32 * FIELD_HEIGHT + 40);
    }
}

static void cursor_hook(double xpos, double ypos, void *param)
{
    struct drawer_info *info = param;
    int x_max_pixel = FIELD_WIDTH * 32;
    int y_max_pixel = FIELD_HEIGHT * 32;

    if (xpos > x_max_pixel || ypos > y_max_pixel)
        return;

    info->mouse_x = xpos;
    info->mouse_y = ypos;
}

static void mouse(mouse_key_t button, action_t action, modifier_key_t mods, void *param)
{
    if (button != 0 || action == 0)
        return;

    (void)mods;
    struct drawer_info *info = param;

    int x = (info->mouse_x + 10) / 32;
    int y = (info->mouse_y + 10) / 32;

    update_semaphore(0, -1); // enter smph

    char *value = (char *)info->field + y * FIELD_WIDTH + x;
    if (*value == WALL)
        *value = 0;
    else if (*value == 0)
        *value = WALL;

    update_semaphore(0, 1); // exit smph
}

int display_shared_memory(void *shared_memory)
{
    mlx_t *mlx = mlx_init(1024, 1000, "lem-ipc", true);
    if (!mlx)
        exit_error("Could not open the window");

    mlx_image_t *img1 = mlx_new_image(mlx, 1024, 1000);
    if (mlx_image_to_window(mlx, img1, 0, 0) < 0)
        exit_error("Could not display the image");

    struct drawer_info info;
    info.mlx = mlx;
    info.image = img1;
    info.shared_memory = shared_memory;
    info.player_count = *(size_t *)shared_memory;
    info.team_count = *((size_t *)shared_memory + 1);
    info.field = (size_t *)shared_memory + 2; // skip player and team count

    mlx_texture_t *cursor = mlx_load_png("hammer.png");
    mlx_set_cursor(mlx, mlx_create_cursor(cursor));

    mlx_cursor_hook(mlx, cursor_hook, (void *)&info);
    mlx_mouse_hook(mlx, mouse, (void *)&info);
    mlx_loop_hook(mlx, ft_hook, (void *)&info);

    mlx_loop(mlx);
    return 0;
    // mlx_terminate(mlx);
}
