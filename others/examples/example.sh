#!/usr/bin/env sh
#
# CarsonSH example: run this file with:
#   sh others/examples/example.sh
#
# CarsonSH currently runs external programs, but does not yet interpret
# .sh/.ps1 files as native CarsonSH scripts. This file is a POSIX shell
# script and is launched through sh.

printf '\n=== Welcome to CarsonSH ===\n'
printf 'This is a small command-line tour.\n\n'

printf '%s\n' '1. Show the current directory:'
pwd

printf '\n%s\n' '2. Print a message:'
echo "Hello from a script!"

printf '\n%s\n' '3. Show the current date:'
date

printf '\n%s\n' '4. Show basic system information:'
uname -s
uname -m

printf '\n%s\n' '5. List files in the current directory:'
ls

printf '\n%s\n' '6. CarsonSH built-ins to try interactively:'
printf '%s\n' '  help'
printf '%s\n' '  history'
printf '%s\n' '  cpufetch'
printf '%s\n' '  gpufetch'
printf '%s\n' '  displayfetch'

printf '\nDone! Try those built-ins directly at the CarsonSH prompt.\n'
