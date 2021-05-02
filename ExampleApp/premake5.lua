workspace "ExampleApp"
    architecture "x64"
    runtime "Release"
    optimize "Full"

    configurations
    {
        "Release"
    }

project "ElectroScript-Core"
    location "%{wks.location}/ElectroScript-Core"
    kind "SharedLib"
    language "C#"

    targetdir ("bin/%{cfg.buildcfg}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "../ElectroScript-Core/src/**.cs"
    }

project "ExampleApp"
    location "%{wks.location}/ExampleApp"
    kind "SharedLib"
    language "C#"

    targetdir ("bin/%{cfg.buildcfg}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "src/**.cs",
    }

    links
    {
        "ElectroScript-Core"
    }