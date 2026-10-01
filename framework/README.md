# CarsonSH Framework

CarsonSH Framework is the native customization layer for CarsonSH, inspired by the
theme/plugin model used by frameworks such as Oh-My-Fish.

It is intentionally independent: themes and plugins are simple CarsonSH configuration
files rather than copied shell-framework code.

## Layout

- `themes/` — customization presets
- `plugins/` — optional alias/configuration packs
- `install.sh` — installs the framework into `~/.config/carsonsh/`

## Install

From the CarsonSH repository:

```sh
./framework/install.sh
```

The installer enables the starter theme and plugins automatically. Restart CarsonSH
or run:

```
reload
```

## Built-in framework commands

```
theme
theme carson-green
plugin
plugin git
```

## Creating a plugin

Create `~/.config/carsonsh/plugins/myplugin.conf`:

```
# CarsonSH plugin: myplugin
alias gs=git status
alias c=clear
```

Then load it:

```
plugin myplugin
```

## Creating a theme

Create `~/.config/carsonsh/themes/mytheme.conf`.

Themes currently use the native CarsonSH prompt and can provide aliases/configuration.
This keeps the framework safe and lightweight while leaving room for richer native
theme APIs later.

CarsonSH Framework is independent and is not affiliated with Oh-My-Fish.
