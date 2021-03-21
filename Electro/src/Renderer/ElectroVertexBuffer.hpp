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
            case ShaderDataType::Bool:     return 1;
        }
        E_INTERNAL_ASSERT("Unknown ShaderDataType!");
        return 0;
    }

    struct VertexBufferElement
    {
        String  Name;
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
            :m_Elements(elements)
        {
            CalculateOffsetsAndStride();
        }

        Uint GetStride() const { return m_Stride; }
        const Vector<VertexBufferElement>& GetElements() const { return m_Elements; }

        Vector<VertexBufferElement>::iterator begin() { return m_Elements.begin(); }
        Vector<VertexBufferElement>::iterator end() { return m_Elements.end(); }
        Vector<VertexBufferElement>::const_iterator begin() const { return m_Elements.begin(); }
        Vector<VertexBufferElement>::const_iterator end() const { return m_Elements.end(); }
    private:
        void CalculateOffsetsAndStride()
        {
            size_t offset = 0;
            m_Stride = 0;
            for (auto& element : m_Elements)
            {
                element.Offset = offset;
                offset += element.Size;
                m_Stride += element.Size;
            }
        }
    private:
        std::vector<VertexBufferElement> m_Elements;
        Uint m_Stride = 0;
    };

    class VertexBuffer : public IElectroRef
    {
    public:
        virtual ~VertexBuffer() = default;

        virtual void Bind() const = 0;
        virtual void Unbind() const = 0;

        virtual const VertexBufferLayout& GetLayout() const = 0;
        virtual void SetLayout(const VertexBufferLayout& layout) = 0;

        virtual void SetData(const void* data, Uint size) = 0;

        static Ref<VertexBuffer> Create(Uint size, VertexBufferLayout layout);
        static Ref<VertexBuffer> Create(void* vertices, Uint size, VertexBufferLayout layout);
    };

}
