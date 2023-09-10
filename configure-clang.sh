set -x
set -e

[ -d "./venv" ] && source venv/bin/activate || python -m venv venv && source venv/bin/activate && pip install conan
[ -d "./build-clang" ] || mkdir build-clang
export CC=clang
export CXX=clang

conan install . --output=build-clang --build=missing --profile=clang

cmake -Bbuild-clang -S.\
    -GNinja -DCMAKE_CXX_COMPILER=clang++\
    -DCMAKE_C_COMPILER=clang\
    -DCMAKE_EXPORT_COMPILE_COMMANDS=On\
    -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake\
    -DCMAKE_BUILD_TYPE=Debug
