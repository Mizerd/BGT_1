# Educational 256-bit hash function (AI-assisted implementation)

## Purpose

This directory contains a small, self-contained, **originally designed** hash
function written for a university Blockchain Technologies assignment. It takes
an arbitrary sequence of bytes and produces a fixed 256-bit digest.

This is the **AI-assisted implementation track** of the assignment: the design
and the code in this directory were produced with the help of an AI assistant,
in contrast to the independently written manual track. The two tracks are
developed separately and share no code.

## Warning

> **This is an educational custom hash function. It has not undergone
> cryptographic analysis and must not be used to protect passwords,
> credentials, financial data, signatures, authentication systems, or other
> security-sensitive information.**

No claim is made that this construction is secure, collision resistant or
preimage resistant. Those are properties that a later stage of the assignment
will investigate experimentally; nothing here should be read as a guarantee
about them. Known and suspected weaknesses are listed near the end of this
document.

## Build

C++20 and CMake 3.20 or newer are required. From this directory:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

The build produces the executable `build/hash-generator`. It compiles with
`-Wall -Wextra -Wpedantic` and is warning free.

## Usage

```bash
./build/hash-generator --text "hello"
./build/hash-generator --text ""
./build/hash-generator --file some_file.bin
./build/hash-generator --help
```

Example output:

```text
$ ./build/hash-generator --text "hello"
dc39127a9e7914e3de9854721fbc1f75d7a679f0d42b3bc6351dddcab22446e5
```

Exit status is `0` on success, `1` for a usage error and `2` when a file
cannot be opened or read. A read failure is reported on standard error and
never silently treated as empty input.

### Sanity checks

```bash
./build/sanity-checks        # checks on the hashing API
./tests/run_sanity.sh build  # checks through the command line program
```

## Output

* Always exactly 256 bits = 32 bytes.
* Printed as exactly 64 lowercase hexadecimal characters, leading zeroes
  included.
* Byte order is fixed by the implementation (big-endian), so the digest is
  identical on little-endian and big-endian machines.

## Input behaviour

* Any byte sequence is accepted, including the empty one and arbitrary binary
  data; the core never interprets bytes as text.
* In `--text` mode the bytes of the command line argument are hashed exactly
  as the shell delivered them: nothing is trimmed, re-cased, Unicode
  normalised, or given a trailing newline. Where the environment supplies
  UTF-8, the UTF-8 bytes are what gets hashed.
* In `--file` mode the file is opened in binary mode and its exact contents
  are hashed. Line endings are never rewritten and the file name is never part
  of the input.
* The result depends on every input byte, on the order of the bytes, and on
  the exact input length.
* The function is deterministic: no randomness, no time, no process state, no
  global state. Repeated calls and repeated program runs give identical
  results.

## Design

### Internal state

The state is **four 64-bit lanes**, named A, B, C and D, that together hold
256 bits. There is no separate chaining variable, no expanded working vector
and no message schedule array; the four lanes are the entire state and they
are also what gets written out at the end.

The lanes start from four fixed constants (see *Constant derivation*).

### The mixing step

Everything is built from one step, `MIX(word0, word1, tag)`. It takes two
64-bit message words plus a 64-bit *tag* and drives them once around the lane
ring **A → B → C → D → A**:

```text
A = A + (word0 XOR tag)      the first word enters lane A, tagged by position
C = C XOR word1              the second word enters lane C
A = rotl(A, 55)
B = (B XOR A) * MUL_B        A -> B
C = C + B                    B -> C
C = rotl(C, 10)
D = (D XOR C) * MUL_D        C -> D
A = A + D                    D -> A, the ring is closed
B = B XOR rotl(C, 17)        extra cross link, so B also sees C
D = D + rotl(A, 35)          extra cross link, so D also sees the closed ring
```

Why each part exists:

