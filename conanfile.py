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
        self.requires("imgui/1.90-docking")
        
    def generate(self):
        copy(self, "*glfw*", os.path.join(self.dependencies["imgui"].package_folder,
             "res", "bindings"), os.path.join(self.source_folder, "src", "imgui", "backends"))
        copy(self, "*vulkan*", os.path.join(self.dependencies["imgui"].package_folder,
             "res", "bindings"), os.path.join(self.source_folder, "src", "imgui", "backends"))
