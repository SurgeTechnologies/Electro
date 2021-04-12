using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Electro
{
    [StructLayout(LayoutKind.Sequential)]
    public struct RaycastHit
    {
        public float Distance { get; private set; }
        public ulong EntityUUID { get; private set; }
        public Vector3 Position { get; private set; }
        public Vector3 Normal { get; private set; }
    }

    public static class Physics
    {
        public static bool Raycast(out RaycastHit hit, Vector3 origin, Vector3 direction, float maxDistance)
        {
            return Raycast_Native(out hit, ref origin, ref direction, maxDistance);
        }
        public static void SetFixedTimestep(float fixedTimestep)
        {
            SetFixedTimestep_Native(fixedTimestep);
        }
        public static float GetFixedTimestep()
        {
            return GetFixedTimestep_Native();
        }
        public static void SetGravity(Vector3 inGravity)
        {
            SetGravity_Native(ref inGravity);
        }
        public static Vector3 GetGravity()
        {
            GetGravity_Native(out Vector3 gravity);
            return gravity;
        }
        public static void SetSolverIterations(uint solverIterations)
        {
            SetSolverIterations_Native(solverIterations);
        }
        public static uint GetSolverIterations()
        {
            return GetSolverIterations_Native();
        }
        public static void SetSolverVelocityIterations(uint solverVelocityIterations)
        {
            SetSolverVelocityIterations_Native(solverVelocityIterations);
        }
        public static uint GetSolverVelocityIterations()
        {
            return GetSolverVelocityIterations_Native();
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Raycast_Native(out RaycastHit hit, ref Vector3 origin, ref Vector3 direction, float maxDistance);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetFixedTimestep_Native(float fixedTimestep);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern float GetFixedTimestep_Native();
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetGravity_Native(ref Vector3 inGravity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetGravity_Native(out Vector3 outGravity);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetSolverIterations_Native(uint solverIterations);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern uint GetSolverIterations_Native();
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetSolverVelocityIterations_Native(uint solverVelocityIterations);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern uint GetSolverVelocityIterations_Native();
    }
}