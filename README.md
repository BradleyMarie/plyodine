# PLYodine

[![Test Status](https://github.com/BradleyMarie/plyodine/actions/workflows/c-cpp.yml/badge.svg?branch=main)](https://github.com/BradleyMarie/plyodine/actions/workflows/c-cpp.yml)
![License](https://img.shields.io/badge/License-BSD_3--Clause-blue.svg)

A zero-dependency library with high test coverage for C++23 for reading from and
writing to PLY mesh files. An overview and definition of the file format is
available on the Library of Congress
[website](https://www.loc.gov/preservation/digital/formats/fdd/fdd000501.shtml).
This format is often used in the computer vision and graphics communities for
its flexibility and simplicity. Famously, PLY is used to distribute the 3D
models in the
[Stanford 3D Scanning Repository](http://graphics.stanford.edu/data/3Dscanrep/).

PLYodine supports reading from and writing to PLY files in binary big-endian,
binary little-endian, and ASCII modes. PLYodine uses stream based readers and
writers meaning that they can both support files larger than can fit into
memory.

While PLYodine does not aim to deliver the absolute highest performance, it does
attempt to be "high performance" by completing all of its working a single pass
and by exposing a streaming API that allows zero-copy implementations if
desired. Additionally, the PLY reader and writers can be reused across multiple
invocations avoiding the need to reallocate internal data structures between
models.

## Getting Started

PLYodine uses Bazel as its build system. If you are using Bazel as well, you can
import PLYodine into your workspace by adding a snippet like the following into
your `WORKSPACE` file.

```
http_archive(
    name = "plyodine",
    sha256 = "e796ce7ba80e9a28d661d75009156eda989d6825cad75c5842f69a632df1bcd4",
    strip_prefix = "plyodine-af404e94b36ba8fd9dc669a1a26d3a44e24ed68e",
    url = "https://github.com/BradleyMarie/plyodine/archive/af404e94b36ba8fd9dc669a1a26d3a44e24ed68e.zip",
)
```

Note: You should update `url` and `strip_prefix` to point to the latest commit
on the main branch and should also update `sha256` with the checksum from that
snapshot.

PLYodine code is structured with the core modules residing in the `plyodine`
directory. `ply_reader` contains the parent `PlyReader` class, `ply_writer`
contains the parent `PlyWriter` class, and `ply_header_reader` contains a
library for reading PLY file headers (and is also an internal dependency of
`ply_reader`). There is no dependency between `ply_reader` and `ply_writer`.

The `PlyReader` and `PlyWriter` class are designed for extension and expose a
small public API as well as a small protected API that derived classes must
implement. Fundamentally, both classes work the same way. When their public API
is invoked they call into the derived class's `Start` method which fills out
a list of read/write callbacks for each element-property pair. The type of the
property is determined implicitly based on the type of the callback provided.

While the APIs are broadly similar between `PlyReader` and `PlyWriter`, there is
a bit of additional subtlety to keep in mind when implementing a `PlyWriter`.
First, in `Start`, the derived class is also responsible for filling out the map
indicating how many instances of each element should exist in the file. If this
is not done, PLYodine assumes a value of zero. Second, implementers must also
implement the `GetPropertyListSizeType` which gets called by `PlyWriter` after
`Start` to determine the index type to use for each property lists contained
in the output. Note that while the callbacks for writing property list data
include a vector in their arguments, there is no requirement to populate this
vector with any data. PLYodine only looks at the returned span and the vector
is passed strictly for convenience as a possible backing store for the returned
span.

Also inside the `plyodine` directory resides the `readers` and `writers`
directories. These directories contain pre-implemented readers and writers for
PLY files that contain more limited APIs than the base `PlyReader` and
`PlyWriter` classes are thus easier to work with.

Currently, `readers` only contains `triangle_mesh_reader` which can handle PLY
files that contain 3D vertices (with optional surface normals and texture
coordinates) and faces formed by lists of vertex indices arranged into triangle
fans.

Additionally, `writers` also contains just a single implementation,
`in_memory_writer`, which generates a PLY file from values that are fully
present in memory.

## Examples

Currently, there is no example code written for PLYodine; however, since
PLYodine has good unit test coverage you can use the test code as a reference
for how to work with the library.

Additionally, the code in the `readers` and `writers` directories can be used
as a reference when working with the `PlyReader` and `PlyWriter` classes
respectively.

## Versioning

PLYodine currently is not strongly versioned and it is recommended that users
update to the latest commit from the main branch as often as is reasonably
possible.

The public API of `PlyReader` and `PlyWriter` at this point should be mostly
locked down; however, it is possible that future commits may introduce breaking
API changes.

## A note on reading PLY files

PLYodine currently is very strict with what it will accept as a valid PLY file
(especially when it comes to parsing ASCII files). If you have a PLY file that
is not quite perfect that is being rejected by PLYodine, to some extent this is
working as intended.

However, with PLY being as loosely standardized as it is, if you encounter some
significant class of files that will not open in PLYodine and will open with
other libraries and applications please document the issue and file an issue or
create a pull request.

Also, if possible, prefer working with binary PLY files where there is less
ambiguity about what deviations from the standard are allowable during parsing.