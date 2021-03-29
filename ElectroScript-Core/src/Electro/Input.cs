using System.Runtime.CompilerServices;

namespace Electro
{
    public enum CursorMode
    {
        Normal = 0,
        Hidden = 1,
        Locked = 2
    }

    public class Input
    {
        public static bool IsKeyPressed(KeyCode keycode)
        {
            return IsKeyPressed_Native(keycode);
        }

        public static bool IsMouseButtonPressed(MouseCode button)
        {
            return IsMouseButtonPressed_Native(button);
        }

        public static Vector2 GetMousePosition()
        {
            GetMousePosition_Native(out Vector2 position);
            return position;
        }

        public static void SetCursorMode(CursorMode mode) 
        {
            SetCursorMode_Native(mode);
        }
        public static CursorMode GetCursorMode()
        {
            return GetCursorMode_Native();
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsKeyPressed_Native(KeyCode key);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern bool IsMouseButtonPressed_Native(MouseCode button);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetMousePosition_Native(out Vector2 position);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetCursorMode_Native(CursorMode mode);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern CursorMode GetCursorMode_Native();

    }
}
