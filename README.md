# CarsonSH 🐚

CarsonSH is a small, experimental Unix shell written in C.

The goal is to build a real shell from the ground up instead of wrapping an existing shell. CarsonSH is intended to run on Linux systems including Debian/Ubuntu-based distributions and Arch-based distributions such as EndeavourOS.

## Current status

**Early development — v0.1**

- Interactive prompt
- Built-in `cd`
- Built-in `pwd`
- Built-in `echo`
- Built-in `exit`
- External command execution through `PATH`
- Basic command arguments

## Building

On Debian/Ubuntu/Linux Lite:

```bash
sudo apt install build-essential
```

On Arch/EndeavourOS:

```bash
sudo pacman -S base-devel
```

Then:

```bash
./build.sh
./carsonsh
```

Install locally:

```bash
./install.sh
```

This installs to `~/.local/bin/carsonsh` by default.

## Roadmap

- [x] Interactive shell loop
- [x] Basic built-ins
- [x] External commands
- [ ] Pipes
- [ ] Redirection
- [ ] Command history
- [ ] Better line editing
- [ ] Aliases
- [ ] Shell variables
- [ ] Functions
- [ ] Job control
- [ ] Shell scripting
- [ ] Configuration file

## Why?

Because making a shell sounds fun. 🐧

CarsonSH is a learning project and will probably get weird along the way.

## License

MIT
