using System.Runtime.InteropServices;

namespace Electro
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Transform
    {
        public Vector3 Position;
        public Vector3 Rotation;
        public Vector3 Scale;

        public Vector3 Up { get; }
        public Vector3 Right { get; }
        public Vector3 Forward { get; }

    }
}
