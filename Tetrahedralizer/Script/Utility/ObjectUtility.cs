using UnityEngine;

public static class ObjectExtension
{
    public static string GetGUID(this Object obj)
    {
        #if UNITY_EDITOR
        string path = UnityEditor.AssetDatabase.GetAssetPath(obj);
        if(string.IsNullOrEmpty(path))
        {
            return string.Empty;
        }
        return UnityEditor.AssetDatabase.AssetPathToGUID(path);
        #else
        return string.Empty;
        #endif
    }
}
