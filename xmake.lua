set_project("infman")
set_version("1.0.1")

add_rules("mode.debug", "mode.release")

set_targetdir("bin")

add_requires("raylib")

target("infman")
    set_kind("binary")
    add_files("src/*.c")
    add_packages("raylib")
    add_includedirs("include")
