# README.md

## Cloning & Building

```bash
  # Where $LOCATION is where you want to clone to
  git clone --branch master --recurse-submodules --shallow-submodules https://github.com/xexaaron/AbyssEngine.git $LOCATION
  cd $LOCATION
  # Directory must be named build to work with certain development tools
  mkdir build
  cd build
  cmake ..
  # Default build mode is Debug
  cmake --build .
```

## Versions

- CMake 3.28.3
- C++ 23
- SPIR-V 1.6
- Vulkan 1.3.250
