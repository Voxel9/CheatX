## (Unimplemented) PC interface
Currently there's not yet a PC interface that allows for more rapid development of cheats and memory exploration.
For now, in order to view the results from a memory search, you'll need to build a simple program that reads a search cache file (searchX.bin, located at `E:\devkit\dxt`). The cache file can literally just be read as `address -> value -> address -> value` and so on, until you've listed all the results. (Take a peek at one in a hex editor and you'll see what I mean)
