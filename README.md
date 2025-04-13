# simplexpm

simplexpm is a simple GUI application used to view XPM image files. The entire application is written in C, even down to the build system.

## Building

This repository uses submodules, so you'll need to clone it recursively.

```console
$ git clone --recursive https://github.com/trenthuber/simplexpm
$ cd simplexpm
```

simplexpm uses [cbs](https://github.com/trenthuber/cbs) as its build system. cbs uses C source code as its build files, so the only thing you need to build this project is a C compiler. The first step is to bootstrap the build system by compiling the build file located at the root level of the repository.

```console
$ cc -o build build.c
```

Running the `build` executable we just generated will build the entire project.

```console
$ ./build
```

Once built, the application will be located in the `bin/` folder. It should be noted that once the application is created it can be moved/copied around the file system as the binary itself has all its assets baked in and doesn't depend on anything in the repository once built.

```console
$ ./bin/simplexpm
```

## Usage

Files can either be provided at the command line with the `-f` flag or dragged into the application once it's opened. Once content is loaded, the window can be resized and the content will scale accordingly.

XPM files can be exported to PNG by pressing `s` (PNG images are exported to the same directory the XPM file is in).

If modifications are made to the XPM file while loaded, hitting `r` reloads the content with the new modifications in place.

Finally, with an XPM file loaded, color modes can be changed by pressing corresponding keys (see a complete list of key bindings by using the `-h` flag).

```console
$ ./bin/simplexpm -f images/test.xpm
```
![Here's an image of the application running on my machine.](images/application.png "A simplexpm window")

## Theory of Operation

Since valid XPM files are necessarily valid C source code, the most straightforward way to process the data is to just compile the file. Integrating that into an interactive application involves the use of dynamic libraries-luckily that's nothing cbs can't handle.

The first phase of the application is to get the path name of the XPM file from the user. Once that's done, we copy the file to a temporary file, compile and link that file to a dynamic library, and then open and load the symbols from the file *all during the runtime of the application* (this process can be seen if you enable running with debug information via the `-d` flag). These symbols have all the data we need to display the image without having to parse almost any of the file itself (we do a bit of parsing to know what symbol name to call, but that's about it). Of course, we still have to parse the actual data stored in the string array, but that's very straightforward since the core standard is relatively simple (I used chapter 2 of [this manual](https://www.xfree86.org/4.8.0/xpm.pdf) as reference).
