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
# Classic dictionaries (_rdict.pcm + .rootmap) instead of C++ modules:
# - macOS: rootcling -cxxmodule crashes (bus error in clang's module macro
#   check, InjectModuleUtilHeader) against conda-forge's ROOT 6.38.06;
# - wherever ROOT itself has no runtime C++ modules: conda-forge cross-
#   compiles linux-aarch64 with -Druntime_cxxmodules=OFF, so there is no
#   etc/cling/module.modulemap for root-plus's modules to build on.
EXTRA_ARGS=()
if [[ "$(uname)" == "Darwin" ]] || [[ "$(root-config --has-runtime_cxxmodules)" != "yes" ]]; then
    EXTRA_ARGS+=(-DROOT_RUNTIME_CXXMODULES=OFF)
    CLASSIC_DICT=1
fi

cmake .. ${CMAKE_ARGS} -DCMAKE_INSTALL_PREFIX=$PREFIX "${EXTRA_ARGS[@]}"

NPROC=$(nproc 2>/dev/null || sysctl -n hw.ncpu)
make -j$NPROC
make install

# Classic dictionaries: repair the installed .rootmap files (enum template
# parameters declared first, core headers by their include path); see the
# script. Without it cling rejects their forward declarations.
if [[ "${CLASSIC_DICT:-0}" == "1" ]]; then
    "${PYTHON}" "${RECIPE_DIR}/scripts/fix_rootmaps.py" "${PREFIX}"
fi