* **Two entry points.** The two words of a block enter at different lanes (A
  and C) and through different operations (addition and XOR). A block is
  therefore never accumulated into a private per-lane accumulator, which is
  what keeps the four output words from behaving like four small independent
  hashes.
* **The ring.** Because each lane is updated from the lane before it, and the
  last update feeds back into A, a single step already makes every lane depend
  on both new message words and on all four previous lane values.
* **Two multiplications.** Addition, XOR and rotation alone are linear or
  almost linear. The two multiplications by fixed odd constants are the
  non-linear part of the design. Odd multipliers are used because
  multiplication by an odd number modulo 2^64 is invertible, so the step never
  throws information away.
* **Four rotations.** A multiplication only carries influence from low bit
  positions towards high ones. The rotations bring high bits back down so that
  the next addition and multiplication can spread them again.
* **Invertibility.** Every single operation above is invertible for a fixed
  message and tag, so `MIX` is a bijection on the state. Two different states
  can never merge into one; digest collisions can only come from different
  inputs, not from the state collapsing. (The same property has a downside,
  described under *Suspected weaknesses*.)

### Processing stages

1. **Whole blocks.** The input is cut into 16-byte blocks. Each block is two
   64-bit words loaded big-endian and absorbed with one `MIX`. The tag for
   block number `i` (counting from 0) is `(i + 1) * TAG_STEP`, so the same
   block contributes differently depending on where it sits in the message.
2. **Tail.** Exactly one tail step is always performed, even when the input
   length is a multiple of 16 and even when the input is empty.
3. **Length.** The total byte count is absorbed by its own `MIX`.
4. **Drain.** Three further `MIX` steps run with no message input.
5. **Output.** The four lanes are serialised big-endian.

### Tail handling

The leftover `r` bytes (`0 <= r <= 15`) are copied to the front of a 16-byte
buffer that is filled with zeroes, and that buffer is absorbed like a normal
block. **No delimiter byte is appended.** Instead the number of leftover bytes
is folded into the tag:

```text
tail tag = (n + 1) * TAG_STEP + (r + 1) * TAG_TAIL
```

where `n` is the number of whole blocks. This is what keeps `"ab"` apart from
`"ab\0"`: both produce the same zero-padded tail buffer, but `r` differs, so
the tags differ. Since `r` and `n` are both determined by the input length,
the encoding of an input as a sequence of absorbed values is unambiguous.

### Length handling

The exact length in bytes is used twice, in two different ways:

* the tail length `r` is part of the tail tag, as described above;
* the total length `L` is absorbed as a separate step,
  `MIX(L, rotl(L, 32), TAG_END)`. The rotated copy is passed as the second
  word so that the length reaches both of the step's entry points.

### Finalization

After the length step, three more `MIX` steps run with both message words set
to zero and a tag that changes every step
(`TAG_END + step * TAG_STEP`). Their purpose is to give the last bytes of the
input as much mixing as the first bytes received, and the changing tag stops
the three steps from being identical repetitions. Measurements on the finished
implementation showed the state already fully mixed after one such step; three
are used as a small margin.

### Output serialization

Lane A is written first, then B, C and D, each as eight bytes most significant
byte first. Loading and storing are done with explicit shift-and-mask loops,
never by casting a byte pointer to a wider integer type, so the digest does not
depend on the host byte order.

## Constant derivation

All constants were produced by one documented procedure that is unrelated to
any existing hash function. Nothing is taken from MD5, the SHA families,
Keccak, BLAKE, SipHash, MurmurHash, xxHash, CityHash, FarmHash, FNV, SplitMix,
PCG or any cipher.

The procedure:

1. Write the decimal digits of `n^n` for `n = 2, 3, 4, ...` and concatenate
   them into one long decimal string
   (`4`, `27`, `256`, `3125`, ... → `"4272563125..."`).
2. Cut the string into consecutive chunks of 20 decimal digits.
3. Read each chunk as an integer, reduce it modulo 2^64 and set its lowest bit
   to 1 (so that every constant is odd, which makes the multipliers
   invertible).
