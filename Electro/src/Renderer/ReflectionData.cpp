//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#include "epch.hpp"
#include "ReflectionData.hpp"

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

        Log::Critical("ShaderBuffer with name {0} doesn't exist in shader!", name);
        E_INTERNAL_ASSERT("Trying to access invalid ShaderBuffer!");
        return dummyBuffer;
    }

    const ShaderBufferMember& ShaderReflectionData::GetBufferMember(const ShaderBuffer& buffer, const String& memberName) const
    {
        for (const ShaderBufferMember& member : buffer.Members)
            if (member.Name == memberName)
                return member;

        Log::Warn("ShaderBufferMember with name {0} doesn't exist in {1} buffer!", memberName, buffer.BufferName);
        E_INTERNAL_ASSERT("Trying to access invalid ShaderBufferMember!");
        return dummyBufferMember;
    }

    void ShaderReflectionData::ValidateBuffer(const ShaderBuffer& buffer)
    {
        if (buffer.BufferName == "" || buffer.Members.size() == 0 || buffer.Size == 0)
        {
            Log::Error("Invalid ShaderBuffer!");
            E_INTERNAL_ASSERT("ShaderBuffer is invalid!");
        }
    }
}
