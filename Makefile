CC = gcc
CFLAGS = -lSDL2 -lm

all: bf16 bf16_grayscale

bf16:
	$(CC) bf16.c -o bf16 $(CFLAGS)

bf16_grayscale:
	$(CC) bf16_grayscale.c -o bf16_grayscale $(CFLAGS)

clean:
	rm -f bf16 bf16_grayscale
