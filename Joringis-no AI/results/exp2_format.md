# 2 eksperimentas. Išvesties formatas

Deklaruotas ilgis: 256 bitų = 64 hex simboliai, mažosios raidės, su pradiniais nuliais.
Tikrinama per API (tas pats kodas, kurį naudoja komandinė eilutė); komandinės eilutės
režimų (rankinis / --text / --file) sutapimas tikrinamas `tests/cli_checks.sh`.

| Failas | Baitų | 64 hex? | Ratas v0.1 |
|---|---:|:---:|---|
| `empty.bin` | 0 | taip | `25bf155273a700862f8583fec8712c06367d2fd6eb4234279b59c593300d98d2` |
| `a.bin` | 1 | taip | `d657166d0fbb82fe1df26bd98166263bb885a8dbc24b53c7dc5a3106af54587f` |
| `b.bin` | 1 | taip | `ed07e77ea02ad64ee327821255f539e3bd9926edc963698872706fc0a2ec3cc8` |
| `random_1.txt` | 1500 | taip | `7563094a47a7f3b619df600ec4bd02432b0de334665a481bb02cff4f8f71eb07` |
| `random_1_start.txt` | 1500 | taip | `7831617a2d23368ea14f5f59c0d59b86337b12580ea40bc637eb6959588e8b72` |
| `random_1_middle.txt` | 1500 | taip | `a87728b335a3f684746c5e2d09eff45156f6e2a48218f547beb76fcc9dda9cd2` |
| `random_1_end.txt` | 1500 | taip | `27acb26c13900f4f3a9df220a1964dde7e694a5ccd7f34e29148f3af47b834b1` |
| `random_2.txt` | 2048 | taip | `ca361ee66446f4845f37b6748dc7d0875e6eb90f407995f23b2f714387ffa212` |
| `random_2_start.txt` | 2048 | taip | `e86ed4cf458b2c1a660cd20ade63883845d3b0e35a3c542e72982fbfce5f4d65` |
| `random_2_middle.txt` | 2048 | taip | `fed01468155d6a2bda7de5c72f3ad97eef80cdb8a602558bf491529c555407e5` |
| `random_2_end.txt` | 2048 | taip | `7fbe6302aef47d16e01956a0766267806032e3c2e4c5a7ecb3da668c108b4afa` |
| `random_3.txt` | 4096 | taip | `bf0574e66c59816cfbf4edb288ad7fa6fd86b34ea44714915b03578fef1fef1a` |
| `random_3_start.txt` | 4096 | taip | `576863a17837552fdb9e5c5aa753469f7671b6924af96dae4e0965c35cd8d00f` |
| `random_3_middle.txt` | 4096 | taip | `cb1ce6677e337b7acc6136368032a9bcefdc6cc67a86b31ed9c7e32b444ddb02` |
| `random_3_end.txt` | 4096 | taip | `5d8793eb0f8b032b9fa8c1606aec945c7b05d7d7ed86166a1a3c3c9e7749ede5` |
| `struct_repeat_a.txt` | 32 | taip | `3494449e6eca6681a83914a773f78df514e679945d92fe21260ea7bbdf3dd687` |
| `struct_repeat_ab.txt` | 32 | taip | `f26c8a25f26c5cabcf20dff561aee201183b1bc308c9e2eaf409e545d5181bf5` |
| `struct_order_abc.txt` | 3 | taip | `0592e0a70d07a3c4d295d4179a5ccca5100b07a6a5900c823c79263c4732fba9` |
| `struct_order_cba.txt` | 3 | taip | `8bfde162bff34f7203e36e46ec967a5dd09db11f10c5dcfdc4842c583a96a58a` |
| `struct_order_words1.txt` | 11 | taip | `92f1a47fb9545b50f591739965597d0475dd41df1b753a0f368837617b3f6964` |
| `struct_order_words2.txt` | 11 | taip | `dd432e72fb3c3fabcb0984a18b448b13b50e5dc7b691e3e3b3c26d40ccd9e136` |
| `struct_space_none.txt` | 7 | taip | `b7e64dce66e8c28f9807ab4ca9ad27fe44ad2c9d1467c1c7e30202663fb14ecb` |
| `struct_space_none_copy.txt` | 7 | taip | `b7e64dce66e8c28f9807ab4ca9ad27fe44ad2c9d1467c1c7e30202663fb14ecb` |
| `struct_space_lead.txt` | 8 | taip | `adbb15f653be3021962ebff2ecc5144a93795dba1a3e8f216f6ab05488303af0` |
| `struct_space_trail.txt` | 8 | taip | `14efdde82bf23c5fce88866cfe04fd38f68cd801c40326a2bb34a9d1e30a183e` |
| `struct_newline_lf.txt` | 8 | taip | `24ef7e95a97721b67bb9c8697cfc3c17d03814a25e49f721ff1942d48a4da769` |
| `struct_newline_crlf.txt` | 9 | taip | `4958acbadeeb0fbd9c11eb353f6dca4efbea2efd8d9390cb16b0e013e111357c` |
| `struct_len15.txt` | 15 | taip | `0bde54252aa2136e3db2a6ebe2745ba7db2d42fb558d4abc76d0c441fe678f5d` |
| `struct_len16.txt` | 16 | taip | `542d5d8ef9481fe58bd14c894cdc43891ce9c82e0ba608bab803e2bf2da9de99` |
| `struct_len17.txt` | 17 | taip | `32e362a484bb183448a7b81cd64117e4d17a43dd7399d8c3c06c435984ed7e40` |
| `struct_pad_ab.txt` | 2 | taip | `47740998562307ee288347fa17a2216b4758f0a7bc59f896d72bdca29822c45a` |
| `struct_pad_ab0.txt` | 3 | taip | `bfa18bb37eb3caafc9265f7d39d38a6a59ffd2e46f81955dd288c2f442bdf4f1` |
| `utf8_lt.txt` | 26 | taip | `f0435b0b19975fdae2bc8fca7429defe7b0e35e37ee995f1c1dce0bfe887534b` |
| `utf8_mixed.txt` | 23 | taip | `f3322c1e9e6482a6135508ef2de89a30381842877413737c9a45a261e0bb811b` |

Netinkamo formato rezultatų: 0.
Sutampančios santraukos tarp failų (turi sutapti tik failai su vienodais baitais): 
- struct_space_none_copy.txt = struct_space_none.txt

Pradinio nulio pavyzdys: `nulis14` -> `0a52ab492b07c39b71b4022c6b0bda4c22e9b2ea5af65f9c8e518c882cead7fc` (64 simboliai).