4. Accept the value as a 64-bit constant only if its population count is
   between 26 and 38, so that no constant has an unbalanced bit pattern.
5. The first nine accepted values become, in order: the four lane initial
   values, the two multipliers, and the three tag constants.
6. The four rotation amounts come from the chunks that follow, mapped with
   `(chunk mod 47) + 9` into the range `[9, 55]` and required to be distinct.

Reproducing the procedure:

```python
digits = "".join(str(n ** n) for n in range(2, 200))
chunks = [digits[i:i+20] for i in range(0, len(digits) - 20, 20)]
words, rots, i = [], [], 0
while len(words) < 9:
    v = (int(chunks[i]) & (2**64 - 1)) | 1
    i += 1
    if 26 <= bin(v).count("1") <= 38:
        words.append(v)
while len(rots) < 4:
    r = (int(chunks[i]) % 47) + 9
    i += 1
    if r not in rots:
        rots.append(r)
```

The resulting values, fixed at compile time and never generated at run time:

| Name | Value |
| --- | --- |
| lane A initial value | `0x50EFEF418AACDDB3` |
| lane B initial value | `0x06837D22C9EC9F1B` |
| lane C initial value | `0x93ED2CF209AF0CA9` |
| lane D initial value | `0x1511760347A610CB` |
| `MUL_B` | `0xE0613E1645D94F07` |
| `MUL_D` | `0x7AA3E75445026145` |
| `TAG_STEP` | `0x0976B9398CFCD1FF` |
| `TAG_TAIL` | `0x2B84EC77CA013781` |
| `TAG_END` | `0x2AD26360F20D2A3D` |
| rotations | 55, 10, 17, 35 |

Every value was checked against a list of constants published by the hash
functions reviewed below (initial values, round constants, multipliers and
finalizer constants) and against their rotation schedules. There were no
matches.

## Pseudocode

This is the finished implementation, written out in words.

```text
constants:
    LANE_INIT = [0x50EFEF418AACDDB3, 0x06837D22C9EC9F1B,
                 0x93ED2CF209AF0CA9, 0x1511760347A610CB]
    MUL_B    = 0xE0613E1645D94F07
    MUL_D    = 0x7AA3E75445026145
    TAG_STEP = 0x0976B9398CFCD1FF
    TAG_TAIL = 0x2B84EC77CA013781
    TAG_END  = 0x2AD26360F20D2A3D
    all arithmetic on 64-bit words is modulo 2^64

procedure MIX(word0, word1, tag):
    A = A + (word0 XOR tag)
    C = C XOR word1
    A = rotate_left(A, 55)
    B = (B XOR A) * MUL_B
    C = C + B
    C = rotate_left(C, 10)
    D = (D XOR C) * MUL_D
    A = A + D
    B = B XOR rotate_left(C, 17)
    D = D + rotate_left(A, 35)

function HASH(input):
    # 1. state initialization
    A, B, C, D = LANE_INIT
    L = number of bytes in input
    n = L / 16            (integer division)
    r = L mod 16

    # 2. input processing, one step per 16-byte block
    for i = 0 to n - 1:
        w0 = big_endian_u64(input[16*i     .. 16*i + 7])
        w1 = big_endian_u64(input[16*i + 8 .. 16*i + 15])
        MIX(w0, w1, (i + 1) * TAG_STEP)

    # 3. tail processing, always performed, even when r = 0
    tail = the last r bytes of input, followed by (16 - r) zero bytes
    MIX(big_endian_u64(tail[0 .. 7]),
        big_endian_u64(tail[8 .. 15]),
        (n + 1) * TAG_STEP + (r + 1) * TAG_TAIL)

    # 4. length incorporation
    MIX(L, rotate_left(L, 32), TAG_END)

    # 5. finalization, no message input
    for s = 1 to 3:
        MIX(0, 0, TAG_END + s * TAG_STEP)

    # 6. output extraction, 32 bytes
    return big_endian_bytes(A) || big_endian_bytes(B) ||
           big_endian_bytes(C) || big_endian_bytes(D)
```

