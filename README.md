# VE482-Mumsh
*A self-made shell program named after "Mum" for VE482-SU20 Project 1.*

## Supported features
1. parsing and executing pre-installed user programs
2. parsing arguments
3. File I/O redirection (`>`, `>>`, `<`, combination of them)
4. pipes (`|`, combination of different operators)
5. Ctrl-D
6. Ctrl-C
7. parsing quotes (`'`, `"`, combination of different operators)
8. additional prompts on incomplete operators
9. error handling
10. parsing background operators (`&`, combination with pipes)

## Supported build-in commands
| keyword | \# task | description |
| ------- | ------- | ----------- |
| exit    | 1       | exit memsh  |
| pwd     | 7       | display current directory  |
| cd      | 7       | change directory |
| jobs    | 13      | display list of background tasks |

## Known issues
1. for empty input filename, the error handler will fail.
2. when choosing gnu11 to compile, a warning "implicitly declaring library function 'memset'" shows even if `string.h` is included.

## Notes
I've attached a notebook `NOTES.md` that might be helpful to beginners that are interested in writing a customized shell program.

## Honor Code
This repo is an educational project. If there is similar course materials assigned in the future, it is the responsibility of JI students not to copy or modify these codes, or MD files because it is against the Honor Code. The owner of this repository doesn't take any commitment for other's faults.
