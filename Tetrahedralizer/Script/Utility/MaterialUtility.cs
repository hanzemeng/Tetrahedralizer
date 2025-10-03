using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;

public static class MaterialExtension
{
    public static List<(Texture2D, List<string>)> GetTexture2Ds(this Material material)
    {
        List<(Texture2D, List<string>)> res = new List<(Texture2D, List<string>)>();
        Shader shader = material.shader;
        int propertyCount = shader.GetPropertyCount();
        for(int i=0; i<propertyCount; i++)
        {
            if(ShaderPropertyType.Texture != shader.GetPropertyType(i))
            {
                continue;
            }
            
            if(material.GetTexture(shader.GetPropertyName(i)) is Texture2D texture2D)
            {
                int index = res.FindIndex(i=>i.Item1==texture2D);
                if(index < 0)
                {
                    res.Add((texture2D, new List<string>{shader.GetPropertyName(i)}));
                }
                else
                {
                    res[index].Item2.Add(shader.GetPropertyName(i));
                }
            }
        }
        return res;
    }
}