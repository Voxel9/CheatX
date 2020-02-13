[Return to table of contents](../README.md)

# Commands Usage

| Command | Description |
| ------- | ----------- |
| **dumpmem!** | Dumps the whole 64MB physical memory map to `E:\DEVKIT\dxt\memdump.bin` |
| **pokemem!** _addr value_ | Writes a specified DWORD (4 bytes) to the specified address. |
| **freezemem!** _addr value_ | Same as pokemem, but instead freezes the specified address with the written value. |
| **startsearch!** _equals/not-equals/less-than/greater-than/unknown value_ | Begins a conditional or unknown search for memory values. Restarts any existing search. |
| **contsearch!** _equals/not-equals/same/different/less-than/greater-than/less/greater value_ | Continues a search for memory values. Execute `startsearch!` first before using this command. |
| **changetype!** _byte/word/dword/float_ | Changes the type of value to search for before starting a search. Size of byte = 1, size of word = 2, size of dword/float = 4. Float is signed, all other types are unsigned.