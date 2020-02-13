[Return to table of contents](../README.md)

## Loading codes via a .txt file
Like most cheat plugins for consoles, CheatX supports loading codes via a .txt file.

Place your .txt files into `E:\DEVKIT\dxt\CheatX\txtcodes\`.
Files must be named as the game's Title ID, in hex format and uppercase.

Jeltaqq's [Xbox Game List](https://github.com/jeltaqq/Xbox-Original-GameList/blob/master/Xbox%20Original%20GameList.tsv) is a good resource covering Title IDs for most games. (If your game's Title ID isn't in that list, there's some other ways of obtaining it, but that's beyond the scope of this readme)

Markdown for the .txt files is currently as follows:
```
# <TEXT> - Comments
$ <TEXT> - Code Title
0xxxxxxx xxxxxxxx (arg 1 = <ADDR>, arg 2 = <VALUE>) - Freeze value at address
```
