# libbio

libbio is Biosphere's userland homebrew library (64-bit only, at least for now).

The library has literally zero dependencies - not even standard C/C++ libraries or headers. Therefore, it provides its own implementations of common C/C++ features (`mem:::SharedObject`, `util::SPrintf`).

The library makes use of C++20 (gnu++20 in clang), recently released, since it makes use of concepts due to the lack of certain useful but quite annoying template stuff in C++ (std::is_base_of).

> TODO: improve README and docs...

## State

The library is a work-in-progress project. Check what's been acomplished and what still needs to be acomplished below:

## Custom results

Result module: 420

Submodules:

- Common: submodule 0 (2420-00**)

- Memory: submodule 1 (2420-01**)

- Dynamic: submodule 2 (2420-02**)

- IPC: submodule 3 (2420-03**)

- Diag: submodule 4 (2420-04**)

- Filesystem: submodule 5 (2420-05**)

- Util: submodule 6 (2420-06**)

- NV driver error codes: submodule 7 (2420-07**)

- GPU (and binder error codes): submodule 8 (2420-08**)

- Input: submodule 9 (2420-09**)

## TODO

- Examples:

  - Migrate current development test projects as proper examples

  - More new examples

- Fully Implement serverside IPC: buffers, sessions/objects, domains, command arguments

- Fully implement CRT0 initialization:

  - hbl ABI, read everything hbl sends us

  - Argv parsing and reading

- Extend exception handling: get registers like libnx does, etc.

- Implement sync-related types:

  - Condition variables

- Implement several kernel types:

  - Threads (almost finished)

  - Events

  - Shared memory (SVCs implemented)

  - Transfer memory (SVCs implemented)

  - Waitable system (almost finished)

- Extend GPU implementation:

  - More layer types (normal layers with ARUIDs, managed layers, ...)

- Implement applet service (has a lot of useful and essential commands, library applet launching, ...)

- RomFs support? (follow homebrew standards and read appended NACP/icon/RomFs data in the NRO file path, via argv[0])

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

- Basic exception handling support

- Filesystem API: device mounting, file creating/deleting/opening/reading/writing, directory creating/deleting/iterating...

- Proper service guarding to ensure services are properly accessed and disposed

- System version getting support, supporting both hbl HosVersion and set:sys service.

- Implement graphics/GPU, along with nv, vi and nvnflinger/dispdrv services and NVIDIA driver stuff (binder, parcels, maps, events, types...)

- Added memory data cache utilities (grabbed from libnx)

- Implemented a dynamic list type - doubly linked lists, supporting iteration and quite easy to use.

- Implemented a basic waitable system (half of it) to simplify handle waiting.

- Added arm system tick utilities.

- Implemented virtual memory reserving.

- Implemented hid service and an input system.