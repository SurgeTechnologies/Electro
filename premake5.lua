include "Premake/Custom/CleanBuild.lua"

workspace "Electro"
    architecture "x86_64"
    startproject "ElectroEditor"
    debugdir "%{wks.location}"

    configurations
    {
        "Debug",
        "Release",
        "Dist"
    }

    flags
    {
        "MultiProcessorCompile"
    }

    outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

    group "Dependencies"
        include "Electro/vendor/imgui"
        include "Electro/vendor/Yaml"
    group ""

IncludeDir = {}
IncludeDir["Src"]         = "%{wks.location}/Electro/src"
IncludeDir["Editor"]      = "%{wks.location}/ElectroEditor/src"
IncludeDir["ImGui"]       = "%{wks.location}/Electro/vendor/ImGui"
IncludeDir["stb_image"]   = "%{wks.location}/Electro/vendor/stb_image"
IncludeDir["Assimp"]      = "%{wks.location}/Electro/vendor/Assimp/include"
IncludeDir["glm"]         = "%{wks.location}/Electro/vendor/glm"
IncludeDir["EnTT"]        = "%{wks.location}/Electro/vendor/EnTT/include"
IncludeDir["Yaml"]        = "%{wks.location}/Electro/vendor/Yaml/include"
IncludeDir["ElectroMono"] = "%{wks.location}/Electro/vendor/ElectroMono/include"
IncludeDir["PhysX"]       = "%{wks.location}/Electro/vendor/PhysX/include"
IncludeDir["SPIR_V"]      = "%{wks.location}/Electro/vendor/SPIR-V/include"

LibraryDir = {}
LibraryDir["Assimp"]                  = "%{wks.location}/Electro/vendor/assimp/lib/assimp-vc142-mt.lib"
LibraryDir["Mono"]                    = "%{wks.location}/Electro/vendor/ElectroMono/lib/mono-2.0-sgen.lib"
LibraryDir["PhysX"]                   = "%{wks.location}/Electro/vendor/PhysX/lib/%{cfg.buildcfg}/PhysX_static_64.lib"
LibraryDir["PhysXCharacterKinematic"] = "%{wks.location}/Electro/vendor/PhysX/lib/%{cfg.buildcfg}/PhysXCharacterKinematic_static_64.lib"
LibraryDir["PhysXCommon"]             = "%{wks.location}/Electro/vendor/PhysX/lib/%{cfg.buildcfg}/PhysXCommon_static_64.lib"
LibraryDir["PhysXCooking"]            = "%{wks.location}/Electro/vendor/PhysX/lib/%{cfg.buildcfg}/PhysXCooking_static_64.lib"
LibraryDir["PhysXExtensions"]         = "%{wks.location}/Electro/vendor/PhysX/lib/%{cfg.buildcfg}/PhysXExtensions_static_64.lib"
LibraryDir["PhysXFoundation"]         = "%{wks.location}/Electro/vendor/PhysX/lib/%{cfg.buildcfg}/PhysXFoundation_static_64.lib"
LibraryDir["PhysXPvd"]                = "%{wks.location}/Electro/vendor/PhysX/lib/%{cfg.buildcfg}/PhysXPvdSDK_static_64.lib"
LibraryDir["Shaderc"]                 = "%{wks.location}/Electro/vendor/SPIR-V/lib/shaderc_shared.lib"
LibraryDir["SpirvCrossCore"]          = "%{wks.location}/Electro/vendor/SPIR-V/lib/spirv-cross-core.lib"
LibraryDir["SpirvCrossGLSL"]          = "%{wks.location}/Electro/vendor/SPIR-V/lib/spirv-cross-glsl.lib"
LibraryDir["SpirvCrossHLSL"]          = "%{wks.location}/Electro/vendor/SPIR-V/lib/spirv-cross-hlsl.lib"

include "Electro"
include "ElectroEditor"
include "ElectroScript-Core"

-- Example App
include "ExampleApp"
