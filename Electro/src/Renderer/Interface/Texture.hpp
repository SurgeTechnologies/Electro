//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Ref.hpp"
#include "Asset/AssetBase.hpp"
#include "Renderer/Interface/Shader.hpp"
#include <glm/glm.hpp>

namespace Electro
{
    enum class TextureFormat
    {
        NONE       = 0,
        RGBA32F    = 1,
        RGBA8UNORM = 2,
        R32SINT    = 3,

        R32_VOID   = 4,
        DEPTH32    = 5,

        DEPTH = DEPTH32,
        SHADOW = R32_VOID
    };

    enum class TextureFlags : Uint
    {
        NONE             = E_BIT(0),
        RENDER_TARGET    = E_BIT(1),
        SHADER_RESOURCE  = E_BIT(2),
        DEPTH_STENCIL    = E_BIT(3),
        COMPUTE_WRITE    = E_BIT(4),
        CREATE_FROM_DISK = E_BIT(5),
        GENERATE_MIPS    = E_BIT(6),
        DEFAULT = CREATE_FROM_DISK | SHADER_RESOURCE
    };

    constexpr bool operator&(const enum TextureFlags selfValue, const enum TextureFlags inValue)
    {
        return static_cast<bool>(static_cast<Uint>(selfValue) & static_cast<Uint>(inValue));
    }

    constexpr enum TextureFlags operator|(const enum TextureFlags selfValue, const enum TextureFlags inValue)
    {
        return static_cast<enum TextureFlags>(static_cast<Uint>(selfValue) | static_cast<Uint>(inValue));
    }

    struct Texture2DSpecification
    {
        Texture2DSpecification() = default;
        Texture2DSpecification(Uint width, Uint height)
            : Width(width), Height(Height) {}

        Texture2DSpecification(const String & path)
            : Path(path) {}

        Uint Width = 0, Height = 0;
        String Path = "";
        TextureFlags Flags = TextureFlags::DEFAULT;
        TextureFormat Format = TextureFormat::NONE;
        bool SRGB = false;
    };

    class Texture2D : public Asset
    {
    public:
        virtual ~Texture2D() = default;

        virtual const Texture2DSpecification& GetSpecification() const = 0;

        // Returns the RendererID, used for the texture
        virtual RendererID GetRendererID() const = 0;

        // Returns true if the texture is loaded to Electro perfectly
        virtual bool Loaded() = 0;

        virtual void VSBindAsShaderResource(Uint slot) const = 0;
        virtual void PSBindAsShaderResource(Uint slot) const = 0;
        virtual void CSBindAsShaderResource(Uint slot) const = 0;
        virtual void CSBindAsUnorderedAccess(Uint slot) const = 0;
        virtual void BindAsRenderTarget() const = 0;

        virtual void VSUnbindShaderResource(Uint slot) const = 0;
        virtual void PSUnbindShaderResource(Uint slot) const = 0;
        virtual void CSUnbindShaderResource(Uint slot) const = 0;
        virtual void CSUnbindUnorderedAccess(Uint slot) const = 0;
        virtual void UnbindAsRenderTarget() const = 0;

        static Ref<Texture2D> Create(const Texture2DSpecification& spec);
    };

    class Cubemap : public IElectroRef
    {
    public:
        virtual ~Cubemap() = default;

        // Returns the RendererID, used for the texture
        virtual RendererID GetRendererID() const = 0;

        // Returns the Path of the HDR texture
        virtual String GetPath() const = 0;

        // Returns the file name
        virtual String const GetName() const = 0;

        // Binds the Cubemap to the pipeline
        virtual void VSBind(Uint slot = 0) const = 0;
        virtual void PSBind(Uint slot = 0) const = 0;
        virtual void CSBind(Uint slot = 0) const = 0;

        // Binds the Cubemap from the pipeline
        virtual void Unbind(Uint slot = 0, ShaderDomain domain = ShaderDomain::Pixel) const = 0;

        // Generates the PreFilter map for the texture cube
        virtual RendererID GenIrradianceMap() = 0;

        // Generates the irradince map for the texture cube
        virtual RendererID GenPreFilter() = 0;

        // Binds the irradince map which was generated for this texture cube, remember to generate one before calling this via GenIrradianceMap();
        virtual void BindIrradianceMap(Uint slot) const = 0;

        // Binds the PreFilter map which was generated for this texture cube, remember to generate one before calling this via GenPreFilter();
        virtual void BindPreFilterMap(Uint slot) const = 0;

        virtual bool operator==(const Cubemap& other) const = 0;

        // Calculates the MipMap count
        virtual Uint CalculateMipMapCount(Uint width, Uint height) = 0;

        static Ref<Cubemap> Create(const String& path);
    };
}
