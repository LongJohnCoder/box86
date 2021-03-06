# box86

Linux Userspace x86 Emulator with a twist

Box86 will let run x86 Linux program (games) on non-x86 Linux, like ARM (needs to be 32bits little-endian).

Also, Box86 use native version for some "system" libraries, like libc, libm, or SDL and OpenGL, leading to more performance and easier integration with host system.

Most x86 Games needs OpenGL, so on ARM platform, a solution like [gl4es](https://github.com/ptitSeb/gl4es) is probably needed.

Note that current version of Box86 doesn't feature any form of JIT/Dynarec: expect everything to be slow.

Current version is higly experimental and early, and most stuff wont run and run correctly. For example, WorldOfGoo does run correctly (at least on the Pandora), Airline Tycoon Deluxe runs too. but if FTL runs, the music is distorted...

If you are serious about developping Box86, you should install ccache and activate it's support in the cmake project (use ccmake for example)
To have the TRACE enabled (i.e. dumping to stdout all individual x86 instruction execute, with dump of registers), you'll also need [Zydis library](https://github.com/zyantific/zydis) accessible on your system.

Some x86 internal opcode use parts of "Realmode X86 Emulator Library", see [x86primop.c](src/x86primop.c) for copyright details

Here are 2 videos, one of "Airline Tycoon Deluxe" running on an gigahertz OpenPandora, and the other of "Bit.Trip.Runner" running on an ODroid XU4.

[![Play on Youtube](https://img.youtube.com/vi/bLt0hMoFDLk/3.jpg)](https://www.youtube.com/watch?v=bLt0hMoFDLk) [![Play on Youtube](https://img.youtube.com/vi/8hr71S029Hg/1.jpg)](https://www.youtube.com/watch?v=8hr71S029Hg)

----

Compiling
----
How to compile can be found [here](COMPILE.md)

----

Usage
----

There are a few environnement variable to control Box86 behavour.

See [here](USAGE.md) for all variables and what they do.

----

Version history
----

The change log is [here](CHANGELOG.md)


(If you use Box86 in your project, please don't forget to mention Box86)
