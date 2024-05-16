import os

from conan import ConanFile
from conan.tools.cmake import cmake_layout
from conan.tools.files import copy


class ImGuiExample(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("fmt/10.0.0")
        self.requires("glm/cci.20230113")
        self.requires("glfw/3.3.8")
        self.requires("catch2/3.4.0")
        self.requires("vulkan-headers/1.3.250.0")
        self.requires("reflect-cpp/0.6.0", options={"with_json": True})