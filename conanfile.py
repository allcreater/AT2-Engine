from conans import ConanFile, CMake
import conans.tools as tools
import shutil, os

class AT2(ConanFile):
   settings = "os", "compiler", "build_type", "arch"
   requires = ["glm/0.9.9.8", "stb/20200203", "glad/0.1.34", "nlohmann_json/3.9.1"]

   generators = "cmake"
   options = {
      "use_sdl": ["ON", "OFF", "True", "False"],
      "use_assimp": ["ON", "OFF", "True", "False"],
      "use_gtest": ["ON", "OFF", "True", "False"]
   }
   default_options = {
      "glad:gl_version": "4.5", #todo: pass from outside, must be 4.1 or 4.5
      "glad:gl_profile" : "core",
      "glad:extensions" : "GL_ARB_texture_filter_anisotropic, GL_ARB_seamless_cubemap_per_texture, GL_ARB_debug_output"
    }

   def requirements(self):
      if self.options.use_sdl in ["ON", "True"]:
         self.requires("sdl/[>=2.0.18]")
      else:
         self.requires("glfw/[>=3.3.2]")

      if self.options.use_gtest in ["ON", "True"]:
         self.requires("gtest/[>=1.10.0]")

      if self.options.use_assimp in ["ON", "True"]:
         self.requires("assimp/[>=5.0.1]")

   def imports(self):
      self.copy("*.dll", dst="bin", src="bin")
      self.copy("*.dylib*", dst="bin", src="lib")

   def configure(self):
      if not os.path.isdir("fx-gltf"):
         git = tools.Git(folder="fx-gltf")
         git.clone("https://github.com/jessey-git/fx-gltf")

   def build(self):
      cmake = CMake(self)
      cmake.configure()
      cmake.build()