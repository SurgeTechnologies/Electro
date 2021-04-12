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

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool Raycast_Native(out RaycastHit hit, ref Vector3 origin, ref Vector3 direction, float maxDistance);
    }
}