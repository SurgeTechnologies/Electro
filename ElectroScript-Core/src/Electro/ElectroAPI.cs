using System;
using System.Runtime.CompilerServices;

namespace Electro
{
    public class Entity
    {
        private Action<ulong> mCollisionBeginCallbacks;
        private Action<ulong> mCollisionEndCallbacks;
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

        public Entity FindEntityByTag(ulong entityID)
        {
            if(EntityExists_Native(entityID))
                return new Entity(entityID);
            Console.LogWarn("Entity with the given ID - " + entityID + " doesn't exixt!");
            return null;
        }

        public bool EntityExists(ulong entityID)
        {
            return EntityExists_Native(entityID);
        }

        public Vector3 Translation
        {
            get
            {
                return GetComponent<TransformComponent>().Translation;
            }
            set
            {
                GetComponent<TransformComponent>().Translation = value;
            }
        }

        public Vector3 Rotation
        {
            get
            {
                return GetComponent<TransformComponent>().Rotation;
            }
            set
            {
                GetComponent<TransformComponent>().Rotation = value;
            }
        }

        public Vector3 Scale
        {
            get
            {
                return GetComponent<TransformComponent>().Scale;
            }
            set
            {
                GetComponent<TransformComponent>().Scale = value;
            }
        }

        public void AddCollisionBeginCallback(Action<ulong> callback)
        {
            mCollisionBeginCallbacks += callback;
        }

        public void AddCollisionEndCallback(Action<ulong> callback)
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

        private void OnCollisionBegin(ulong data)
        {
            if (mCollisionBeginCallbacks != null)
                mCollisionBeginCallbacks.Invoke(data);
        }

        private void OnCollisionEnd(ulong data)
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
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool EntityExists_Native(ulong entityID);
    }
}
