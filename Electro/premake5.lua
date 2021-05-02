project "Electro"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "On"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "epch.hpp"
    pchsource "src/epch.cpp"

    files
    {
        "src/**.hpp",
        "src/**.cpp",
        "vendor/stb_image/**.h",
        "vendor/stb_image/**.cpp",
        "vendor/glm/glm/**.hpp",
        "vendor/glm/glm/**.inl",
	}

    defines
    {
        "_CRT_SECURE_NO_WARNINGS",
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
        "ImGui",
        "Yaml",
        "%{LibraryDir.Assimp}",
        "%{LibraryDir.Mono}",
        "%{LibraryDir.PhysX}",
        "%{LibraryDir.PhysXCharacterKinematic}",
        "%{LibraryDir.PhysXCommon}",
        "%{LibraryDir.PhysXCooking}",
        "%{LibraryDir.PhysXExtensions}",
        "%{LibraryDir.PhysXFoundation}",
        "%{LibraryDir.PhysXPvd}",
        "%{LibraryDir.Shaderc}",
        "%{LibraryDir.SpirvCrossCore}",
        "%{LibraryDir.SpirvCrossGLSL}",
        "%{LibraryDir.SpirvCrossHLSL}"
    }

    defines
    {
        "PX_PHYSX_STATIC_LIB"
    }

    filter "system:windows"
        systemversion "latest"

        links
        {
            "d3d11",
            "dxgi",
            "dxguid",
            "d3dCompiler"
        }

    filter "configurations:Debug"
        defines "E_DEBUG"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "Full"

        defines
        {
            "E_RELEASE",
            "NDEBUG"
        }

    filter "configurations:Dist"
        defines "E_DIST"
        runtime "Release"
        optimize "Full"
