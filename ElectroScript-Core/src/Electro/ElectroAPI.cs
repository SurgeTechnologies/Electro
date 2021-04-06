using System;
using System.Runtime.CompilerServices;

namespace Electro
{
    public class Entity
    {
        private Action<float> mCollisionBeginCallbacks;
        private Action<float> mCollisionEndCallbacks;
        private Action<float> mTriggerBeginCallbacks;
        private Action<float> mTriggerEndCallbacks;

        public ulong ID { get; private set; }

        protected Entity() { ID = 0; }

        internal Entity(ulong id)
        {
            ID = id;
        }

        ~Entity() {}

        public T CreateComponent<T>() where T : Component, new()
        {
            CreateComponent_Native(ID, typeof(T));
            T component = new T();
            component.Entity = this;
            return component;
        }

        public bool HasComponent<T>() where T : Component, new()
        {
            return HasComponent_Native(ID, typeof(T));
        }

        public T GetComponent<T>() where T : Component, new()
        {
            if (HasComponent<T>())
            {
                T component = new T();
                component.Entity = this;
                return component;
            }
            return null;
        }

        public Entity FindEntityByTag(string tag)
        {
            ulong entityID = FindEntityByTag_Native(tag);
            return new Entity(entityID);
        }

        public void AddCollisionBeginCallback(Action<float> callback)
        {
            mCollisionBeginCallbacks += callback;
        }

        public void AddCollisionEndCallback(Action<float> callback)
        {
            mCollisionEndCallbacks += callback;
        }

        public void AddTriggerBeginCallback(Action<float> callback)
        {
            mTriggerBeginCallbacks += callback;
        }

        public void AddTriggerEndCallback(Action<float> callback)
        {
            mTriggerEndCallbacks += callback;
        }

        private void OnCollisionBegin(float data)
        {
            if (mCollisionBeginCallbacks != null)
                mCollisionBeginCallbacks.Invoke(data);
        }

        private void OnCollisionEnd(float data)
        {
            if (mCollisionEndCallbacks != null)
                mCollisionEndCallbacks.Invoke(data);
        }

        private void OnTriggerBegin(float data)
        {
            if (mTriggerBeginCallbacks != null)
                mTriggerBeginCallbacks.Invoke(data);
        }

        private void OnTriggerEnd(float data)
        {
            if (mTriggerEndCallbacks != null)
                mTriggerEndCallbacks.Invoke(data);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void CreateComponent_Native(ulong entityID, Type type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool HasComponent_Native(ulong entityID, Type type);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern ulong FindEntityByTag_Native(string tag);

    }
}