## Important implementation decisions

* **Hashing is separated from input handling.** `custom_hash` takes a
  `std::span<const std::uint8_t>` and returns a `std::array<std::uint8_t, 32>`.
  It has no idea whether the bytes came from a command line argument, a text
  file or a binary file, and it performs no I/O.
* **Explicit byte order.** Message words are assembled with shifts, and the
  digest is written with shifts. There is no `reinterpret_cast` to a wider
  integer type and no dependence on host endianness. The C++ output was
  cross-checked against an independent big-integer reference implementation
  and matches byte for byte.
* **Fixed-width unsigned types only.** All arithmetic uses `std::uint64_t`,
  where overflow wraps and is well defined, so there is no signed overflow and
  no undefined behaviour. The implementation was also run under the address and
  undefined-behaviour sanitizers.
* **C++20 `std::rotl`** is used instead of hand-written shift pairs, which
  avoids the classic undefined shift by 64.
* **One uniform code path.** There are no special cases for short inputs: a
  0-byte input, a 5-byte input and a 5-megabyte input all go through the same
  stages. This keeps the function easy to explain and avoids the family of
  bugs that hides in length-dependent branches.
* **A tail step always runs**, so the code has no "if there is a remainder"
  branch in the middle of the pipeline and the block-aligned case cannot be
  forgotten.
* **File reading fails loudly.** A file that cannot be opened, or that fails
  part way through reading, produces an error message and a non-zero exit
  status instead of an empty-input digest.

## Suspected weaknesses

These are honest observations about the construction, not results of a formal
analysis.

1. **The whole state is the output.** There is no hidden capacity: the 256-bit
   digest *is* the 256-bit internal state. Anyone holding a digest knows the
   final state exactly.
2. **The step is invertible.** `MIX` is a bijection, which is good for
   avoiding state collapse but means the pipeline can also be run backwards.
   Combined with point 1, working backwards from a digest towards a state that
   an attacker can reach is a realistic exercise, so preimage resistance should
   not be assumed.
3. **Only one mixing step per block.** Established cryptographic hashes run
   many rounds per block. Here a 16-byte block gets a single step, which leaves
   very little margin against a differential attack in which the attacker
   chooses two consecutive blocks.
4. **Multiplication carries influence upwards only.** Diffusion from high bit
   positions depends entirely on the four rotations. A difference pattern
   deliberately placed in the top bits of a message word may need more steps to
   spread than an average difference does.
5. **Tags are not individually injective.** Different `(n, r)` pairs can
   produce the same tail tag value modulo 2^64. This does not make the padding
   ambiguous, because the total length is absorbed separately, but the tags
   should not be thought of as a unique encoding by themselves.
6. **No keying or salting.** There is no key, seed or salt input, so nothing
   here resists an attacker who can simply recompute the function.
7. **Good measured avalanche is not evidence of security.** The diffusion
   measurements taken during development say only that obvious structure is
   absent; they say nothing about resistance to a targeted attack.
8. **No third-party review.** The design is a few days old and has been looked
   at by nobody outside this assignment.

## Originality review

The design was created by starting from the required properties, not by taking
an existing algorithm apart. Before the construction was fixed, and again after
the implementation was finished and tested, it was compared against the
following families using their specifications, reference sources and
authoritative descriptions. The point of the comparison is to identify
structures that had to be avoided.

**Families inspected:** MD5, SHA-1, SHA-2 (SHA-256 and SHA-512), SHA-3 /
Keccak, BLAKE2, BLAKE3, SipHash, MurmurHash3, xxHash / XXH3, CityHash,
FarmHash, FNV / FNV-1a. Additionally consulted because they are the nearest
relatives of a multiply-and-rotate lane design: MetroHash, SpookyHash,
HighwayHash, and the Jenkins hashes.

