#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include <math.h>

// Size of the output window, NOT the size of the frambuffer
#define WINDOW_SIZE 512
#define PIXEL_SCALE (WINDOW_SIZE / 16)

#define SAMPLE_RATE 48000
#define AMPLITUDE   28000
uint8_t current_note = 0;

/**
 * Plays a sine wave generated from the given MIDI pitch number.
 * The note has a minimum duration of approximately 166ms (arbitary)
 * and an attack-decay envelope to mitigate audio clicking.
 *
 * @param dev The SDL audio device ID to play the note through
 * @param pitch MIDI pitch number (69 = A4 = 440Hz)
 */
void play_note(SDL_AudioDeviceID dev, uint8_t pitch) {
  // Clear any remaining audio
  SDL_ClearQueuedAudio(dev);

  double freq = 440.0 * pow(2.0, ((int)pitch - 69.0) / 12.0);
  int samples = SAMPLE_RATE / 6; // ~166ms duration
  int16_t *buf = malloc(samples * sizeof(int16_t));
  if (!buf) return;

  // Apply envelope to prevent clicks
  for (int i = 0; i < samples; i++) {
    double t = (double)i / SAMPLE_RATE;
    // Attack-decay envelope
    double envelope = 1.0;
    if (i < SAMPLE_RATE / 50) { // 20ms attack
      envelope = (double)i / (SAMPLE_RATE / 50);
    } else if (i > samples - SAMPLE_RATE / 50) { // 20ms release
      envelope = (double)(samples - i) / (SAMPLE_RATE / 50);
    }
    buf[i] = (int16_t)(AMPLITUDE * envelope * sin(2.0 * M_PI * freq * t));
  }

  SDL_QueueAudio(dev, buf, samples * sizeof(int16_t));
  free(buf);
}

/**
 * The program array consists of 16-bit byte pairs. The first byte
 * of a pair is a char (Brainfuck operator), and the second denotes
 * either the length of the character sequence or, in the case of
 * loops, the distance to the other bracket in this array.
 */
uint16_t program[16777216] = {0};
// As per Brainfuck "spec", 30000 cells are allocated for memory.
uint8_t memory[30000] = {0};

// `cursor` is the program counter, `address` is the memory pointer.
size_t program_size = 0, cursor = 0;
int address = 0;

// Runs the Brainfuck program until EOF or until `.` is encountered.
void run_program () {
  while (cursor < program_size) {
    switch (program[cursor++]) {
      case '>':
        address += program[cursor++];
        break;
      case '<':
        address -= program[cursor++];
        break;
      case '+': memory[address] += program[cursor++]; break;
      case '-': memory[address] -= program[cursor++]; break;
      case '[':
        if (!memory[address]) {
          cursor += program[cursor];
        }
        cursor++;
        break;
      case ']':
        if (memory[address]) {
          cursor -= program[cursor];
        }
        cursor++;
        break;
      case '.':
        cursor++;
        return;
      case ',':
        cursor++;
        uint8_t key = 0;
        const uint8_t *keystate = SDL_GetKeyboardState(NULL);
        if (keystate[SDL_SCANCODE_Z]) key |= 0x80;
        if (keystate[SDL_SCANCODE_X]) key |= 0x40;
        if (keystate[SDL_SCANCODE_RETURN]) key |= 0x20;
        if (keystate[SDL_SCANCODE_SPACE]) key |= 0x10;
        if (keystate[SDL_SCANCODE_UP]) key |= 0x08;
        if (keystate[SDL_SCANCODE_DOWN]) key |= 0x04;
        if (keystate[SDL_SCANCODE_LEFT]) key |= 0x02;
        if (keystate[SDL_SCANCODE_RIGHT]) key |= 0x01;
        memory[address] = key;
        break;
      case '?':
        cursor++;
        printf("memory[%d]: %d\n", address, memory[address]);
        break;
      default:
        printf("unexpected char '%c'\n", program[cursor - 1]);
        break;
    }
  }
}

// Returns `true` if input is a valid Brainfuck char, `false` otherwise
// Also responds to "?", which is a runtime-specific debug character
int isBFChar (char a) {
  return (a == '>' || a == '<' || a == '+' || a == '-' ||
    a == '[' || a == ']' || a == '.' || a == ',' ||
    a == '?');
}

