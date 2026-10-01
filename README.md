# CarsonSH 🐚

CarsonSH is a small, experimental Unix shell written in C.

The goal is to build a real shell from the ground up instead of wrapping an existing shell. CarsonSH is intended to run on Linux systems including Debian/Ubuntu-based distributions and Arch-based distributions such as EndeavourOS.

## Current status

**Early development — v0.1**

- Interactive prompt
- Kali-style green prompt and banner
- Git branch shown in the prompt when inside a Git repository
- Built-in `cd`
- Built-in `pwd`
- Built-in `echo`
- Built-in `alias` and `unalias`
- Built-in `history` with persistent history storage
- Built-in `reload` for reloading configuration
- Built-in `help`
- External command execution through `PATH`
- Basic command arguments
- User configuration at `~/.config/carsonsh/config`
- History stored at `~/.carsonsh_history`
- Consecutive duplicate history entries are ignored

## Oh-My-Shell-inspired customization

CarsonSH includes a native C implementation inspired by documented ideas from KasRoudra's Oh-My-Shell project: aliases, a customized prompt, a banner, and terminal customization.

See [CREDITS.md](CREDITS.md) for attribution and licensing information.

CarsonSH intentionally does not execute or copy the original Oh-My-Shell installer's obfuscated payload. Features are implemented directly for CarsonSH instead.

## caarlos0/dotfiles.zsh-inspired shell behavior

CarsonSH also takes inspiration from shell configuration practices documented in [caarlos0/dotfiles.zsh](https://github.com/caarlos0/dotfiles.zsh), especially persistent history, ignoring consecutive duplicate history entries, and a convenient configuration reload command.

These features are independently implemented in C; CarsonSH does not copy the upstream Zsh configuration files. See [CREDITS.md](CREDITS.md).

### Configuration

Copy the example configuration:

```bash
mkdir -p ~/.config/carsonsh
cp config/carsonsh.conf.example ~/.config/carsonsh/config
```

Then start CarsonSH. Aliases from the file are loaded automatically.

Examples:

```text
alias ll=eza -la
alias la=eza -a
alias cls=clear
alias reload=reload
```

You can also create an alias during a session:

```text
alias ll=eza -la
unalias ll
```

### History

CarsonSH keeps up to 1000 history entries in `~/.carsonsh_history`.

```text
history
history 50
```

The default `history` output shows the 20 most recent commands. Consecutive duplicate commands are not added again.

### Reloading configuration

After editing `~/.config/carsonsh/config`, reload aliases without restarting CarsonSH:

```text
reload
```

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
./build/carsonsh
```

> **Note:** Depending on how you obtained the repository or which filesystem you are using, the shell scripts may not have their executable bit set. If you get a **Permission denied** error, run:
>
> ```bash
> chmod +x build.sh install.sh uninstall.sh
> ```

Install locally:

```bash
./install.sh
```

This installs to `~/.local/bin/carsonsh` by default.

## Debian package

GitHub Actions automatically builds a Debian package on pushes to `main`. Releases include CarsonSH and the Ubuntu Condensed font support.

```bash
sudo apt install ./carsonsh_0.1.X_amd64.deb
```

## Roadmap

- [x] Interactive shell loop
- [x] Basic built-ins
- [x] External commands
- [x] Aliases
- [x] Custom configuration
- [x] Git-aware prompt
- [x] Persistent command history
- [ ] Pipes
- [ ] Redirection
- [ ] Better line editing
- [ ] Interactive auto-suggestion
- [ ] Interactive syntax highlighting
- [ ] Shell variables
- [ ] Functions
- [ ] Job control
- [ ] Shell scripting

## Why?

Because making a shell sounds fun. 🐧

CarsonSH is a learning project and will probably get weird along the way.

## License

MIT
