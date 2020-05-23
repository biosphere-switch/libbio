# libbio

libbio is Biosphere's userland homebrew library.

The library has literally zero dependencies - not even standard C/C++ libraries or headers. Therefore, it provides its own implementations of common C/C++ features (`mem:::SharedObject`, `util::SPrintf`). Some functions which are often internally used by the compiler, like `memcpy` or `memset`, aren't implemented, and using code which makes the compiler make use of them should be avoided (`SomeStruct var = {};` would imply a `memset` call, for instance!).

The library makes use of C++20 (gnu++20 in clang), recently released, since it makes use of concepts due to the lack of certain useful but quite annoying template stuff in C++ (std::is_base_of).

## State

The library is a work-in-progress project. Check what's been acomplished and what still needs to be acomplished below:

## TODO

- Implement a filesystem device (in replacement of stuff like `fopen`, `fwrite`...)

- Fully implement CRT0 initialization: homebrew API processing, argv

- Add several services (some interesting/relevant ones: applet, ns, account...)

- Implement sync-related types: condition variable

- Implement several kernel types: thread (almost done), event, shared memory, transfer memory, waitable

## Done

- Working memory allocation system - not the best for sure, but does its job properly

- Basic CRT0 initialization: heap, .bss cleanup, relocation, TLS setup

- Module name support (shown in crash reports)

- Basic string utils found in standard libraries (`util::SPrintf`, `util::Strlen`, `util::Strcmp`...)

- Supported services: sm (partially), set:sys (partially), ldr:ro (partially), lm, fsp-srv (partially), fatal

- Dynamic module support, being able to load modules from NROs (with ldr:ro) or raw memory.

- Custom implementation of shared pointers (`mem::SharedObject`) - global objects for services and modules make use of them, and they are properly disposed when the program finalizes.

- Logging interface via lm service (diag), with proper and working packet generating and encoding.

- Added sync-related types: mutex

- Implemented threading (almost completely)

- Implemented an assertion system (lm, fatal, svcBreak, process exit)

## Future plans (maybe)

- Support more hw-accelerated crypto (not something necessary for now)

- 32-bit support?