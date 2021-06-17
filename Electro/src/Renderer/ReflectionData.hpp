//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Interface/Shader.hpp"

namespace Electro
{
    enum class ShaderDomain;

    // Represents Textures
    struct ShaderResource
    {
        Uint Binding = 0;
        String Name = "";
    };

    // Represents a ConstantBuffer Member
    struct ShaderBufferMember
    {
        String Name = "";
        Uint MemoryOffset = 0;
    };

    // Represents a ConstantBuffer
    struct ShaderBuffer
    {
        Uint Binding = 0;
        String BufferName = "None";
        Uint Size = 0;
        Vector<ShaderBufferMember> Members = {};
    };

    class ShaderReflectionData
    {
    public:
        void SetDomain(const ShaderDomain& domain) { mShaderDomain = domain; }
        void PushResource(const ShaderResource& res);
        void PushBuffer(const ShaderBuffer& buffer);

        [[nodiscard]] const ShaderBuffer& GetBuffer(const String& name) const;
        [[nodiscard]] const ShaderBufferMember& GetBufferMember(const ShaderBuffer& buffer, const String& memberName) const;
        const Vector<ShaderResource>& GetResources() const { return mShaderResources; }
        const Vector<ShaderBuffer>& GetBuffers() const { return mShaderBuffers; }

        void ValidateBuffer(const ShaderBuffer& buffer);
    private:
        ShaderDomain mShaderDomain = ShaderDomain::None;
        Vector<ShaderResource> mShaderResources;
        Vector<ShaderBuffer> mShaderBuffers;
    };
}
