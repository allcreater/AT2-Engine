from conans import ConanFile, CMake

class AT2(ConanFile):
   settings = "os", "compiler", "build_type", "arch"
   requires = "glfw/3.3.2", "glew/2.1.0", "glm/0.9.9.8", "stb/20200203", "assimp/5.0.1", "gtest/1.10.0", "glad/0.1.34", "nlohmann_json/3.9.1"
   generators = "cmake_find_package"
   default_options = { "glad:gl_version": "4.5" }
   #default_options = {"poco:shared": True, "openssl:shared": True} (example for future use)

   def imports(self):
      self.copy("*.dll", dst="bin", src="bin")
      self.copy("*.dylib*", dst="bin", src="lib")

   def build(self):
      cmake = CMake(self)
      cmake.configure()
      cmake.build()