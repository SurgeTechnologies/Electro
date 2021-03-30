using System;
using System.Runtime.InteropServices;

namespace Electro
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector3
    {
        public static Vector3 Zero = new Vector3(0, 0, 0);

        public static Vector3 Forward = new Vector3(0, 0, -1);
        public static Vector3 Right = new Vector3(1, 0, 0);
        public static Vector3 Up = new Vector3(0, 1, 0);

        public float X;
        public float Y;
        public float Z;

        public Vector3(float scalar)
        {
            X = Y = Z = scalar;
        }

        public Vector3(float x, float y, float z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        public Vector3(float x, Vector2 yz)
        {
            X = x;
            Y = yz.X;
            Z = yz.Y;
        }

        public Vector3(Vector2 vector)
        {
            X = vector.X;
            Y = vector.Y;
            Z = 0.0f;
        }

        public Vector3(Vector4 vector)
        {
            X = vector.X;
            Y = vector.Y;
            Z = vector.Z;
        }

        public void Clamp(Vector3 min, Vector3 max)
        {
            X = Mathf.Clamp(X, min.X, max.X);
            Y = Mathf.Clamp(Y, min.Y, max.Y);
            Z = Mathf.Clamp(Z, min.Z, max.Z);
        }

        public float Length()
        {
            return (float)Math.Sqrt(X * X + Y * Y + Z * Z);
        }

        public Vector3 Normalized()
        {
            float length = Length();
            float x = X / length;
            float y = Y / length;
            float z = Z / length;
            return new Vector3(x, y, z);
        }

        public void Normalize()
        {
            float length = Length();
            X = X / length;
            Y = Y / length;
            Z = Z / length;
        }

        public static Vector3 operator *(Vector3 left, float scalar)
        {
            return new Vector3(left.X * scalar, left.Y * scalar, left.Z * scalar);
        }

        public static Vector3 operator *(float scalar, Vector3 right)
        {
            return new Vector3(scalar * right.X, scalar * right.Y, scalar * right.Z);
        }

        public static Vector3 operator +(Vector3 left, Vector3 right)
        {
            return new Vector3(left.X + right.X, left.Y + right.Y, left.Z + right.Z);
        }

        public static Vector3 operator +(Vector3 left, float right)
        {
            return new Vector3(left.X + right, left.Y + right, left.Z + right);
        }

        public static Vector3 operator -(Vector3 left, Vector3 right)
        {
            return new Vector3(left.X - right.X, left.Y - right.Y, left.Z - right.Z);
        }

        public static Vector3 operator /(Vector3 left, Vector3 right)
        {
            return new Vector3(left.X / right.X, left.Y / right.Y, left.Z / right.Z);
        }

        public static Vector3 operator /(Vector3 left, float scalar)
        {
            return new Vector3(left.X / scalar, left.Y / scalar, left.Z / scalar);
        }

        public static Vector3 operator -(Vector3 vector)
        {
            return new Vector3(-vector.X, -vector.Y, -vector.Z);
        }

        public static Vector3 Cos(Vector3 vector)
        {
            return new Vector3((float)Math.Cos(vector.X), (float)Math.Cos(vector.Y), (float)Math.Cos(vector.Z));
        }

        public static Vector3 Sin(Vector3 vector)
        {
            return new Vector3((float)Math.Sin(vector.X), (float)Math.Sin(vector.Y), (float)Math.Sin(vector.Z));
        }

        public override string ToString()
        {
            return "Vector3[" + X + ", " + Y + ", " + Z + "]";
        }

        public Vector2 XY
        {
            get { return new Vector2(X, Y); }
            set { X = value.X; Y = value.Y; }
        }
        public Vector2 XZ
        {
            get { return new Vector2(X, Z); }
            set { X = value.X; Z = value.Y; }
        }
        public Vector2 YZ
        {
            get { return new Vector2(Y, Z); }
            set { Y = value.X; Z = value.Y; }
        }

    }
}
