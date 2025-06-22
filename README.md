# bf16
The first ever *visual* Brainfuck runtime built for running interactive games.

## Usage
In order to use this runtime, you will have to compile it using a Linux distribution. 

### Requirements 
A C compiler (`gcc`), `make` and Simple DirectMedia Layer (SDL2) development packages are required. 

You can install them with one of the following commands. 

On RHEL-based distributions (includes Fedora). 

```bash
sudo dnf install gcc make SDL2-devel
```

On Debian-based distributions (includes Ubuntu and Mint). 

```bash
sudo apt install gcc make libsdl2-dev
```

On Arch-based distributions (includes EndeavourOS, CachyOS and Manjaro). 

```bash
sudo pacman -S gcc make sdl2
```

If your Linux distribution is immutable (such as Fedora Atomic, Bazzite or SteamOS), you can use [Distrobox](https://distrobox.it/). 

### Compiling 
Simply run the following commands inside a personal directory. 

```bash
git clone https://github.com/p2r3/bf16
```

```bash
cd bf16
```

```bash
make
```

This will produce two binaries — `bf16` and `bf16_grayscale`. The former uses an RGB332 palette, the latter renders in grayscale. Test with the programs under `examples/`. Note that `badapple.b` is best used with `bf16_grayscale`.

### Example usage 
```bash
./bf16 examples/snake.b
```

```bash
./bf16_grayscale examples/badapple.b
```

## Contributing
Submit issues before pull requests (discuss, then code), and adhere to the code style. Ideally, use the [conventional commits](https://www.conventionalcommits.org/) naming scheme. Code that doesn't respect the style/conventions of the code base will not be accepted.
