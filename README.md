# libbio

libbio is Biosphere's userland homebrew library.

The library has literally zero dependencies - not even standard C/C++ libraries or headers. Therefore, it provides its own implementations of common C/C++ features (`mem:::SharedObject`, `util::SPrintf`). Some functions which are often internally used by the compiler, like `memcpy` or `memset`, aren't implemented, and using code which makes the compiler make use of them should be avoided (`SomeStruct var = {};` would imply a `memset` call, for instance!).

The library makes use of C++20 (gnu++20 in clang), recently released, since it makes use of concepts due to the lack of certain useful but quite annoying template stuff in C++ (std::is_base_of).

## State

The library is a work-in-progress project. Check what's been acomplished and what still needs to be acomplished below:

## TODO

- Implement a proper memory allocation system - current one is fairly simple and cannot reallocate/free...

- Implement a filesystem device (in replacement of stuff like `fopen`, `fwrite`...)

- Fully implement CRT0 initialization: homebrew API processing, argv

- Add several services (some interesting/relevant ones: applet, ns, account...)

- Implement sync-related types: mutex, condition variable

- Implement several kernel types: thread (fully), event, shared memory, transfer memory, waitable

## Done

- Basic CRT0 initialization: heap, .bss cleanup and relocation...

- Module name support (shown in crash reports)

- Basic string utils found in standard libraries (`util::SPrintf`, `util::Strlen`, `util::Strcmp`...)

- Supported services: sm (partially), set:sys (partially), ldr:ro (partially), lm, fsp-srv (partially)

- Dynamic module support, being able to load modules from NROs (with ldr:ro) or raw memory.

- Custom implementation of shared pointers (`mem::SharedObject`)

- Logging interface via lm service (diag), with proper and working packet generating and encoding.

> TODO: add more here

## Future plans (maybe)

- Support more hw-accelerated crypto (not something necessary for now)

- 32-bit support?