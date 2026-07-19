# clang-tidy findings

Report-only sweep over all 248 of our C++ translation units, using the checks enabled in
`.clang-tidy`. Nothing here has been applied.

Reproduce with:

```bash
export PATH="/c/msys64/mingw64/bin:$PATH"
run-clang-tidy -p build/debug-win -quiet -j 8 \
  -header-filter='^C:[/\\]Users[/\\]alexs[/\\]git[/\\]firelight[/\\](src|libs[/\\]firelight|include[/\\]firelight)' \
  $(git ls-files '*.cpp' | grep -vE '^(thirdparty|libs/rcheevos|libs/discord|lib)/')
```

## Totals

2451 warning lines. Counts are per translation unit, so a finding in a widely-included header is
counted once per TU that includes it. The naming numbers below are deduplicated by file:line.

| Check | Count | Worth acting on? |
|---|---|---|
| `modernize-use-nodiscard` | 1148 | Mostly noise. It fires on every getter. Consider dropping the check from `.clang-tidy` rather than adding 1148 attributes |
| `readability-identifier-naming` | 778 (369 unique sites) | Yes - this is your `m_` / `UPPER_SNAKE_CASE` / `get*` ruleset. See below |
| `cppcoreguidelines-narrowing-conversions` | 168 | Worth a read. These are potential real bugs, not style |
| `cppcoreguidelines-pro-type-member-init` | 116 | Worth a read. Uninitialized members |
| `readability-convert-member-functions-to-static` | 53 | Cheap, safe |
| `readability-use-anyofallof` | 45 | Style preference; take it or drop the check |
| `google-default-arguments` | 45 | Virtual functions with default args - subtle, worth a look |
| `readability-avoid-const-params-in-decls` | 29 | Cheap, safe |
| everything else | ~70 | Small tail: `use-override`, `explicit-constructor`, `pass-by-value`, ... |

## Naming: 369 unique sites

By kind: 357 private member, 351 parameter, 34 protected member, 10 local variable,
10 constexpr variable, 5 global constant, 4 static constant, 3 struct, 3 function, 1 class constant.
(Sums past 369 because one site can trip several.)

Concentrated in a few files:

| File | Findings |
|---|---|
| `src/app/libretro/core.hpp` | 312 |
| `include/firelight/libretro/retropad.hpp` | 135 |
| `include/firelight/libretro/retropad_provider.hpp` | 37 |
| `src/app/achievements/gui/achievement_list_sort_filter_model.hpp` | 27 |
| `include/firelight/libretro/audio_output.hpp` | 27 |
| `libs/firelight/input/src/firelight/input/sdl_controller.hpp` | 25 |

**Do not run `--fix` across these.** Of the 778, **31 are in 3 files that expose `Q_PROPERTY` /
`Q_INVOKABLE` / `roleNames`**; renaming those still compiles and breaks the UI at runtime, because
QML resolves them dynamically. The other 747 are in non-QML-facing files and are safer, but
`core.hpp` alone is 312 of them and it is the libretro boundary - worth doing deliberately.

## Real defect found by this sweep

`clang-diagnostic-error` x3, all in `libs/firelight/cheats/tests/cheat_engine_test.cpp`:
`FakeRamCore` did not implement `ICore::sendKeyboardEvent` or `ICore::wantsKeyboard`, so it was
abstract and the target did not compile. `firelight_cheats_test` had been failing to build since
those methods were added to `ICore`; running the stale `.exe` still passed, which hid it.

Fixed in `libs/firelight/cheats/tests/fake_ram_core.hpp` - the two methods are now stubbed and the
target builds and passes 4/4.
