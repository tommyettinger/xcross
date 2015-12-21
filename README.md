# xCross
xCross is a tool that enable you to build standalone binaries from an haXe/Neko program. By running a single command,
xCross will create three binaries, for Windows, OSX (Universal) and Linux with no required dependencies.

To install :

- install haXe + Neko from http://haxe.org (~~needs to be 1.0~~ never mind, master seems to be ported to haXe 3.0)
- run "haxelib install xcross" (may require 1.0 if you aren't building from source... like a pro)

To run :

- run "haxelib run xcross myfile.n"
- this will produce three standalone executables "myfile-win.exe", "myfile-osx" and "myfile-linux"
- you can create an OSX bundle instead of a binary with "haxelib run xcross -bundle MyBundle myfile.n"

xCross includes / statically link :
- the NekoVM itself (neko.dll or libneko.so)
- the Boehm GC used by NekoVM (gc.dll or libgc.so)
- the Neko standard library (std.ndll) which contains filesystem access, sockets, threads, etc.
- the zlib.ndll and regexp.ndll libraries (for zip and regular expressions support)
- a [very tiny UI toolkit used by haXe Installer](https://github.com/tommyettinger/xcross/tree/master/xcross)

If you want to use additional libraries, you'll have to make a custom build of xCross.
