# CarsonSH command cheat sheet

This page is a quick reference for commands currently available in CarsonSH.

> CarsonSH is still experimental. Commands listed as built-ins are handled directly by CarsonSH. Other commands are started as external programs found through `PATH`.

## Start here

| Command | What it does |
| --- | --- |
| `help` | Show the built-in command list. |
| `pwd` | Print the current working directory. |
| `cd DIR` | Change directories. |
| `cd ~` | Change to your home directory. |
| `echo TEXT` | Print text. |
| `exit` | Exit CarsonSH. |
| `exit 1` | Exit with status code 1. |

Examples:

```text
help
pwd
cd ..
cd ~
echo Hello from CarsonSH
```

## System information

These three commands are built into CarsonSH:

| Command | What it shows |
| --- | --- |
| `cpufetch` | CPU model, logical CPU count, and a short usage sample where supported. |
| `gpufetch` | Detected GPU and available utilization or memory details. |
| `displayfetch` | Connected display modes and refresh rates where supported. |

Try them:

```text
cpufetch
gpufetch
displayfetch
```

Some details depend on the operating system and installed utilities. For example, Linux GPU details may use `nvidia-smi` or `lspci`.

## Aliases

Create an alias with `NAME=VALUE`:

```text
alias ll=ls -la
alias cls=clear
```

List aliases:

```text
alias
```

Remove one:

```text
unalias ll
```

Aliases are also read from `~/.config/carsonsh/config`.

## Command history

CarsonSH saves command history in `~/.carsonsh_history`.

```text
history
history 50
```

The default command shows the 20 most recent entries. Consecutive duplicate commands are skipped.

## Themes and plugins

Show the active theme:

```text
theme
```

Select a theme:

```text
theme minimal
theme pure
```

List loaded plugins:

```text
plugin
```

Load a plugin:

```text
plugin git
```

The available themes and plugins depend on the framework files installed for your CarsonSH setup.

## External commands

CarsonSH can launch programs found through your system's `PATH`. For example:

```text
ls
date
uname -a
whoami
clear
```

Availability depends on your operating system and installed programs. On Windows, use commands available in the environment used to run the CarsonSH build.

## Configuration

Your user configuration is:

```text
~/.config/carsonsh/config
```

Create the directory and copy the example:

```sh
mkdir -p ~/.config/carsonsh
cp config/carsonsh.conf.example ~/.config/carsonsh/config
```

Example settings:

```text
alias ll=ls -la
alias cls=clear
welcome on
```

CarsonSH prints `welcome to carsonsh` at startup by default, regardless of the selected prompt theme. To hide that line, add this to your config:

```text
welcome off
```

To show it again:

```text
welcome on
```

After editing the configuration, run:

```text
reload
```

## Current limitations

CarsonSH is not yet a full POSIX-compatible shell. Native shell scripting, variables, functions, pipes, output redirection, job control, and advanced command parsing are still on the roadmap.

The `.sh` and `.ps1` files in this folder are examples for external interpreters, not scripts interpreted natively by CarsonSH.
