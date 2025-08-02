# fcl
A C89 standard compliant, single header, nostdlib (no C Standard Library) fast compression library (FCL).

For more information please look at the "fcl.h" file or take a look at the "examples" or "tests" folder.

> [!WARNING]
> THIS PROJECT IS A WORK IN PROGRESS! ANYTHING CAN CHANGE AT ANY MOMENT WITHOUT ANY NOTICE! USE THIS PROJECT AT YOUR OWN RISK!

## Quick Start

Download or clone fcl.h and include it in your project.

```C
#include "fcl.h" /* Fast compression library */

int main() {
    
    /* Example input */
    unsigned char input[] = {
    'A', 'A', 'A', 'A', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'C', 'C', 'C',
    'D', 'E', 'F', 'F', 'F', 'F', 'F', 'F', 'F', 'F', 'F', 'F', 'F'};

    /* Define a buffer to hold the compressed data */
    unsigned char compressed[64];
    unsigned long compressed_size = 0;

    /* Define a buffer to hold the decompressed data */
    unsigned char decompressed[64];
    unsigned long decompressed_size = 0;

    /* Compress data */
    fcl_compress(input, sizeof(input), compressed, &compressed_size, sizeof(compressed))
    
    /* Decompress data */
    fcl_decompress(compressed, compressed_size, decompressed, &decompressed_size, sizeof(decompressed));

    return 0;
}
```

## Run Example: nostdlib, freestsanding

In this repo you will find the "examples/fcl_win32_nostdlib.c" with the corresponding "build.bat" file which
creates an executable only linked to "kernel32" and is not using the C standard library and executes the program afterwards.

## "nostdlib" Motivation & Purpose

nostdlib is a lightweight, minimalistic approach to C development that removes dependencies on the standard library. The motivation behind this project is to provide developers with greater control over their code by eliminating unnecessary overhead, reducing binary size, and enabling deployment in resource-constrained environments.

Many modern development environments rely heavily on the standard library, which, while convenient, introduces unnecessary bloat, security risks, and unpredictable dependencies. nostdlib aims to give developers fine-grained control over memory management, execution flow, and system calls by working directly with the underlying platform.

### Benefits

#### Minimal overhead
By removing the standard library, nostdlib significantly reduces runtime overhead, allowing for faster execution and smaller binary sizes.

#### Increased security
Standard libraries often include unnecessary functions that increase the attack surface of an application. nostdlib mitigates security risks by removing unused and potentially vulnerable components.

#### Reduced binary size
Without linking to the standard library, binaries are smaller, making them ideal for embedded systems, bootloaders, and operating systems where storage is limited.

#### Enhanced performance
Direct control over system calls and memory management leads to performance gains by eliminating abstraction layers imposed by standard libraries.

#### Better portability
By relying only on fundamental system interfaces, nostdlib allows for easier porting across different platforms without worrying about standard library availability.
