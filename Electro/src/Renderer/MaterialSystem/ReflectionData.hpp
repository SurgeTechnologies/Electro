//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"

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
        void SetDomain(const ShaderDomain& domain) { mShaderDomain = domain; }
        void PushResource(const ShaderResource& res);
        void PushBuffer(const ShaderBuffer& buffer);

        E_NODISCARD const ShaderBuffer& GetBuffer(const String& name) const;
        E_NODISCARD const ShaderBufferMember& GetBufferMember(const ShaderBuffer& buffer, const String& memberName) const;
        Vector<ShaderResource>& GetResources() { return mShaderResources; }
        Vector<ShaderBuffer>& GetBuffers() { return mShaderBuffers; }

        void ValidateBuffer(const ShaderBuffer& buffer);
    private:
        ShaderDomain mShaderDomain;
        Vector<ShaderResource> mShaderResources;
        Vector<ShaderBuffer> mShaderBuffers;
    };
}
