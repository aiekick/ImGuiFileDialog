project("ImGuiFileDialog")
kind("StaticLib")
language("C++")
staticruntime("off")
warnings("off")

targetdir("bin/%{prj.name}")
objdir("bin-int/%{prj.name}")

files({
    "*.cpp",
    "*.h"
})
includedirs({ "./" })

filter("system:linux")
pic("On")
systemversion("latest")

filter("system:macosx")
pic("On")

filter("system:windows")
systemversion("latest")

filter("configurations:Debug")
runtime("Debug")
symbols("on")

filter("configurations:Release")
runtime("Release")
optimize("speed")

filter("configurations:Dist")
runtime("Release")
optimize("speed")
symbols("off")