| Family | Its structure | How this construction differs |
| --- | --- | --- |
| **MD5** | Merkle-Damgård, 512-bit blocks, four 32-bit chaining words, 64 rounds driven by a table of 64 additive constants and a per-round shift table, feed-forward addition of the chaining value, little-endian, padding of `0x80` + zeroes + length | No chaining variable and no feed-forward, no round constant table, no shift table, one step per block instead of 64 rounds, 64-bit lanes, 16-byte blocks, big-endian, and no `0x80` delimiter |
| **SHA-1** | Five 32-bit words, 80 rounds, message expansion of 16 words into 80 with a rotate-by-1 recurrence, four round constants, feed-forward | No message expansion at all: each message word is used exactly once, directly. No round constants, no feed-forward, different state shape |
| **SHA-2** | Eight words, 64 or 80 rounds, message schedule with σ functions, a table of 64/80 additive constants, Ch/Maj/Σ functions, feed-forward, initial values from square roots of primes | None of these elements is present. No schedule, no constant table, no Ch/Maj, no feed-forward, and initial values from an unrelated digit procedure |
| **SHA-3 / Keccak** | Sponge over a 1600-bit 5×5×64 state, permutation of θ, ρ, π, χ, ι repeated 24 times, rate/capacity split, `pad10*1` with a domain byte, squeezing phase | Not a sponge: there is no capacity, the state is 256 bits, there is no bit-plane permutation, no χ-style non-linearity, no `pad10*1`, and output is read directly rather than squeezed |
| **BLAKE2** | HAIFA-style compression, 16-word working vector expanded from state plus initial values, G quarter-round applied to columns then diagonals, 12 rounds per 128-byte block, `sigma` message permutation table, counter and finalization flags, feed-forward XOR | No expanded working vector, no G function, no column/diagonal pattern, no `sigma` permutation table, no finalization flag, no feed-forward. The one shared idea is that block position influences mixing; here it is an index multiplied by an odd constant that is XORed into a message word, not a byte counter XORed into two words of a working vector |
| **BLAKE3** | 1024-byte chunks hashed independently, binary Merkle tree over chunk results, chunk/parent flags, extendable output | Strictly sequential, no tree, no chunk independence, no flags, fixed 256-bit output only |
| **SipHash** | Four 64-bit lanes, keyed by two key words, 8-byte blocks XORed into one lane before and after two ARX SipRounds, rotations 13/16/21/32/17, final block carries the length in its top byte, `0xff` XOR into one lane, four finalization rounds, output is the XOR of all lanes | Closest in outward shape (four 64-bit lanes plus finalization rounds), and deliberately different inside: no key, 16-byte blocks, two entry lanes instead of one, multiplication is used where SipHash is strictly add-rotate-xor by design, the update is a one-directional ring rather than SipHash's paired butterfly, the length is a tagged injection step rather than a byte in the last block, finalization uses changing tags rather than a single `0xff` XOR, and the output is the lanes concatenated rather than XORed together |
| **MurmurHash3 (x64 128)** | 16-byte blocks, two 64-bit accumulators treated symmetrically, message word multiplied by `c1`, rotated, multiplied by `c2`, XORed into the accumulator, then `h = rotl(h, r) + h_other`, then `h * 5 + constant`; finalization XORs the length, cross-adds and applies the xor-shift-multiply `fmix64` | Same block size, nothing else in common. The message words here are never multiplied; multiplication is applied to a XOR of two *state* lanes. There is no `* 5 + constant` step, no symmetric duplicate pipeline, and no xor-shift avalanche anywhere in the design |
| **xxHash / XXH3** | Four accumulators updated **independently**, 32-byte stripes with one 8-byte lane per accumulator, `acc = rotl(acc + lane * P2, 31) * P1`, accumulators merged by a rotate-1/7/12/18 sum plus merge rounds, length added at the end, xor-shift-multiply avalanche with shifts 33/29/32; XXH3 adds a 192-byte secret and a scramble step | The opposite arrangement: the lanes are never independent, they form a ring in which each lane is updated from the previous one, so two words feed four lanes. No per-lane message word, no prime-multiplied message, no merge formula, no secret, no xor-shift avalanche |
| **CityHash** | Separate code paths for inputs of ≤16, ≤32, ≤64 and >64 bytes, a rolling multi-word state over 64-byte chunks for long inputs, `ShiftMix` (`x ^= x >> 47`) and 128-bit multiply-based mixes, little-endian unaligned loads | One uniform path for every length with no branching on size, no `ShiftMix`, no 128-bit intermediate products, and big-endian explicit loads |
| **FarmHash** | CityHash's successor: the same length-branched family with additional SIMD and CRC-assisted paths | Differs for the same reasons as CityHash, and uses no platform-specific instructions at all |
| **FNV / FNV-1a** | A single accumulator, one byte at a time, `h = (h XOR byte) * prime`, starting from an offset basis | Processes 16 bytes at a time into four lanes rather than one byte into one accumulator. The `XOR then multiply` primitive does appear here as `B = (B XOR A) * MUL_B`, but what is XORed in is another *state lane*, not an input byte, and it sits inside a four-lane ring rather than being the entire algorithm |
| **MetroHash** (extra) | Four lanes, 32-byte blocks, each lane absorbs its own 8-byte word as `v += m * k`, then `v = rotr(v, 29) + v_neighbour` | The nearest relative in spirit, and structurally different: 16-byte blocks with two words for four lanes instead of one word per lane, the message is never multiplied, the multiplications act on XORed state, four distinct rotation amounts in one direction instead of a single shared `rotr 29`, plus position tags and a drain phase that MetroHash has no equivalent of |
| **SpookyHash / HighwayHash / Jenkins** (extra) | SpookyHash: twelve 64-bit variables over 96-byte blocks. HighwayHash: 4×4 SIMD lanes with multiply-permute and a zipper merge. Jenkins: shift-and-add mixing over three words | Different state sizes and shapes, no SIMD or permute-based merging, and no shift-and-add-only mixing |

