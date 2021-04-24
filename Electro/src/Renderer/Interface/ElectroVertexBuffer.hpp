//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/ElectroRef.hpp"

namespace Electro
{
    enum class ShaderDataType
    {
        None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
    };

    static Uint ShaderDataTypeSize(ShaderDataType type)
    {
        switch (type)
        {
            case ShaderDataType::Float:    return 4;
            case ShaderDataType::Float2:   return 4 * 2;
            case ShaderDataType::Float3:   return 4 * 3;
            case ShaderDataType::Float4:   return 4 * 4;
            case ShaderDataType::Mat3:     return 4 * 3 * 3;
            case ShaderDataType::Mat4:     return 4 * 4 * 4;
            case ShaderDataType::Int:      return 4;
            case ShaderDataType::Int2:     return 4 * 2;
            case ShaderDataType::Int3:     return 4 * 3;
            case ShaderDataType::Int4:     return 4 * 4;
            case ShaderDataType::Bool:     return 4;
        }
        E_INTERNAL_ASSERT("Unknown ShaderDataType!");
        return 0;
    }

    struct VertexBufferElement
    {
        String Name;
        ShaderDataType Type;
        Uint Size;
        size_t Offset;
        bool Normalized;

        VertexBufferElement() = default;

        VertexBufferElement(ShaderDataType type, const String& name, bool normalized = false)
            :Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
        {
        }

        Uint GetComponentCount() const
        {
            switch (Type)
            {
                case ShaderDataType::Float:  return 1;
                case ShaderDataType::Float2: return 2;
                case ShaderDataType::Float3: return 3;
                case ShaderDataType::Float4: return 4;
                case ShaderDataType::Mat3:   return 3 * 3;
                case ShaderDataType::Mat4:   return 4 * 4;
                case ShaderDataType::Int:    return 1;
                case ShaderDataType::Int2:   return 2;
                case ShaderDataType::Int3:   return 3;
                case ShaderDataType::Int4:   return 4;
                case ShaderDataType::Bool:   return 1;
            }
            E_INTERNAL_ASSERT("Unknown ShaderDataType!");
            return Size;
        }
    };

    class VertexBufferLayout
    {
    public:
        VertexBufferLayout() {}

        VertexBufferLayout(const std::initializer_list<VertexBufferElement>& elements)
            :mElements(elements)
        {
            CalculateOffsetsAndStride();
        }

        Uint GetStride() const { return mStride; }
        const Vector<VertexBufferElement>& GetElements() const { return mElements; }

        Vector<VertexBufferElement>::iterator begin() { return mElements.begin(); }
        Vector<VertexBufferElement>::iterator end() { return mElements.end(); }
        Vector<VertexBufferElement>::const_iterator begin() const { return mElements.begin(); }
        Vector<VertexBufferElement>::const_iterator end() const { return mElements.end(); }
    private:
        void CalculateOffsetsAndStride()
        {
            size_t offset = 0;
            mStride = 0;
            for (auto& element : mElements)
            {
                element.Offset = offset;
                offset += element.Size;
                mStride += element.Size;
            }
        }
    private:
        std::vector<VertexBufferElement> mElements;
        Uint mStride = 0;
    };

    class VertexBuffer : public IElectroRef
    {
    public:
        virtual ~VertexBuffer() = default;

        //Binds the VertexBuffer to the pipeline
        virtual void Bind() const = 0;

        //Unninds the VertexBuffer from the pipeline, this doesnt give 100% gurantee that it will Unbind it, may not work for all graphics API
        virtual void Unbind() const = 0;

        //Returns the Sayout of the buffer
        virtual const VertexBufferLayout& GetLayout() const = 0;

        //Sets the layout, useful for dynamic vartex buffer
        virtual void SetLayout(const VertexBufferLayout& layout) = 0;

        //Sets the actual raw data of the vertex buffer, useful for dynamic vertex buffer
        virtual void SetData(const void* data, Uint size) = 0;
    };
}
