//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroBase.hpp"

namespace Electro
{
    enum class ShaderDomain;
    enum class ShaderDataType;

    struct ShaderResource
    {
        Uint Binding;
        String Name;
    };

    struct ShaderBufferMember
    {
        String Name;
        ShaderDataType Type;
        Uint MemoryOffset;
    };

    struct ShaderBuffer
    {
        Uint Binding;
        String BufferName;
        Uint Size;
        Vector<ShaderBufferMember> Members;
    };

    class ShaderReflectionData
    {
    public:
        const void SetDomain(const ShaderDomain& domain) { mShaderDomain = domain; }
        const void PushResource(const ShaderResource& res);
        const void PushBuffer(const ShaderBuffer& buffer);

        const ShaderBuffer& GetBuffer(const String& name) const;
        const ShaderBufferMember& GetBufferMember(const ShaderBuffer& buffer, const String& memberName) const;
        Vector<ShaderResource>& GetResources() { return mShaderResources; }
        Vector<ShaderBuffer>& GetBuffers() { return mShaderBuffers; }

        const void ValidateBuffer(const ShaderBuffer& buffer);
    private:
        ShaderDomain mShaderDomain;
        Vector<ShaderResource> mShaderResources;
        Vector<ShaderBuffer> mShaderBuffers;
    };
}
