# CheatX by Voxel9
CheatX is a long-overdue, all-in-one FOSS cheat plugin/memory searcher for the Original Xbox, developed using [nxdk](https://github.com/XboxDev/nxdk). The plugin is provided in DXT format, meaning you'll need a debug bios to load it.

## Installation
Place the DXT file onto the HDD at `E:\devkit\dxt\` and reload your debug bios for the plugin to take effect.

## Usage
CheatX communicates over telnet, meaning you can use a telnet CLI to interface with the plugin.
On Windows, you can connect to the Xbox in cmd by entering `telnet <Xbox IP> 731`.
(Some Windows machines may need telnet client enabled under 'Control Panel/Programs/Turn Windows features on or off' in order to use this)

You can always find an up-to-date list of commands for CheatX on the [wiki](https://github.com/Voxel9/CheatX/wiki/Commands-Usage).

## (Unimplemnted) PC interface
Currently there's not yet a PC interface that allows for more rapid development of cheats and memory exploration.
For now, in order to view the results from a memory search, you'll need to build a simple program that reads a search cache file (searchX.bin, located at `E:\devkit\dxt`). The cache file can literally just be read as `address -> value -> address -> value` and so on, until you've listed all the results. (Take a peek at one in a hex editor and you'll see what I mean)

## (Unimplemented) Loading codes via txt file
Also at the moment, there's no way to load cheat codes via a text file or anything yet, so at the moment you can either save addresses/values you've found yourself and manually run `freezemem!` for each code, or wait until I've implemented loading via a text file for ease of use.

## Building
For those who wish to help out, I'm specifically using JayFoxRox's [dxt branch](https://github.com/JayFoxRox/nxdk/tree/dxt) of nxdk, as building DXTs is not yet supported in mainline nxdk. Just [follow the setup guide](https://github.com/JayFoxRox/nxdk/tree/dxt#getting-started).

The below commands should be executed inside the folder that the nxdk folder is present in, so you have the nxdk folder and CheatX folder next to eachother.

```
git clone https://github.com/Voxel9/CheatX.git
cd CheatX/plugin
make
```

Then place the resulting DXT from the `bin` folder into `E:\devkit\dxt` on your Xbox HDD.
