using System;

namespace Electro
{
    public static class Mathf
    {
        public const float Rad2Deg = 360.0f / ((float)Math.PI * 2.0f);
        public const float Deg2Rad = ((float)Math.PI * 2.0f) / 360.0f;

        public static float Clamp(float value, float min, float max)
        {
            if (value < min)
                return min;
            if (value > max)
                return max;

            return value;
        }
    }
}
