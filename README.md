# Current [![CI](https://travis-ci.org/C5T/Current.svg?branch=master)](https://travis-ci.org/C5T/Current)

#### [TypeSystem](https://github.com/C5T/Current/blob/master/typesystem/README.md)
The type system used in Current framework.

#### [RipCurrent](https://github.com/C5T/Current/blob/master/RipCurrent/README.md)
The language to define continuous data pipelines.

#### [Sherlock](https://github.com/C5T/Current/blob/master/Sherlock/README.md)
Structured, append-only, immutable data persistence layer with publish-subscribe.

#### [FnCAS](https://github.com/C5T/Current/blob/master/fncas/README.md)
An efficient convex optimization engine.

#### [CompactTSV](https://github.com/C5T/Current/blob/master/compacttsv/)
Low-level compact persistence layer with 1+ GB/s throughput.

#### [EventCollector](https://github.com/C5T/Current/blob/master/event_collector/README.md)
An extensible event collecting HTTP server.

#### [Blocks](https://github.com/C5T/Current/blob/master/blocks/README.md)
HTTP server and client, efficient in-memory message queue, persistence layer, streaming API interface.

#### [Bricks](https://github.com/C5T/Current/blob/master/bricks/README.md)
JSON and binary serialization, string manipulation library, command line flags library, and other core pieces.

#### [Storage](https://github.com/C5T/Current/blob/master/Storage/REST-API.md)
Storage layer with super easy to use in-memory data views and Sherlock-based persistence.

#### [Type Evolution](https://github.com/C5T/Current/blob/master/typesystem/Evolution.md)
Compact and autogenerated C++ framework to evolve objects from one type hierarchy into another type hierarchy while remaining fully within the strong typing paradigm.

## Contribution

Welcome, contributors! Please [start here by signing the CLA](https://github.com/C5T/Current/blob/master/contributors/README.md).

## Quick Start

### Install the development dependencies

- `nasm` for `FnCAS`.
  - **macOS**: `brew install nasm`
- `geninfo` from `lcov` for coverage report.
  - **macOS**: `brew install lcov`
- `clang-format-3.6` for code formatting (`make indent`).
  - **macOS**: Only `clang-format-3.8` is available via Homebrew: `brew install clang-format@3.8 && ln -s /usr/local/bin/clang-format-3.6 /usr/local/opt/clang-format@3.8/bin/clang-format` _(pretend we've got 3.6)_

### Clean the output of the previous builds

```
make clean
```

### Run the tests

Builds and runs all the tests as a single binary. Slow, eats up tons of CPU, but measures coverage.
Consider `make individual_tests` or `make test` within individual directories to run the subset of tests.
```
make test
```

Builds and runs the tests for each module separately:
```
make individual_tests
```

Builds and runs the tests for one of the modules (e.g. `Blocks/HTTP`):
```
( cd Blocks/HTTP && make test )
```

### Verify the code

"Builds" all header files individually, twice each header file, and "links" these pairs together.
Ensures no symbols are exported, and the ODR will not be violated when linking together two objects, each of which is independently using Current.
```
make check
```
