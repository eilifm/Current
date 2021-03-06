# `c5t-current-schema-ts`

C5T/Current schema common type definitions and validators for TypeScript.

The library provides compile-time (in TypeScript) and runtime (in [`io-ts`](https://github.com/gcanti/io-ts/)) type definitions for the common primitive and collection types used by the schema definitions generated by the C5T/Current schema export as TypeScript.


## Usage

If C5T/Current TypeScript schemas are used, this library is required to be installed as a peer dependency because it is required from the schema files, along with the [`io-ts`](https://github.com/gcanti/io-ts/) library which provides low-level primitives for TypeScript compile-time and runtime type definitions used in the schema files.

```
npm install --save c5t-current-schema-ts io-ts
```

NOTE: The schema files specify the recommended versions of `c5t-current-schema-ts` and `io-ts` at the top, for example:
```
// peerDependencies: io-ts@0.5.1 c5t-current-schema-ts@0.1.0
```

The generated TypeScript schema files export two items per `CURRENT_STRUCT`:
- a `const` with a runtime type object created via `io-ts` `interface` or `union`,
- a compile-time TypeScript `type` representing that runtime type.


## Examples


### Type definitions

Given the following C5T/Current type definitions:
```cpp
CURRENT_STRUCT(Primitives) {
  CURRENT_FIELD(a, uint8_t);
  CURRENT_FIELD_DESCRIPTION(a, "It's the \"order\" of fields that matters.");
  CURRENT_FIELD(b, uint16_t);
  CURRENT_FIELD_DESCRIPTION(b, "Field descriptions can be set in any order.");
  CURRENT_FIELD(c, uint32_t);
  CURRENT_FIELD(d, uint64_t);
  CURRENT_FIELD(e, int8_t);
  CURRENT_FIELD(f, int16_t);
  CURRENT_FIELD(g, int32_t);
  CURRENT_FIELD(h, int64_t);
  CURRENT_FIELD(i, char);
  CURRENT_FIELD(j, std::string);
  CURRENT_FIELD(k, float);
  CURRENT_FIELD(l, double);
  CURRENT_FIELD(m, bool);
  CURRENT_FIELD_DESCRIPTION(m, "Multiline\ndescriptions\ncan be used.");
  CURRENT_FIELD(n, std::chrono::microseconds);
  CURRENT_FIELD(o, std::chrono::milliseconds);
};

CURRENT_VARIANT(MyFreakingVariant, A, X, Y);
```

The following TypeScript type definitions are generated:
```ts
import * as iots from 'io-ts';
import * as C5TCurrent from 'c5t-current-schema-ts';

export const Primitives_IO = iots.interface({
  // It's the "order" of fields that matters.
  a: C5TCurrent.UInt8_IO,

  // Field descriptions can be set in any order.
  b: C5TCurrent.UInt16_IO,
  c: C5TCurrent.UInt32_IO,
  d: C5TCurrent.UInt64_IO,
  e: C5TCurrent.Int8_IO,
  f: C5TCurrent.Int16_IO,
  g: C5TCurrent.Int32_IO,
  h: C5TCurrent.Int64_IO,
  i: C5TCurrent.Char_IO,
  j: C5TCurrent.String_IO,
  k: C5TCurrent.Float_IO,
  l: C5TCurrent.Double_IO,

  // Multiline
  // descriptions
  // can be used.
  m: C5TCurrent.Bool_IO,
  n: C5TCurrent.Microseconds_IO,
  o: C5TCurrent.Milliseconds_IO,
}, 'Primitives');
export type Primitives = iots.TypeOf<typeof Primitives_IO>;

export const MyFreakingVariant_IO = iots.union([
  A_IO,
  X_IO,
  Y_IO,
  iots.null,
], 'MyFreakingVariant');
export type MyFreakingVariant = iots.UnionType<[
  typeof A_IO,
  typeof X_IO,
  typeof Y_IO,
  typeof iots.null
], (
  iots.TypeOf<typeof A_IO> |
  iots.TypeOf<typeof X_IO> |
  iots.TypeOf<typeof Y_IO> |
  iots.TypeOf<typeof iots.null>
)>;
```


### Type validation

The `validate` function from `io-ts` returns a value of the `Either` type which can be either `Right` or `Left`.
This `Either` type and the `isRight` function to tell `Right` from `Left` are defined in the `fp-ts` module at `fp-ts/lib/Either`, so `fp-ts` is required for the type validation.

```
npm install --save fp-ts
```

```ts
import * as fs from 'fs';
import * as iots from 'io-ts';
import { isRight } from 'fp-ts/lib/Either';
import { PathReporter } from 'io-ts/lib/PathReporter';

import * as generated_schema_ts from './generated_schema_ts';

const generated_schema_serialized: generated_schema_ts.FullTest = JSON.parse(String(fs.readFileSync('./generated_schema_serialized.json')));
const validationResult = iots.validate(generated_schema_serialized, generated_schema_ts.Primitives_IO);

isRight(validationResult);  // => `true` if validated successfully; `false` otherwise.

const error_report = PathReporter.report(validationResult);  // => string[]
```


## Contribution

Please see [C5T/Current](https://github.com/C5T/Current) guidelines for contribution.