Questions asked explicitly during the second review, after the code was
working:

* *Was a compression function reproduced?* No. There is no compression
  function in the Merkle-Damgård sense: no chaining variable is compressed
  together with a block and fed forward.
* *Is the state update sequence recognisably the same as some existing one?*
  No. The one-directional ring `A → B → C → D → A` with two message entry
  points, two state-side multiplications and two extra cross links does not
  match any of the update patterns above.
* *Is the block structure nearly identical to an existing one?* The 16-byte
  block size coincides with MurmurHash3's x64 variant, but the way a block is
  consumed is unrelated.
* *Were known rotation schedules reproduced?* No. The set 55, 10, 17, 35 was
  derived by the documented procedure and matches no schedule among the
  families reviewed.
* *Were known multipliers reproduced?* No. All nine 64-bit constants were
  checked against the published constants of the reviewed algorithms.
* *Was an existing finalizer reproduced?* No. The finalization is repeated
  applications of the same step with changing tags. In particular the
  xor-shift-multiply avalanche shared by MurmurHash, xxHash and the SplitMix
  family is deliberately absent, as is the SipHash-style `XOR the lanes
  together` output.
* *Was padding or length processing copied?* No. There is no `0x80`
  delimiter, no `pad10*1`, and no length field appended to a final padded
  block. The tail length lives in a tag and the total length is absorbed by a
  step of its own.
* *Is this an existing algorithm with different constants?* No. Replacing the
  constants of any algorithm above would not produce this structure, and
  replacing the constants here does not produce any of those.

What remains genuinely shared with prior art is the vocabulary rather than the
construction: modular addition, XOR, bit rotation and multiplication by fixed
odd constants are the standard primitives available to any such design, and
they are used here in an independently chosen arrangement.
