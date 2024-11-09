# simplexpm

simplexpm is a simple C program that displays image files that are in the XPM format. I found program this useful when we were using the format in a computer graphics class but the only options to look at the files was with GIMP or worse: online image converters. This program is extremely lightweight, simple, and intuitive to use.

## Building/Running

To build the program, all you need is `git` (which you obviously have if you're cloning this repo) and a C compiler.

```console
$ git clone https://github.com/trenthuber/simplexpm.git
$ cd simplexpm
$ git submodule update --init --recursive
$ cc -o cbs cbs.c
$ ./cbs
```

This will update all submodules (I use [Raylib](https://www.raylib.com) and my own C build system [cbs](https://github.com/trenthuber/cbs.git)) and then build and run the program.

To run the program after it's been built, just run the executable found in the `./bin` folder.

## Using the program

Simply drag and drop XPM files into the window to view them. The window can be resized and the image will scale accordingly.

To reload the current image press R. To save the current image as a PNG, press S. PNG files are saved to the same folder as the XPM file.

## Unimplemented features
- Not tested for Linux (built on macOS)
- Processing files with HSV or colorname data
- Different color modes (and UI for such modes)
- Hotspots and XPM extensions
