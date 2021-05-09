//                    ELECTRO ENGINE
// Copyright(c) 2021 - Electro Team - All rights reserved
#pragma once
#include "Base.hpp"

namespace Electro
{
    class IElectroRef
    {
    public:
        void IncRefCount() const
        {
            mRefCount++;
        }
        void DecRefCount() const
        {
            mRefCount--;
        }

        Uint GetRefCount() const { return mRefCount; }
    private:
        mutable Uint mRefCount = 0;
    };

    template<typename T>
    class Ref
    {
    public:
        Ref()
            : mInstance(nullptr) {}

        Ref(std::nullptr_t n)
            : mInstance(nullptr) {}

        Ref(T* instance)
            : mInstance(instance)
        {
            static_assert(std::is_base_of<IElectroRef, T>::value, "Class is not RefCounted!");
            IncRef();
        }

        template<typename T2>
        Ref(const Ref<T2>& other)
        {
            mInstance = (T*)other.mInstance;
            IncRef();
        }

        template<typename T2>
        Ref(Ref<T2>&& other)
        {
            mInstance = (T*)other.mInstance;
            other.mInstance = nullptr;
        }

        ~Ref()
        {
            DecRef();
        }

        Ref(const Ref<T>& other)
            : mInstance(other.mInstance)
        {
            IncRef();
        }

        Ref& operator=(std::nullptr_t)
        {
            DecRef();
            mInstance = nullptr;
            return *this;
        }

        Ref& operator=(const Ref<T>& other)
        {
            other.IncRef();
            DecRef();

            mInstance = other.mInstance;
            return *this;
        }

        template<typename T2>
        Ref& operator=(const Ref<T2>& other)
        {
            other.IncRef();
            DecRef();

            mInstance = other.mInstance;
            return *this;
        }

        template<typename T2>
        Ref& operator=(Ref<T2>&& other)
        {
            DecRef();

            mInstance = other.mInstance;
            other.mInstance = nullptr;
            return *this;
        }

        operator bool() { return mInstance != nullptr; }
        operator bool() const { return mInstance != nullptr; }

        T* operator->() { return mInstance; }
        const T* operator->() const { return mInstance; }

        T& operator*() { return *mInstance; }
        const T& operator*() const { return *mInstance; }

        T* Raw() { return  mInstance; }
        const T* Raw() const { return  mInstance; }

        void Reset(T* instance = nullptr)
        {
            DecRef();
            mInstance = instance;
        }

        template<typename... Args>
        static Ref<T> Create(Args&&... args)
        {
            return Ref<T>(new T(std::forward<Args>(args)...));
        }
    private:
        void IncRef() const
        {
            if (mInstance)
                mInstance->IncRefCount();
        }

        void DecRef() const
        {
            if (mInstance)
            {
                mInstance->DecRefCount();
                if (mInstance->GetRefCount() == 0)
                    delete mInstance;
            }
        }

        template<class T2>
        friend class Ref;
        T* mInstance;
    };
    // TODO: WeakRef
}
