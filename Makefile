CC := arm-linux-gnueabihf-gcc
CFLAGS := -I./include

# Source files
SRCS := \
	main.c \
	src/button.c \
	src/snake_game.c \
	src/ssd1306.c

# Object files (giữ nguyên đường dẫn)
OBJS := $(SRCS:.c=.o)

all: main

# Compile rule (áp dụng cho cả main.c và src/*.c)
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

# Link
main: $(OBJS)
	$(CC) $^ -o $@

clean:
	rm -f $(OBJS) main
