using System.Runtime.CompilerServices;

namespace Electro
{
    public class Console
    {
        public static void LogInfo(object message)
        {
            LogInfo_Native(message);
        }

        public static void LogWarn(object message)
        {
            LogWarn_Native(message);
        }

        public static void LogDebug(object message)
        {
            LogDebug_Native(message);
        }

        public static void LogError(object message)
        {
            LogError_Native(message);
        }

        public static void LogFatal(object message)
        {
            LogFatal_Native(message);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void LogInfo_Native(object message);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void LogWarn_Native(object message);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void LogDebug_Native(object message);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void LogError_Native(object message);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void LogFatal_Native(object message);
    }
}
