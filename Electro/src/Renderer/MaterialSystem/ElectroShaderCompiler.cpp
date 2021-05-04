//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ElectroShaderCompiler.hpp"
#include "Core/System/ElectroOS.hpp"
#include "Renderer/Interface/ElectroShader.hpp"
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
            stage = "vertex"; break;

        case ShaderDomain::PIXEL:
            extension = "pixel.hlsl";
            stage = "fragment"; break;

        case ShaderDomain::COMPUTE:
            extension = "compute.hlsl";
            stage = "compute"; break;
        }

        String filepath = "Electro/assets/SPIRvCache/" + OS::GetNameWithoutExtension(name) + extension;
        String spvFilePath = filepath + ".spv";

        // Make sure we have the required folder
        OS::CreateOrEnsureFolderExists("Electro/assets/SPIRvCache");

        //The .spv file already exists, get that
        if (OS::FileExists(spvFilePath.c_str()))
        {
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

            return SPIRVHandle(result, OS::GetNameWithExtension(spvFilePath.c_str()), domain);
        }

        //The SPIR-V(.spv) is not in the cache, so generate one
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
        OS::RunInTerminal(command.c_str());

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

        return SPIRVHandle(result, OS::GetNameWithExtension(spvFilePath.c_str()), domain);
    }

    static void PrepareCompilation(spirv_cross::Compiler& compiler)
    {
        Uint sampler = compiler.build_dummy_sampler_for_combined_images();
        if (sampler != 0)
        {
            compiler.set_decoration(sampler, spv::DecorationDescriptorSet, 0);
            compiler.set_decoration(sampler, spv::DecorationBinding, 0);
        }

        compiler.build_combined_image_samplers();
        for (auto& remap : compiler.get_combined_image_samplers())
            compiler.set_name(remap.combined_id, spirv_cross::join("Electro_SPIRV_Cross_Combined_", compiler.get_name(remap.image_id), compiler.get_name(remap.sampler_id)));
    }

    void ShaderCompiler::Reflect(const SPIRVHandle& spirv, const String& shaderName, bool print)
    {
        spirv_cross::Compiler compiler(spirv.SPIRV);
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        //TODO: Make a struct/class ReflectionData ans store the information these
        // Later pass this ReflectionData to the shader and the material
        // Then material can validate the shader resources before setting them, bind slot is also availabe,
        // So no more hardcoded binding value!

        if (print)
        {
            String shaderStage = "";
            switch (spirv.Domain)
            {
                case ShaderDomain::VERTEX: shaderStage = "Vertex"; break;
                case ShaderDomain::PIXEL: shaderStage = "Pixel"; break;
                case ShaderDomain::COMPUTE: shaderStage = "Compute"; break;
            }

            ELECTRO_TRACE("||||||||||||||||||| %s - %s |||||||||||||||||||", shaderName.c_str(), shaderStage.c_str());

            ELECTRO_TRACE("---------------Constant buffers---------------");
            for (const spirv_cross::Resource& resource : resources.uniform_buffers)
            {
                const auto& bufferType = compiler.get_type(resource.base_type_id);
                Uint bufferSize = compiler.get_declared_struct_size(bufferType);
                Uint binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                Uint memberCount = bufferType.member_types.size();

                ELECTRO_TRACE("  %s", resource.name.c_str());
                ELECTRO_TRACE("    Size = %i", bufferSize);
                ELECTRO_TRACE("    Binding = %i", binding);
                ELECTRO_TRACE("    MemberCount = %i", memberCount);
            }
            ELECTRO_TRACE("----------------------------------------------");


            ELECTRO_TRACE("---------------Resources---------------");
            for (const spirv_cross::Resource& resource : resources.separate_images)
            {
                Uint set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
                Uint binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
                ELECTRO_TRACE("Texture: %s - Binding = %i", resource.name.c_str(), binding);
            }
            ELECTRO_TRACE("----------------------------------------------");
            ELECTRO_TRACE("|||||||||||||||||||||||||||||||||||||||||||||||||||||||||");
        }
    }

    String ShaderCompiler::CrossCompileToGLSL(const SPIRVHandle& spirv)
    {
        E_ASSERT(!spirv.SPIRV.empty(), "Invalid SPIR-V handle!");

        spirv_cross::CompilerGLSL compiler(spirv.SPIRV);
        PrepareCompilation(compiler);
        String result = compiler.compile();
        return result;
    }

    String ShaderCompiler::CrossCompileToHLSL(const SPIRVHandle& spirv)
    {
        E_ASSERT(!spirv.SPIRV.empty(), "Invalid SPIR-V handle!");

        spirv_cross::CompilerHLSL compiler(spirv.SPIRV);
        PrepareCompilation(compiler);

        spirv_cross::CompilerHLSL::Options options;

        //Set the shader model to 5.0
        options.shader_model = 50;
        compiler.set_hlsl_options(options);

        String result = compiler.compile();
        return result;
    }
}
