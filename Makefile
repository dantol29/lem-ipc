NAME = ipc
CC = gcc
CFLAGS = -O3 -Wall -Wextra -Werror -ffast-math -I./include -I$(LIBMLX)/include
FSANITIZE = -fsanitize=address
LIBMLX := lib_mlx42
LIBS := $(LIBMLX)/build/libmlx42.a -ldl -L/opt/homebrew/lib -lglfw -pthread -lm 
SOURCES_M := src/main.c src/semaphores.c src/queue.c src/utils.c src/drawer.c src/player.c src/a_star.c

OBJECTS := $(SOURCES_M:.c=.o)

$(NAME): $(LIBMLX)/build/libmlx42.a $(OBJECTS)
		$(CC) $(CFLAGS) $(OBJECTS) $(LIBS) $(FSANITIZE) -o $(NAME)

all: $(NAME)

clean:
		$(RM) $(OBJECTS)

fclean: clean
		$(RM) $(NAME) $(NAME)
		@rm -rf lib_mlx42/build/

re: fclean all

$(LIBMLX):
		@echo "Downloading MLX42 library..."; \
		git clone https://github.com/codam-coding-college/MLX42.git lib_mlx42; \
		rm -rf lib_mlx42/.git;

$(LIBMLX)/build/libmlx42.a: $(LIBMLX)
		@echo "Building mlx42..."
		@cd lib_mlx42; cmake  -DDEBUG=1 -B build; cmake --build build;

.PHONY: all clean fclean re bonus
