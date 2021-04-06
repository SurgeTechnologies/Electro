using System;
using System.Runtime.CompilerServices;

namespace Electro
{
    public abstract class Component
    {
        public Entity Entity { get; set; }
    }
    public class TagComponent : Component
    {
        public string Tag
        {
            get => GetTag_Native(Entity.ID);
            set => SetTag_Native(Entity.ID, value);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern string GetTag_Native(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetTag_Native(ulong entityID, string tag);

    }

    public class TransformComponent : Component
    {
        public Transform Transform
        {
            get
            {
                GetTransform_Native(Entity.ID, out Transform result);
                return result;
            }

            set
            {
                SetTransform_Native(Entity.ID, ref value);
            }
        }

        public Vector3 Translation
        {
            get
            {
                GetTranslation_Native(Entity.ID, out Vector3 result);
                return result;
            }

            set
            {
                SetTranslation_Native(Entity.ID, ref value);
            }
        }

        public Vector3 Rotation
        {
            get
            {
                GetRotation_Native(Entity.ID, out Vector3 result);
                return result;
            }

            set
            {
                SetRotation_Native(Entity.ID, ref value);
            }
        }

        public Vector3 Scale
        {
            get
            {
                GetScale_Native(Entity.ID, out Vector3 result);
                return result;
            }

            set
            {
                SetScale_Native(Entity.ID, ref value);
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetTransform_Native(ulong entityID, out Transform outTransform);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetTransform_Native(ulong entityID, ref Transform inTransform);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetTranslation_Native(ulong entityID, out Vector3 outTranslation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetTranslation_Native(ulong entityID, ref Vector3 inTranslation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetRotation_Native(ulong entityID, out Vector3 outRotation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetRotation_Native(ulong entityID, ref Vector3 inRotation);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetScale_Native(ulong entityID, out Vector3 outScale);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetScale_Native(ulong entityID, ref Vector3 inScale);
    }

    public enum CameraType
    {

    }

    //TODO: Make this work fully, extend the API so that it can control every thing like a camara component has
    public class CameraComponent : Component
    {
        public bool IsPrimary()
        {
            return IsPrimary_Native(Entity.ID);
        }

        public void SetAsPrimary(bool isPrimary)
        {
            SetAsPrimary_Native(Entity.ID, isPrimary);
        }

        public bool IsFixedAspectRatio()
        {
            return IsFixedAspectRatio_Native(Entity.ID);
        }

        public void SetFixedAspectRatio(bool isFixedAspectRatio)
        {
            SetFixedAspectRatio_Native(Entity.ID, isFixedAspectRatio);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool IsPrimary_Native(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetAsPrimary_Native(ulong entityID, bool isPrimary);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetFixedAspectRatio_Native(ulong entityID, bool isFixedAspectRatio);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool IsFixedAspectRatio_Native(ulong entityID);

    }

    public class SpriteRendererComponent : Component
    {
        public Vector4 GetColor()
        {
            GetColor_Native(Entity.ID, out Vector4 color);
            return color;
        }
        public void SetColor(Vector4 color)
        {
            SetColor_Native(Entity.ID, ref color);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetColor_Native(ulong entityID, out Vector4 color);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetColor_Native(ulong entityID, ref Vector4 color);
    }

    public enum ForceMode
    {
        Force = 0,
        Impulse,
        VelocityChange,
        Acceleration
    }

    public class RigidBodyComponent : Component
    {
        public enum Type
        {
            Static,
            Dynamic
        }

        public Type BodyType
        {
            get
            {
                return GetBodyType_Native(Entity.ID);
            }
        }

        public float Mass
        {
            get { return GetMass_Native(Entity.ID); }
            set { SetMass_Native(Entity.ID, value); }
        }

        public void AddForce(Vector3 force, ForceMode forceMode = ForceMode.Force)
        {
            AddForce_Native(Entity.ID, ref force, forceMode);
        }

        public void AddTorque(Vector3 torque, ForceMode forceMode = ForceMode.Force)
        {
            AddTorque_Native(Entity.ID, ref torque, forceMode);
        }

        public Vector3 GetLinearVelocity()
        {
            GetLinearVelocity_Native(Entity.ID, out Vector3 velocity);
            return velocity;
        }

        public void SetLinearVelocity(Vector3 velocity)
        {
            SetLinearVelocity_Native(Entity.ID, ref velocity);
        }

        public Vector3 GetAngularVelocity()
        {
            GetAngularVelocity_Native(Entity.ID, out Vector3 velocity);
            return velocity;
        }

        public void SetAngularVelocity(Vector3 velocity)
        {
            SetAngularVelocity_Native(Entity.ID, ref velocity);
        }

        public void Rotate(Vector3 rotation)
        {
            Rotate_Native(Entity.ID, ref rotation);
        }

        public void UseGravity(bool use)
        {
            UseGravity_Native(Entity.ID, use);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void AddForce_Native(ulong entityID, ref Vector3 force, ForceMode forceMode);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void AddTorque_Native(ulong entityID, ref Vector3 torque, ForceMode forceMode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetLinearVelocity_Native(ulong entityID, out Vector3 velocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetLinearVelocity_Native(ulong entityID, ref Vector3 velocity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetAngularVelocity_Native(ulong entityID, out Vector3 velocity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetAngularVelocity_Native(ulong entityID, ref Vector3 velocity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Rotate_Native(ulong entityID, ref Vector3 rotation);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float GetMass_Native(ulong entityID);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float SetMass_Native(ulong entityID, float mass);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern Type GetBodyType_Native(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void UseGravity_Native(ulong entityID, bool use);
    }
}
