using UnityEngine;

public static class Texture2DExtension
{
    public static Texture2D DuplicateWithUninitializedPixels(this Texture2D texture2D)
    {
        return new Texture2D(texture2D.width, texture2D.height, texture2D.format, texture2D.mipmapCount > 1);
    }
}