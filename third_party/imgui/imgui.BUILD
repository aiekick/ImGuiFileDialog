load("@rules_cc//cc:defs.bzl", "cc_library")

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "imgui",
    srcs = glob([
        "backends/imgui_impl_opengl3*",
        "backends/imgui_impl_glfw*",
        "backends/imgui_impl_opengl3*",
        "*.h",
        "*.cpp",
    ]),
    hdrs = [
        "imgui.h",
        "imgui_internal.h",
    ],
    defines = [
        "IMGUI_DEFINE_MATH_OPERATORS",
        "ImDrawIdx=\"unsigned int\"",
    ],
    include_prefix = "imgui",
    includes = ["."],
    deps = [
        
    ],
)
