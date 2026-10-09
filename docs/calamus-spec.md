# Calamus v1

Calamus is Amanuensis' second format: a line-based text format for Umbra entity files (`.calamus`). Each field sits on its own line, and a marker column shows which fields override the entity's base (its prefab, or its type's default). It maps 1:1 onto `amanuensis::core::Value`. The JSON reader and writer stay strict RFC 8259; Calamus is a separate dialect that shares their value model, string rules and number formatting.

The design note behind it is `amanuensis-json-library/calamus-format.md`. This spec is the normative version.

> **Status: draft for review.** Rulings tagged **(proposed)** were made while writing this spec and haven't been signed off. Everything else was decided in the design note or on the board. [Rulings to review](#rulings-to-review) lists the proposed ones in one place.

## Example

```
@version 3
@type Door
@id 3f9c2a7be1d04c58
@base "prefabs/wooden_door"
@base_hash 9f2c41e07b3d5a18

* position:
*   x : 46.103531 | 0.0
*   y : 108.07968 | 0.0
    z : 0.0
  scale:
    x : 1.0
    y : 1.0
    z : 1.0
* orientation:
*   x : 0.0 | 0.0
*   y : 0.0 | 0.0
*   z : -0.069125 | 0.0
*   w : 0.997608 | 1.0
  name : ""
  flags : 160
* locked : true | false
  tags : ["puzzle", "act1"]
  waypoints:
    -
      x : 0.0
      z : 2.5
    -
      x : 4.0
      z : 2.5
* light:
*   intensity : 2.5 | 1.0
    colour:
      r : 1.0
      g : 0.9
      b : 0.8
```

`orientation` is atomic, so one changed component marks all four (see [Atomic objects](#atomic-objects)).

## Text

- **Encoding.** UTF-8. A leading byte-order mark is skipped on read and never written **(proposed)**.
- **Line endings.** The reader accepts LF and CRLF. A carriage return anywhere else is an error. The writer writes LF.
- **Trailing whitespace.** Spaces and tabs at the end of a line are ignored.
- **Blank lines.** Lines that are empty, or hold only spaces and tabs, are ignored anywhere **(proposed)**. The writer writes exactly one, between the header and the body.
- **End of file.** The writer ends the file with exactly one LF. The reader doesn't require a final newline.
- **Columns** in error positions count bytes, the same as the JSON reader.

## Header

The file starts with header lines. Each is `@name`, one or more spaces, then a value. The writer uses one space and this order:

| Line | Value | Required | Meaning |
| --- | --- | --- | --- |
| `@version` | Decimal integer, at least 1 | Yes | The entity type's data version, for migration. Not the Calamus syntax version. |
| `@type` | Bare name **(proposed)**: a letter or `_`, then letters, digits, `_`, `.` or `:` | Yes | The entity type. |
| `@id` | 1 to 16 hex digits, either case. The writer writes 16 lowercase digits **(proposed)** | No | The entity's ID, held as `std::uint64_t`. A file without one gets one from Umbra's tooling on load. |
| `@base` | Quoted string | No | The prefab the entity is based on. Umbra resolves it. |
| `@base_hash` | Exactly 16 hex digits, either case. The writer writes lowercase | No | `core::StableHash` of the base the writer diffed against, in canonical form (see [Numbers](#numbers)). |

- The filename is free. `@id` lives only in the header, so files can be named `door_01.calamus` and moved or renamed.
- Every header line comes before the first body line. An unknown, repeated or late header line is an error, and so is a missing `@version` or `@type`.
- The writer always writes `@base_hash`, because it always has a base: the prefab, or the type's default when there's no `@base` **(proposed)**. The reader treats it as optional, because a hand-written file may not have one.
- v1 files don't name their syntax version. A later syntax change adds a header line such as `@calamus 2`. A v1 reader rejects unknown header lines, so it fails loudly on such a file **(proposed)**.

## Body lines

After the header, every non-blank line is a body line:

```
<marker><space><indent><content>
```

- **Marker column.** Column 1 is `*` or a space, and column 2 is a space. Toggling the marker is a one-character change that never shifts alignment.
- **Indent.** After those two columns, two spaces per level. Tabs are an error, and so is an odd number of spaces or a line more than one level deeper than its parent.
- **Content** is one of:

| Content | Line kind | Means |
| --- | --- | --- |
| `key : value` | Field line | Sets a field to a scalar or inline array. |
| `key :` or `key:` | Block line | Opens a container under the field. See [Blocks](#blocks). |
| `- value` | Item line | An array item that is a scalar or inline array **(proposed)**. |
| `-` | Item block line | An array item that is a container. |

- **Separators.** The reader allows any number of spaces, including none, around `:` and `|` **(proposed)**. The writer writes `key : value`, `key:` for a block, and ` | ` before a recorded default. It never pads keys to line up the colons, because adding one long key would then re-pad every line and bury the real change in the diff.
- **Keys** are bare when they're identifiers (`[A-Za-z_][A-Za-z0-9_]*`) and quoted otherwise, with the same rules as strings: `"door frame" : 1`. The writer writes a key bare whenever it can.
- **Duplicate keys** in one object are an error, reported at the second one.
- **Comments are not part of v1.** A `#` outside a quoted string is an error. That keeps hand-written notes from being dropped silently, and a later version can add comments without breaking existing files.

## Blocks

A block line (`key:` or a bare `-`) opens a container whose kind is set by the lines indented directly under it:

- Field lines make an **Object**.
- Item lines (`-` or `- value`) make an **Array** **(proposed)**.
- No lines make an **empty Object**. An empty array is always written inline as `[]`.

Mixing field lines and item lines under one block is an error. Item lines anywhere else are an error, including directly under the root, which is always an Object.

### Arrays

- An array that holds no Object at any depth is written inline, as `[a, b, c]`. Inline arrays can nest, such as `[[1, 2], [3]]`, and hold only scalars and inline arrays **(proposed)**. They stay on one line, since arrays override as a whole anyway. A trailing comma is an error.
- Any other array is an array block. Each item is an item line: `-` with the item's fields or items indented under it, or `- value` for an item that is a scalar or inline array.
- An array is one field. It compares, overrides and is marked as a whole.

## Values

- **Scalars:** `null`, `true`, `false`, numbers and quoted strings.
- **Strings** are always quoted, so an empty string is `""` and needs no sentinel. They use JSON's rules: the escapes `\" \\ \/ \b \f \n \r \t \uXXXX` (with surrogate pairs), and no raw control characters. The tokenizer is quote-aware, so `,`, `:`, `|` and `#` inside a string are just characters.
- **Numbers** follow [Numbers](#numbers).

## Numbers

- **Reading** uses the JSON reader's grammar and rule: a `.` or an exponent makes a Double, anything else an Integer. An integer that overflows `long long` reads as a Double, the same as JSON.
- **Integers** are written in plain decimal.
- **Doubles** are written in their *canonical text* **(proposed)**:
  - If the double converts to `float` and back unchanged (and is within `float`'s range), write the shortest text that reads back as that `float`. Otherwise write the shortest text that reads back as the double.
  - Add `.0` when the text has neither a `.` nor an exponent, so it reads back as a Double. So `1.0` stays `1.0`, and a `float` 0.1 is written `0.1` rather than `0.10000000149011612` (hole 4).
- **NaN and infinity** have no syntax in v1. The writer fails with an error naming the field's path **(proposed)**.
- **Canonical values** **(proposed)**. The canonical text of a `float`-valued double reads back as a *different* double: `0.1` reads as 0.1, not 0.10000000149011612. If the reader and writer compared values bit for bit, every `float` field would differ from its base after one round trip. Every `float` would be marked on save, or promoted as a hand edit on load. So Calamus compares and hashes *canonical* values, in which every double is replaced by the double its canonical text reads back as. The writer canonicalises the entity and the base before working out markers and `@base_hash`. The reader canonicalises the base and the file's values before comparing them and checking the hash. `core::Diff` and `core::StableHash` stay bit-exact; canonicalising first is what makes them agree with the text.
- **Integer for a float field.** A hand-typed `1` in a `float` or `double` field must still load. The serialisation layer's float and double traits accept an Integer (hole 3). This is serialisation work, tracked separately.

## Markers and overrides

Unmarked fields follow the base. The marked fields are exactly the entity's overrides (option A).

### Which marks the reader honours

The reader honours the mark on a line that holds a whole field value:

- a field line (scalar or inline array);
- a block line whose block turns out to be an array or an empty object.

Marks anywhere else are informational, and the writer regenerates them: on a block line that opens a non-empty object, and on any line inside an array block. A block line is written marked when any line under it is marked.

### Load

The reader is given the file and the canonical base. It reads every honoured line as a *(key path, value, marked)* entry, then decides each one:

| Line | Compared with the base | `@base_hash` | Result | Reported as | Sets `needsRewrite` |
| --- | --- | --- | --- | --- | --- |
| Marked | Differs, or the base has no field at that path | Any | Override | `additions` if the base has no field there | No |
| Marked | Equal | Matches or absent | Follows the base | `reset` | Yes |
| Marked | Equal | Differs | Follows the base | | No |
| Unmarked | Equal | Any | Follows the base | | No |
| Unmarked | Differs, or the base has no field there | Matches or absent | Override, since a hand edit forgot its `*` | `promoted` | Yes |
| Unmarked | Differs | Differs | Follows the base, since the base changed under it | `refreshed` | No |
| Unmarked | The base has no field there | Differs | Dropped, since the base removed it | `stale` | No |

The loaded value is `core::Overlay(base, overrides)`. Keys the file doesn't mention come from the base, so an entity can't remove a field its base has.

- **`@base_hash` matches** when it equals `StableHash` of the canonical base. A missing hash counts as a match, because a file without one was written by hand **(proposed)**.
- **Marked fields the base doesn't have** are kept as additions, not rejected, and listed in `additions` so tooling can flag an override of nothing **(proposed)**. This replaces the review's "error with line and column" (hole 8): it lets objects with open-ended keys, such as a `std::map` field, gain entries. It also doesn't stop a level loading because a prefab dropped a field.
- **`needsRewrite`** is set only by things a person did to this file (`promoted`, `reset`). Changes to the base (`refreshed`, `stale`, a stale `@base_hash`, override review) wait for the next save, so one prefab edit doesn't rewrite every entity that uses it **(proposed)**. A missing `@id` doesn't set it either: Umbra assigns the ID and writes the file back itself.
- **`needsRewrite` means "regenerating may change the text"**, not "the file is wrong". The caller regenerates the text and writes it only if it differs from the file on disk. A clean file therefore never produces a diff. One expected case is an atomic object: the writer marks its components that equal the base, so a reload reports them as `reset`, and regenerating gives identical text, so nothing is written.
- **Override review.** A marked line can carry the base value it overrides, after `|` (see [Recorded defaults](#recorded-defaults)). When that recorded default differs from the current canonical base, the base has moved under the override. The field is listed in `moved` so a designer can check the override still makes sense.
- **The reader never writes.** Writing a healed file back is the caller's decision, and shipped builds never do it.

### Write

The writer is given the entity and its base, and canonicalises both. It then:

- writes the header, with `@base_hash` set to `StableHash` of the canonical base;
- writes every field of the entity, in the entity's key order. After a load that is the base's order, with additions at the end of their object;
- marks every line at a path `core::Diff` reports, and every line under that path when its value is a container;
- skips keys only the base has, because Calamus can't express removing a field.

#### Atomic objects

Some objects only make sense as a whole, such as a quaternion or a colour. Whether an object is atomic comes from the entity's C++ types, not from the file, through a hook on the traits (a separate task). When any line inside an atomic object is marked, the writer marks every line in it. Otherwise a prefab change could combine new and old components into an invalid rotation.

### Recorded defaults

```
*   x : 46.103531 | 0.0
```

- The writer adds ` | <base value>` to a marked field line when the base has a scalar or inline array at that path. It doesn't add one to array blocks, to lines inside arrays, or to additions.
- **Informational only.** The reader never loads the value after `|`. It must still be a valid scalar or inline array, and it's used only for the override review. Deleting from `|` to the end of the line removes it.
- An unmarked line with a recorded default, left behind by deleting the `*`, is allowed. Its value is promoted or ignored like any other unmarked line, and the next save drops the default.
- **Resetting a field.** Copy the recorded default over the value, or delete the line. Either way the field equals the base, so it follows the base, and the next save drops the `*`.

## Errors

Every error stops the load and reports a message with a line and column, through `core::ParseResult` like the JSON reader. Beyond the shared string and number errors, Calamus reports:

| Error | Example |
| --- | --- |
| Expected `*` or a space in the marker column | `position:` written at column 1 |
| Expected a space after the marker column | `*position:` |
| Tabs aren't allowed in indentation | |
| Indentation must be a multiple of two spaces | |
| Indented more than one level under the line above | |
| Duplicate key | The second `x` in one object |
| A block can't mix fields and `-` items | |
| `-` items can only appear inside an array block | |
| Comments aren't supported in Calamus v1 | `x : 1  # note` |
| A carriage return not followed by a line feed | |
| Unknown, repeated or late header line, or a missing `@version` or `@type` | |
| Invalid header value | `@id xyz` |

## Not in v1

- Comments (see [Body lines](#body-lines)).
- Type annotations such as `Vec3<f32>`, set aside in the design note.
- NaN and infinity.
- Removing a field the base has.

These stay in Umbra: finding and loading prefabs and resolving prefab chains (hashing the resolved values), assigning IDs, when a healed file is written back, and which C++ types are atomic. The `@version` migration hook is designed in its own task.

## Holes from the review

| Hole | Ruling | Where |
| --- | --- | --- |
| 1. Silent hand edits | Base hash and self-healing: an unmarked difference is promoted when `@base_hash` matches | [Load](#load) |
| 2. One-item list looks like a scalar | Arrays always use brackets | [Arrays](#arrays) |
| 3. Integer/Double drift | Doubles always get `.0` or an exponent; float traits accept an Integer | [Numbers](#numbers) |
| 4. Float noise | `float`-valued doubles use their shortest `float` text; values are compared and hashed canonically **(proposed)** | [Numbers](#numbers) |
| 5. Arrays of objects | `-` items, and arrays override as a whole | [Arrays](#arrays) |
| 6. Keys that aren't identifiers | Quoted keys, written bare only when they're identifiers | [Body lines](#body-lines) |
| 7. Separators inside strings | A quote-aware tokenizer with JSON's escapes | [Values](#values) |
| 8. Stale unmarked fields | Dropped on load and removed on the next save; marked fields the base lacks are kept as additions **(proposed)** | [Load](#load) |
| 9. Empty objects | A block line with nothing under it | [Blocks](#blocks) |
| 10. Comments don't survive a save | Cut from v1; `#` is an error | [Body lines](#body-lines) |
| 11. Small things | Duplicate keys are errors; CRLF read, LF written; trailing whitespace ignored; one final newline | [Text](#text), [Errors](#errors) |

## Rulings to review

These were made while writing the spec:

1. **Canonical values** (hole 4). Compare and hash canonicalised values, or `float` fields mismatch their base after every round trip.
2. **Marked fields the base lacks are additions**, not errors (hole 8). They support map-like objects and don't block loading.
3. **`needsRewrite` only for a person's edits** (`promoted`, `reset`), never for base changes.
4. **A missing `@base_hash` counts as a match**, so a hand-written file's values are kept.
5. **`@base_hash` is always written**, hashing the type default when there's no `@base`.
6. **NaN and infinity make the writer fail**, naming the field.
7. **`- value` items and `-` blocks with `-` children**, so any `Value` round-trips, including arrays that mix objects and scalars.
8. **Nested inline arrays** such as `[[1, 2], [3]]`.
9. **Lenient spacing** around `:` and `|`, and blank lines ignored anywhere.
10. **Header values:** `@type` as a bare name, `@id` as 1 to 16 hex digits, and `@base_hash` as exactly 16.
11. **A future `@calamus N` header** for syntax versions, which v1 readers reject.
12. **A leading BOM** is skipped on read.
