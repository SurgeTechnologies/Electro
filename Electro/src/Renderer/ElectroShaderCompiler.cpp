//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroShaderCompiler.hpp"
#include "Core/System/ElectroOS.hpp"
#include "Interface/ElectroShader.hpp"
#include <SPIRV-Cross/spirv.hpp>
#include <SPIRV-Cross/spirv_glsl.hpp>
#include <SPIRV-Cross/spirv_hlsl.hpp>

namespace Electro
{
    SPIRVHandle ShaderCompiler::CompileToSPIRv(const String& name, const String& shaderSource, const ShaderDomain& domain)
    {
        Vector<Uint> result;
        String extension = "";
        String stage = "";

        switch (domain)
        {
            case ShaderDomain::NONE:    E_INTERNAL_ASSERT("Shader type NONE is invalid in this context!"); break;
            case ShaderDomain::VERTEX:
                extension = "vert.hlsl";
                stage     = "vertex"; break;

            case ShaderDomain::PIXEL:
                extension = "pixel.hlsl";
                stage     = "fragment"; break;

            case ShaderDomain::COMPUTE:
                extension = "compute.hlsl";
                stage     = "compute"; break;
        }

        String filepath = "Electro/assets/SPIRvCache/" + OS::GetNameWithoutExtension(name) + extension;
        String spvFilePath = filepath + ".spv";
        std::ofstream out(filepath, std::ios::out);
        if (out)
        {
            out.write(shaderSource.c_str(), shaderSource.size());
            out.flush();
            out.close();
        }
        else
            ELECTRO_ERROR("Cannot open filepath %s", filepath.c_str());

        String command = "glslc.exe -fshader-stage=" + stage + " -c " + filepath + " -o " + spvFilePath;
        system(command.c_str());

        std::ifstream in(spvFilePath, std::ios::in | std::ios::binary);
        if (in.is_open())
        {
            in.seekg(0, std::ios::end);
            auto size = in.tellg();
            in.seekg(0, std::ios::beg);

            result.resize(size / sizeof(Uint));
            in.read((char*)result.data(), size);
            in.close();
        }
        else
            ELECTRO_ERROR("Cannot open filepath %s", spvFilePath.c_str());

        return SPIRVHandle(result, OS::GetNameWithExtension(spvFilePath.c_str()));
    }

    String ShaderCompiler::CrossCompileToGLSL(const SPIRVHandle& spirv)
    {
        E_ASSERT(!spirv.SPIRV.empty(), "Invalid SPIR-V handle!");

        String result = String();
        spirv_cross::CompilerGLSL compiler(spirv.SPIRV);

        Uint sampler = compiler.build_dummy_sampler_for_combined_images();
        if (sampler != 0)
        {
            compiler.set_decoration(sampler, spv::DecorationDescriptorSet, 0);
            compiler.set_decoration(sampler, spv::DecorationBinding, 0);
        }

        compiler.build_combined_image_samplers();
        for (auto& remap : compiler.get_combined_image_samplers())
            compiler.set_name(remap.combined_id, spirv_cross::join("Electro_SPIRV_Cross_Combined_", compiler.get_name(remap.image_id), compiler.get_name(remap.sampler_id)));

        result = compiler.compile();
        std::cout << result << std::endl;
        std::cout << "-----------------------------------------------------------------------" << std::endl;
        return result;
    }
}
