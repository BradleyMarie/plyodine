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
binary little-endian, and ASCII modes. PLYodine uses a stream-based reader
meaning that it can support PLY files greater than 4GB in size. Writing
currently requires all properties to be in memory.

While PLYodine does not aim to deliver the highest performance possible, it does
attempt to be "high performance" by using a zero-copy based approach to reading
and writing and by doing all of its work in a single pass. The PLY readers can
also be reused across multiple models to avoid needing to reallocate their
internal data structures.

## Getting Started

PLYodine uses Bazel as its build system. If you are using Bazel as well, to get
started you need to import Bazel into your Bazel workspace by adding a snippet
like the following to your WORKSPACE file.

```
http_archive(
    name = "plyodine",
    sha256 = "3c140ada65523d7f2f61ea69189804b4b1ab2b1ff7ab3b4de96a5c83e25b5e09",
    strip_prefix = "plyodine-3df49d7eae095f0a89810e21b22ff0c6d33d42c3",
    url = "https://github.com/BradleyMarie/plyodine/archive/3df49d7eae095f0a89810e21b22ff0c6d33d42c3.zip",
)
```

Note: You should update `url` and `strip_prefix` to point to the latest commit
on the main branch and should also update `sha256` with the checksum from that
snapshot.

PLYodine code is structured with the core modules residing in the `plyodine`
directory. `ply_reader` contains the parent class for PLY readers, `ply_writer`
contains the logic for writing a PLY file, and `ply_header_reader` contains a
library for reading PLY file headers and is depended on by `ply_reader`.

There is no dependency between `ply_reader` and `ply_writer`.

Also inside the `plyodine` directory resides the `readers` directory. This
directory contains classes which contain PLY readers with more logic and simpler
interfaces than the base class found in `ply_reader`. Currently, there is only
one such class, `triangle_mesh_reader` which can handle PLY files that contain
3D vertices (with optional surface normals and texture coordinates) and faces
formed by lists of vertex indicies arranged into triangle fans.

**Most users that just want to open PLY files** should prefer
`triangle_mesh_reader` over the base PLY reader class since it handles a lot of
the complexity of parsing PLY files for you.

The best way to get an idea of how to use each of the modules in PLYodine is to
look at the unit tests for module.

## Versioning

PLYodine currently is not strongly versioned and it is recommended that users
update to the latest commit from the main branch as often as is reasonably
possible.

The public API of plyodine at this point should be mostly locked down; however,
it is possible that future commits may introduce breaking API changes.

## A note on reading PLY files

PLYodine currently is very strict with what it will accept as a valid PLY file
(especially when it comes to parsing ASCII files). If you have a PLY file that
is not quite perfect that is being rejected by PLYodine, to some extent this is
working as intended.

However, with PLY being as loosely standardized as it is if you encounter some
significant class of files that will not open in PLYodine and will open with
other libraries and applications please document the issue and file an issue or
create a pull request.

Also, if possible, prefer working with binary PLY files where there is less
ambiguity about what deviations from the standard are allowable during parsing.