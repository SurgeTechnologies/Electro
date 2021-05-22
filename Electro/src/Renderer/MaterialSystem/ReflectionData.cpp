//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ReflectionData.hpp"
#include "Renderer/Interface/Shader.hpp"

namespace Electro
{
    ShaderBuffer dummyBuffer = ShaderBuffer();
    ShaderBufferMember dummyBufferMember = ShaderBufferMember();

    void ShaderReflectionData::PushResource(const ShaderResource& res)
    {
        mShaderResources.push_back(res);
    }

    void ShaderReflectionData::PushBuffer(const ShaderBuffer& buffer)
    {
        mShaderBuffers.push_back(buffer);
    }

    const ShaderBuffer& ShaderReflectionData::GetBuffer(const String& name) const
    {
        for (const ShaderBuffer& buffer : mShaderBuffers)
            if (buffer.BufferName == name)
                return buffer;

        ELECTRO_CRITICAL("ShaderBuffer with name %s doesn't exist in shader!", name.c_str());
        E_INTERNAL_ASSERT("Trying to access invalid ShaderBuffer!");
        return dummyBuffer;
    }

    const ShaderBufferMember& ShaderReflectionData::GetBufferMember(const ShaderBuffer& buffer, const String& memberName) const
    {
        for (const ShaderBufferMember& member : buffer.Members)
            if (member.Name == memberName)
                return member;

        ELECTRO_WARN("ShaderBufferMember with name %s doesn't exist in %s buffer!", memberName.c_str(), buffer.BufferName.c_str());
        E_INTERNAL_ASSERT("Trying to access invalid ShaderBufferMember!");
        return dummyBufferMember;
    }

    void ShaderReflectionData::ValidateBuffer(const ShaderBuffer& buffer)
    {
        if (buffer.BufferName == "" || buffer.Members.size() == 0 || buffer.Size == 0)
        {
            ELECTRO_ERROR("Invalid ShaderBuffer!");
            E_INTERNAL_ASSERT("ShaderBuffer is invalid!");
        }
    }
}
