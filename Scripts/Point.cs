using System;
using UnityEngine;


public struct Point2D
{
    public double x;
    public double y;

    public Point2D(double x,double y)
    {
        this.x = x;
        this.y = y;
    }
}

public struct Point3D
{
    public double x;
    public double y;
    public double z;

    public Point3D(double x,double y,double z)
    {
        this.x = x;
        this.y = y;
        this.z = z;
    }
    public Point3D(Vector3 point)
    {
        this.x = point.x;
        this.z = point.y;
        this.y = point.z;
    }

    public double GetAxis(int n)
    {
        switch (n)
        {
            case 0: return x;
            case 1: return y;
            case 2: return z;
            default: throw new Exception();
        }
    }
    public Point3D ToPoint3D()
    {
        return this;
    }

    public Vector3 ToVector3()
    {
        return new Vector3((float)x, (float)y, (float)z);
    }

    public void Normalize()
    {
        double magnitude = Point3D.Magnitude(this);
        x /= magnitude;
        y /= magnitude;
        z /= magnitude;
    }

    public override string ToString()
    {
        return $"{x.ToString("F4")}, {y.ToString("F4")}, {z.ToString("F4")}";
    }

    public static Point3D operator+(Point3D a, Point3D b)
    {
        return new Point3D(a.x+b.x, a.y+b.y, a.z+b.z);
    }
    public static Point3D operator-(Point3D a, Point3D b)
    {
        return new Point3D(a.x-b.x, a.y-b.y, a.z-b.z);
    }
    public static Point3D operator*(Point3D a, Point3D b)
    {
        return new Point3D(a.x*b.x, a.y*b.y, a.z*b.z);
    }
    public static Point3D operator*(Point3D a, double b)
    {
        return new Point3D(a.x*b, a.y*b, a.z*b);
    }
    public static Point3D operator/(Point3D a, double b)
    {
        return new Point3D(a.x/b, a.y/b, a.z/b);
    }

    public static double Dot(Point3D a, Point3D b)
    {
        return a.x*b.x + a.y*b.y + a.z*b.z;
    }
    public static Point3D Cross(Point3D a, Point3D b)
    {
        return new Point3D(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
    }
    public static double SquareMagnitude(Point3D a)
    {
        return a.x*a.x + a.y*a.y + a.z*a.z;
    }
    public static double Magnitude(Point3D a)
    {
        return Math.Sqrt(a.x*a.x + a.y*a.y + a.z*a.z);
    }
}
