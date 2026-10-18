# cat-ui 🐈

Shared look and feel for all the cat plugins (JUCE): wood side cheeks, knobs, header with info, settings and "reset to 100%", resizable from every corner and edge with locked aspect ratio.

## Use it
```cmake
include(FetchContent)
FetchContent_Declare(cat_ui GIT_REPOSITORY https://github.com/SpeakingColors/cat-ui GIT_TAG main)
FetchContent_MakeAvailable(cat_ui)
target_link_libraries(MyApp PRIVATE cat_ui)
```

## Build the demo
```
cmake -S . -B build -DCAT_UI_BUILD_DEMO=ON
cmake --build build --config Release
```
GitHub Actions builds it on Linux, Windows and macOS on every push.

## Status
Early alpha. Free and open source. Donations welcome (link coming).

## License
AGPL-3.0-or-later, see [LICENSE](LICENSE).

Not affiliated with or endorsed by any hardware maker named here. All trademarks belong to their owners.
