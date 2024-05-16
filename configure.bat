conan install . --output=build --build=missing --settings=build_type=Release --settings=compiler.cppstd=20
conan install . --output=build --build=missing --settings=build_type=Debug --settings=compiler.cppstd=20

cmake -Bbuild -S. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
glslc shaders/vertex.vert -o shaders/vertex.spv
glslc shaders/fragment.frag -o shaders/fragment.spv

xcopy /s/e/y .\shaders .\build\shaders

devenv.exe .\build\renderer.sln