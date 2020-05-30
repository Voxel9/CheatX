[Return to table of contents](../README.md)

## Building
[Follow this setup guide](https://github.com/XboxDev/nxdk/wiki/Install-the-Prerequisites) to first install nxdk dependencies.

Then run the following commands in the folder you wish to clone CheatX to:
```
git clone --recurse-submodules https://github.com/Voxel9/CheatX.git
cd CheatX/plugin/nxdk/tools/cxbe
make
cd ../../..
make
```

### IMPORTANT:
Currently, clang + lld versions 9 and above produce non-working DXT files. Ensure that you install clang + lld 8.0.1 packages until compatibility with the new versions is added (clang + lld 7.0.0 have also been tested and working).

Place the resulting DXT from the `bin` folder into `E:\devkit\dxt` on your Xbox HDD.
