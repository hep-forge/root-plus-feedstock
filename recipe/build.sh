#! /usr/bin/bash

mkdir build-scripts
cd build-scripts

cmake $RECIPE_DIR/scripts
cd ..

mkdir build
cd build

# ${CMAKE_ARGS} carries conda-build's own -DCMAKE_BUILD_TYPE=Release
# (plus toolchain/strip paths) -- omitting it leaves CMAKE_BUILD_TYPE
# unset (this project's own CMakeLists.txt never defaults it either),
# producing an unoptimized, unstripped debug-info binary.
# macOS: rootcling -cxxmodule crashes (bus error in clang's module macro
# check, InjectModuleUtilHeader) against conda-forge's ROOT 6.38.06, so build
# classic dictionaries there (_rdict.pcm + .rootmap), which ROOT loads next to
# its own modules. Linux keeps runtime C++ modules.
EXTRA_ARGS=()
if [[ "$(uname)" == "Darwin" ]]; then
    EXTRA_ARGS+=(-DROOT_RUNTIME_CXXMODULES=OFF)
fi

cmake .. ${CMAKE_ARGS} -DCMAKE_INSTALL_PREFIX=$PREFIX "${EXTRA_ARGS[@]}"

NPROC=$(nproc 2>/dev/null || sysctl -n hw.ncpu)
make -j$NPROC
make install
