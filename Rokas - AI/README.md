# Educational 256-bit hash function (AI-assisted implementation)

An originally designed hash function for a university Blockchain Technologies
assignment. This is the AI-assisted track; the manual track shares no code.

> **This is an educational custom hash function. It has not undergone cryptographic
> analysis and must not be used to protect passwords, credentials, financial data,
> signatures, authentication systems, or other security-sensitive information.**

## Build and use

Requires C++20 and CMake 3.20+. Compiles warning free with `-Wall -Wextra -Wpedantic`.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build
./build/hash-generator --text "hello"      # 2607ba4e...43936c54, 64 hex characters
./build/hash-generator --file file.bin     # --text "" is valid and hashes empty input
./build/sanity-checks && ./tests/run_sanity.sh build
```

Exit status is `0` on success, `1` for a usage error and `2` when a file cannot be
opened or read; a read failure is never silently hashed as empty input.

## Algorithm

The internal state is **eight 64-bit lanes (512 bits)**; the digest is **256 bits**, made
by folding lane pairs, so only half as many bits as the state holds are ever published.
One mixing step consumes one 32-byte block, read as four big-endian words:

1. the words enter lanes 0, 2, 4 and 6, alternating addition and XOR so the entry points
   differ algebraically, and the block's position tag rides along with the first word;
2. a **forward sweep** updates lanes 1…7, each from the lane below it;
3. a **backward sweep** updates lanes 6…0, each from the lane above it.

Both sweeps multiply by a fixed odd constant (the non-linear part) and rotate, feeding
the high bits a multiplication produces back into low positions. Going both ways makes
every lane depend on every other lane and on all four words within one step, and odd
multipliers keep each operation invertible, so the state never collapses. The tail needs
no delimiter byte: leftover bytes go to the front of a zero-filled block and their count
into that step's tag, which is what keeps `"ab"` apart from `"ab\0"`.

```text
state s[0..7] = LANE_INIT                                 # 512 bits, mod 2^64
procedure STEP(w0, w1, w2, w3, tag):
    s[0] += w0 XOR tag ;  s[2] ^= w1 ;  s[4] += w2 ;  s[6] ^= w3
    for i = 1 to 7:       s[i] = (s[i] XOR rotl(s[i-1], 41)) * MUL_F
    for i = 6 down to 0:  s[i] = (s[i]  +  rotl(s[i+1], 53)) * MUL_B

function HASH(input):
    L = length(input) ;  n = L / 32 ;  r = L mod 32
    for i = 0 to n-1:                                     # whole blocks
        STEP(four big-endian words of block i, (i+1) * TAG_STEP)
    tail = last r bytes, followed by (32 - r) zero bytes
    STEP(four big-endian words of tail, (n+1)*TAG_STEP + (r+1)*TAG_TAIL)
    STEP(L, rotl(L, 32), 0, 0, TAG_END)                   # length
    for j = 1 to 3:  STEP(0, 0, 0, 0, TAG_END + j*TAG_STEP)    # no more input
    for i = 0 to 3:  out[i] = s[i] XOR rotl(s[i+4], 40)        # 512 -> 256
    return big-endian bytes of out[0..3]                  # 32 bytes
```

### Input and output

Any byte sequence works, including the empty one and binary data. `--text` hashes the
argument bytes as the shell delivered them (nothing trimmed, re-cased, normalised or
newline-terminated), `--file` the exact file contents in binary mode, never the name.
The digest is deterministic, depends on every byte, their order and the exact length, and
is always 32 bytes / 64 lowercase hex characters, the same on either endianness.

## Constants

All of them — eight lane initial values, two multipliers, three tags, three rotations —
come from one procedure unrelated to any existing hash: concatenate the decimal digits of
`n^n` for `n = 2, 3, 4, …`, cut into 20-digit chunks, reduce each modulo 2^64 with the
lowest bit set (so multipliers are odd and invertible) and keep it only if its population
count is 26…38; rotations follow as `(chunk mod 47) + 9`, required distinct. They are
compile-time constants in `src/custom_hash.cpp`, never generated at run time, and match
no published constant or rotation schedule below.

## Originality

Compared against MD5, SHA-1, SHA-2, SHA-3/Keccak, BLAKE2, BLAKE3, SipHash, MurmurHash3,
xxHash/XXH3, CityHash, FarmHash, FNV, MetroHash, SpookyHash and HighwayHash; it reuses
none of their constants, round functions, schedules or finalizers. What differs most:

* no Merkle-Damgård chaining variable or feed-forward and no message schedule — each
  message word is used exactly once, directly;
* not a sponge: a fold, not a rate/capacity split, is what holds state back;
* lanes are chained by a two-directional sweep rather than being independent accumulators
  (xxHash, MetroHash) or paired butterflies (SipHash);
* padding has no delimiter byte or appended length field, and the finalizer is the same
  step with changing tags plus a pairwise fold, not an xor-shift-multiply avalanche.

## Known limitations

* **Not analysed.** No claim is made that this is secure, collision resistant or
  preimage resistant; a later assignment stage investigates such properties. Measured
  diffusion shows only that obvious structure is absent.
* The previous revision kept a 256-bit state and published it unchanged, so a digest
  named the final state exactly and the computation could be unwound from it directly.
  The larger internal state and reduced output remove that obvious direct-state-reversal
  weakness. This does not establish cryptographic security or preimage resistance.
* Each 32-byte block gets a single step, so there is little margin against an attacker
  who chooses consecutive blocks. Multiplication carries influence upward only, so
  spreading downward depends on the rotations.
* No key, seed or salt input, and no third-party review.
