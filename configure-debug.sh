set -x
set -e

[ -d "./venv" ] && source venv/bin/activate || python -m venv venv && source venv/bin/activate && pip install conan
[ -d "./build-debug" ] || mkdir build-debug

conan install . --output=build-debug --build=missing --profile=debug

export CMAKE_MAKE_PROGRAM=/usr/bin/ninja

cmake -Bbuild-debug -S.\
    -GNinja -DCMAKE_CXX_COMPILER=g++\
    -DCMAKE_C_COMPILER=gcc\
    -DCMAKE_EXPORT_COMPILE_COMMANDS=On\
    -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake\
    -DCMAKE_BUILD_TYPE=Debug

cp build-debug/compile_commands.json ..
