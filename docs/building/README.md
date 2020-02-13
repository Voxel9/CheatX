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

Place the resulting DXT from the `bin` folder into `E:\devkit\dxt` on your Xbox HDD.