int main (int argc, char *argv[]) {

  if (argc != 2) {
    printf("Usage: %s <filename>\n", argv[0]);
    return 1;
  }

  FILE *file = fopen(argv[1], "rb");
  if (!file) {
    printf("Failed to open file: %s\n", argv[1]);
    return 1;
  }

  /**
   * Builds the `program` array.
   *
   * For special characters (. , ?), the second byte of the pair is left
   * empty. For simple characters (> < + -), the second byte denotes the
   * length of the character chain. For loops, the opening bracket is
   * initially ignored. The closing bracket steps backwards through
   * `program` to find the opening bracket, and assigns the distance to
   * both of the brackets.
   */
  char ch, tmp;
  while (fread(&ch, 1, 1, file) == 1) {
    switch (ch) {
      case '.':
      case ',':
      case '?':
      case '[':
        program[program_size] = ch;
        program_size += 2;
        break;
      case '>':
      case '<':
      case '+':
      case '-':
        program[program_size++] = ch;
        program[program_size] = 1;
        int found = 0;
        while (fread(&tmp, 1, 1, file) == 1) {
          if (!isBFChar(tmp)) continue;
          if (tmp != ch) {
            fseek(file, -1, SEEK_CUR);
            found = 1;
            break;
          }
          program[program_size]++;
        }
        if (!found) fseek(file, -1, SEEK_CUR);
        program_size++;
        break;
      case ']':
        program[program_size] = ch;
        int depth = 1;
        int i = program_size;
        while (i > 0 && depth > 0) {
          i -= 2;
          if (program[i] == '[') depth--;
          if (program[i] == ']') depth++;
        }
        program[i + 1] = program_size - i;
        program[program_size + 1] = program_size - i;
        program_size += 2;
        break;
      default:
        break;
    }
  }
  fclose(file);

  // Write out a binary file for debugging (and because it's cool!)
  FILE *out = fopen("program.bin", "wb");
  for (size_t i = 0; i < program_size; i += 2) {
    uint8_t command = program[i];
    fwrite(&command, 1, 1, out);
    fwrite(&program[i + 1], 2, 1, out);
  }
  fclose(out);

  // Initialize SDL for rendering and create the window...
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("SDL initialization failed: %s\n", SDL_GetError());
    return 1;
  }

  SDL_Window *window = SDL_CreateWindow(
    "BF16",
    SDL_WINDOWPOS_CENTERED,
    SDL_WINDOWPOS_CENTERED,
    WINDOW_SIZE, WINDOW_SIZE,
    SDL_WINDOW_SHOWN
  );
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

  // Initialize SDL for audio and open an audio device...
  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    SDL_Log("SDL_Init error: %s", SDL_GetError());
    return 1;
  }

  SDL_AudioSpec want, have;
  SDL_zero(want);
  want.freq     = SAMPLE_RATE;
  want.format   = AUDIO_S16SYS;
  want.channels = 1;
  want.samples  = 1024;

  SDL_AudioDeviceID dev = SDL_OpenAudioDevice(
    NULL, 0, &want, &have, 0
  );
  if (!dev) {
    SDL_Log("SDL_OpenAudioDevice error: %s", SDL_GetError());
    SDL_Quit();
    return 1;
  }
  SDL_PauseAudioDevice(dev, 0);

  int running = 1;
  SDL_Event event;

  uint32_t frame_start, frame_time;
  const int target_frame_time = 16; // 60 FPS

  while (running) {
    frame_start = SDL_GetTicks();

    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = 0;
      }
    }

    // Draw framebuffer as grayscale squares
    for (int i = 0; i < 256; i++) {
      SDL_SetRenderDrawColor(renderer, memory[i], memory[i], memory[i], 255);
      SDL_Rect rect = {
        (i % 16) * PIXEL_SCALE,
        (i / 16) * PIXEL_SCALE,
        PIXEL_SCALE,
        PIXEL_SCALE
      };
      SDL_RenderFillRect(renderer, &rect);
    }

    // Calculate one frame (up to next . instruction)
    run_program();
    // If pointer at a nonzero location, play that as a MIDI note
    if (memory[address] != current_note) {
      current_note = memory[address];
      if (current_note) play_note(dev, current_note);
    }

    // Render the framebuffer
    SDL_RenderPresent(renderer);

    // Try to keep 60 FPS regardless of how long the BF program took
    frame_time = SDL_GetTicks() - frame_start;
    if (frame_time < target_frame_time) {
      SDL_Delay(target_frame_time - frame_time);
    }
  }

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_CloseAudioDevice(dev);
  SDL_Quit();
  return 0;

}
