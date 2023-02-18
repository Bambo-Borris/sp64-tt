# SP 64 Texture Tool

## What is it? 
Tool to unpack & repack textures for the South Park N64 game.

## Current Progress
- [x] Unpack textures (without labelling/tagging)
- [ ] Unpack textures (with sensible labelling/tagging for repacking functionality)
- [ ] Repack textures into .tex file 

## How to use it (CLI only for now)
To unpack: 
```
./sp64-tt win32.tex
```

### Windows (Prebuilt)
- Grab prebuilt .zip from the Releases section


### Windows/Linux/MacOS (From Source)
```
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF
cmake --build build
```
