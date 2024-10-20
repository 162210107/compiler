-- project basic configuration
add_rules("mode.debug")
set_warnings("all")
set_optimize("none")
set_languages("cxx20")
-- add third-party library requirements


target("Test")
    set_kind("binary")
    add_includedirs("Include")
    add_files("src/*.cpp")
    add_links("Kernel32")
    add_defines("UNICODE", "_UNICODE")  -- 定义 Unicode 宏