# PLYodine

[![Test Status](https://github.com/BradleyMarie/plyodine/actions/workflows/c-cpp.yml/badge.svg?branch=main)](https://github.com/BradleyMarie/plyodine/actions/workflows/c-cpp.yml)
[![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)](https://github.com/BradleyMarie/plyodine/blob/main/LICENSE)

A zero-dependency library with high test coverage for C++23 for reading from and
writing to PLY mesh streams. An overview and definition of the format is
available on the Library of Congress
[website](https://www.loc.gov/preservation/digital/formats/fdd/fdd000501.shtml).
This format is often used in the computer vision and graphics communities for
its flexibility and simplicity. Famously, PLY is used to distribute the 3D
models in the
[Stanford 3D Scanning Repository](http://graphics.stanford.edu/data/3Dscanrep/).

PLYodine supports reading from and writing to PLY streams in ASCII, binary
big-endian, and binary little-endian modes. PLYodine uses stream based readers
and writers meaning that they can both support streams larger than can fit into
memory.

While PLYodine does not aim to deliver the absolute highest performance, it does
attempt to be reasonably performant by completing all of its working a single
pass and by exposing a streaming API that allows zero-copy implementations if
desired. Additionally, the PLY reader and writers can be reused across multiple
invocations avoiding the need to reallocate internal data structures between
models.

## Getting Started

PLYodine uses Bazel as its build system and is published into the
[Bazel Central Registry](https://registry.bazel.build/modules/plyodine). If you
are using Bazel as well, you can import PLYodine into your workspace by adding a
snippet like the following into your `MODULE.bazel` file.

```
bazel_dep(name = "plyodine", version = "1.0.4")
```

Note: You should update `version` to reference the to the 
[latest version](https://registry.bazel.build/modules/plyodine) that has been
published.

PLYodine code is structured with the core modules residing in the `plyodine`
directory. `ply_reader` contains the parent `PlyReader` class, `ply_writer`
contains the parent `PlyWriter` class, and `ply_header_reader` contains a
library for reading PLY headers (and is also an internal dependency of
`ply_reader`). There is no dependency between `ply_reader` and `ply_writer`.

The `PlyReader` and `PlyWriter` class are designed for extension and expose a
small public API as well as a small protected API that derived classes must
implement. Fundamentally, both classes work the same way. When their public API
is invoked they call into the derived class's `Start` method which fills out
a list of read/write callbacks for each element-property pair. The type of the
property is determined implicitly based on the type of the callback provided.

While the APIs are broadly similar between `PlyReader` and `PlyWriter`, there is
a bit of additional subtlety to keep in mind when implementing a `PlyWriter`.

Most significantly implementers have the option of implementing the `DelegateTo`
which allows the implementing class to delegate entirely to another `PlyWriter`
sub-class. If this is done, there will be no call to `Start` or any of the other
`PlyWriter` methods and all of those calls will instead go to the delegate.

Additionally, when not delegating there are other differences to be aware of.
First, in `Start`, the derived class is also responsible for filling out the map
indicating how many instances of each element should exist in the input. If this
is not done, PLYodine assumes a value of zero. Second, implementers may also
implement the `GetPropertyListSizeType`, `GetElementRank`, `GetPropertyRank` 
which gets called by `PlyWriter` after `Start` to determine the index type to
use for each property lists contained in the output and the order in which
elements and properties will be emitted to the output respectively.

Also inside the `plyodine` directory resides the `readers` and `writers`
directories. These directories contain pre-implemented readers and writers for
PLY streams that contain more limited APIs than the base `PlyReader` and
`PlyWriter` classes are thus easier to work with.

Currently, `readers` only contains `triangle_mesh_reader` which can handle PLY
input that contain 3D vertices (with optional surface normals and texture
coordinates) and faces formed by lists of vertex indices arranged into triangle
fans.

Additionally, `writers` also contains just a single implementation,
`in_memory_writer`, which generates a PLY output from values that are fully
present in memory.

## Examples

Currently, there is no example code written for PLYodine; however, the code in
the `readers` and `writers` directories can be used as a reference when working
with the `PlyReader` and `PlyWriter` classes respectively.

Additionally, there is good documentation of the PLYodine interface in its
various header files.

## Forwards Compatibility

The public API of `PlyReader` and `PlyWriter` at this point should be mostly
locked down and at this point it can be reasonably expected that there will not
be major breaking changes in the future (or at least the medium feature). This
stability; however, does not extend to the `triangle_mesh_reader` and
`in_memory_writer` classes which are both considered experimental.

For both the stable and experimental portions of the PLYodine API, it is
expected that any future breaking changes will be minor.

Also note that the details of the error codes returned by PLYodine are
explicitly not stable and are subject to change in the future without warning.

## A note on reading PLY files

PLYodine currently is somewhat restrictive with what it will accept as a valid
PLY input (especially when it comes to parsing ASCII-formatted input). If you
are attempting to read a PLY input that is not quite perfect that is being
rejected by PLYodine, to some extent this is working as intended.

However, with PLY being as loosely standardized as it is, if you encounter a
significant corpus of PLY files that will not open with PLYodine but will open
with other libraries or applications please file an issue (and ideally create a
pull request that resolves it). If you aren't sure if the issue is with PLYodine
or your own code, there is a `ply_validator` tool in the `tools` directory that
can be used to verify if a PLY file will open with PLYodine in isolation.

Also, if possible, prefer working with binary PLY files where there is less
ambiguity about what deviations from the "standard" are allowable during
parsing. There is a `ply_sanitizer` tool in the `tools` director that can be
used to translate PLY files between the ASCII, binary big-endian, and binary
little-endian formats (and also sanitize them in the process to be fully
"standards-compliant").
