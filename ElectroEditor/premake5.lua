project "ElectroEditor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "On"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "src/**.hpp",
        "src/**.cpp"
    }

    includedirs
    {
        "%{IncludeDir.Src}",
        "%{IncludeDir.Editor}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.stb_image}",
        "%{IncludeDir.Assimp}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.EnTT}",
        "%{IncludeDir.Yaml}",
        "%{IncludeDir.ElectroMono}",
        "%{IncludeDir.PhysX}",
        "%{IncludeDir.SPIR_V}"
    }

    links
    {
        "Electro"
    }

    filter "system:windows"
        systemversion "latest"

    filter "configurations:Debug"
        defines "E_DEBUG"
        runtime "Debug"
        symbols "On"

        postbuildcommands
        {
            '{COPY} "%{wks.location}/Electro/vendor/Assimp/lib/assimp-vc142-mt.dll" "%{cfg.targetdir}"',
            '{COPY} "%{wks.location}/Electro/vendor/ElectroMono/bin/Debug/mono-2.0-sgen.dll" "%{cfg.targetdir}"',
            '{COPY} "%{wks.location}/Electro/vendor/SPIR-V/lib/shaderc_shared.dll" "%{cfg.targetdir}"',
        }

    filter "configurations:Release"
        runtime "Release"
        optimize "Full"

        defines
        {
            "E_RELEASE",
            "NDEBUG"
        }

        postbuildcommands
        {
            '{COPY} "%{wks.location}/Electro/vendor/Assimp/lib/assimp-vc142-mt.dll" "%{cfg.targetdir}"',
            '{COPY} "%{wks.location}/Electro/vendor/ElectroMono/bin/Release/mono-2.0-sgen.dll" "%{cfg.targetdir}"',
            '{COPY} "%{wks.location}/Electro/vendor/SPIR-V/lib/shaderc_shared.dll" "%{cfg.targetdir}"',
        }

    filter "configurations:Dist"
        defines "E_DIST"
        runtime "Release"
        optimize "Full"

        postbuildcommands
        {
            '{COPY} "%{wks.location}/Electro/vendor/Assimp/lib/assimp-vc142-mt.dll" "%{cfg.targetdir}"',
            '{COPY} "%{wks.location}/Electro/vendor/ElectroMono/bin/Release/mono-2.0-sgen.dll" "%{cfg.targetdir}"',
            '{COPY} "%{wks.location}/Electro/vendor/SPIR-V/lib/shaderc_shared.dll" "%{cfg.targetdir}"',
        }
