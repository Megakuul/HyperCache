from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps


class hc_dbRecipe(ConanFile):
    name = "hc_db"
    package_type = "application"

    license = "GPLv3"
    author = "Linus Ilian Moser megakuul@gmx.ch"
    url = "https://github.com/megakuul/hypercache"
    description = "Core Database of the HyperCache System"

    settings = "os", "compiler", "build_type", "arch"

    exports_sources = "CMakeLists.txt", "src/*", "include/*"

    requires = "boost/1.83.0"

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
