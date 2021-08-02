//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Core/Base.hpp"
#include "Core/Log.hpp"


namespace Electro
{
    struct Buffer
    {
        Buffer()
            : Data(nullptr), Size(0) {}

        Buffer(void* data, Uint size)
            : Data(data), Size(size) {}

        Uint GetSize() const { return Size; }
        [[nodiscard]] void* GetData() const { return Data; }

        static Buffer Copy(const void* data, Uint size)
        {
            Buffer buffer;
            buffer.Allocate(size);
            memcpy(buffer.Data, data, size);
            return buffer;
        }

        void Allocate(Uint size)
        {
            delete[] Data;
            Data = nullptr;

            if (size == 0)
                return;

            Data = new byte[size];
            Size = size;
        }

        void Release()
        {
            delete[] Data;
            Data = nullptr;
            Size = 0;
        }

        void ZeroMem() const
        {
            if (Data)
                memset(Data, 0, Size);
        }

        template<typename T>
        T& Read(Uint offset = 0)
        {
            return *(T*)((byte*)Data + offset);
        }

        template<typename T>
        const T& Read(Uint offset = 0) const
        {
            return *(T*)((byte*)Data + offset);
        }

        byte* ReadBytes(Uint size, Uint offset) const
        {
            E_ASSERT(offset + size <= Size, "Buffer overflow!");
            byte* buffer = new byte[size];
            memcpy(buffer, (byte*)Data + offset, size);
            return buffer;
        }

        void Write(void* data, Uint size, Uint offset = 0) const
        {
            E_ASSERT(offset + size <= Size, "Buffer overflow!");
            memcpy((byte*)Data + offset, data, size);
        }

        operator bool() const
        {
            return Data;
        }

        byte& operator[](int index) { return ((byte*)Data)[index]; }
        byte operator[](int index) const { return ((byte*)Data)[index]; }

        template<typename T>
        T* As() { return (T*)Data; }
    private:
        void* Data;
        Uint Size;
    };

}
