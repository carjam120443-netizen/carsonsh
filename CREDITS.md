# CarsonSH credits

## Oh-My-Shell

CarsonSH's optional shell customization features are **inspired by and adapted from ideas documented by KasRoudra's Oh-My-Shell project**:

https://github.com/KasRoudra/oh-my-shell

Oh-My-Shell is MIT licensed. Its documented features include a customized prompt, aliases, banner, fonts, and interactive shell enhancements.

CarsonSH does **not** copy or execute the original project's installer payload. In particular, the original installer contains an obfuscated/evaluated section that is intentionally not carried into CarsonSH. CarsonSH implements the compatible features natively in C.

Copyright notice for the original project is retained here as attribution; see the original repository for its complete license text.

## caarlos0/dotfiles.zsh

CarsonSH's persistent history and configuration-reload ideas are **inspired by the shell configuration practices documented in caarlos0/dotfiles.zsh by caarlos0**:

https://github.com/caarlos0/dotfiles.zsh

In particular, CarsonSH takes inspiration from its Zsh configuration's persistent history settings, duplicate-history handling, and shell reload alias. CarsonSH reimplements these ideas independently in C rather than copying the Zsh configuration files.

The original repository includes its own license; see the upstream repository for the complete license text.

## Other components

CarsonSH also uses and/or is inspired by standard Unix concepts and ANSI terminal escape sequences. The Ubuntu Condensed font bundled by the Debian package is distributed under its own Ubuntu Font Licence.

## Additional thanks

Special thanks to **caarlos0** for the ideas and configuration examples in dotfiles.zsh that helped inspire CarsonSH's persistent history and reload features.

CarsonSH is an independent project and is not affiliated with or endorsed by the upstream projects mentioned above.


## Prompt theme inspiration

The CarsonSH Framework includes native prompt styles inspired by documented styles from other shell prompt projects:

- **Robby Russell / Oh My Zsh** — the compact arrow-and-directory concept is inspired by the Robby theme.
- **Agnoster / Oh My Zsh** — the segmented two-line prompt concept is inspired by Agnoster.
- **Pure** — the minimal directory-and-arrow concept is inspired by Pure.
- **Spaceship** — the expressive cross-shell prompt concept is inspired by Spaceship.
- **Powerlevel10k** — the compact two-line layout concept is inspired by Powerlevel10k.

These CarsonSH themes are independently implemented in C/configuration and do not copy upstream theme source code. Theme names are used to identify the styles they are inspired by.
