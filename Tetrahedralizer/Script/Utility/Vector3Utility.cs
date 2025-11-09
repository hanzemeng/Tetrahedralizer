using UnityEngine;

public static class Vector3Extension
{
    public static bool LessThan(this ref Vector3 self, Vector3 other)
    {
        if(self.x < other.x)
        {
            return true;
        }
        if(self.x > other.x)
        {
            return false;
        }
        if(self.y < other.y)
        {
            return true;
        }
        if(self.y > other.y)
        {
            return false;
        }
        if(self.z < other.z)
        {
            return true;
        }
        if(self.z > other.z)
        {
            return false;
        }
        return false;
    }
    public static bool GreaterThan(this ref Vector3 self, Vector3 other)
    {
        if(self.x > other.x)
        {
            return true;
        }
        if(self.x < other.x)
        {
            return false;
        }
        if(self.y > other.y)
        {
            return true;
        }
        if(self.y < other.y)
        {
            return false;
        }
        if(self.z > other.z)
        {
            return true;
        }
        if(self.z < other.z)
        {
            return false;
        }
        return false;
    }
}
