using System;
using System.Linq;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using UnityEngine;

public class TextureProjection
{
    public unsafe class TextureProjectionInput
    {
        public IList<double> m_srcExplicitVertices; // Every 3 doubles are x,y,z of a point. Assuming left hand coordinate.
        public IList<float> m_srcUVs; // Every vertex has 16 floats, 2 for each uv.
        public IList<int> m_srcTriangles; // Triangles point out using left hand rule.
        
        public IList<double> m_desExplicitVertices; // Every 3 doubles are x,y,z of a point. Assuming left hand coordinate.
        public IList<int> m_desImplicitVertices; // 5/9 followed by indexes of m_explicitVertices
        public IList<float> m_desUVs; // Every vertex has 2 floats
        public IList<int> m_desTriangles; // Triangles point out using left hand rule.
        
        public IList<int> m_texturesDimensions; // x,y for every texture. src and des have same dimensions
        public IList<int> m_texturesUVChannels; // which uv channel the ith texture should use
        public IList<bool> m_texturesHaveAlphaChannel; // true if the texture has alpha channel

        public IList<NativeArray<byte>> m_srcTexturesColors; // per texture, 4 bytes (0-255) per pixel
        public IList<NativeArray<byte>> m_desTexturesColors; // will be overwritten
    }


    public void CalculateTextureProjection(TextureProjectionInput input)
    {
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern IntPtr CreateTextureProjectionHandle();
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern void DisposeTextureProjectionHandle(IntPtr handle);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern void AddTextureProjectionSrcInput(IntPtr handle, int explicit_count, double[] explicit_values, float[] uvs, int triangle_count, int[] triangles);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern void AddTextureProjectionDesInput(IntPtr handle, int explicit_count, double[] explicit_values, int implicit_count, int[] implicit_values, float[] uvs, int triangle_count, int[] triangles);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        unsafe static extern void AddTextureProjectionTextureInput(IntPtr handle, int textures_count, int[] textures_dimensions, int[] textures_uv_channels, byte** textures_colors);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        static extern void CalculateTextureProjection(IntPtr handle);
        [DllImport(TetrahedralizerConstant.TETRAHEDRALIZER_LIBRARY_NAME)]
        unsafe static extern double** GetOutputTexturesColors(IntPtr handle);

        IntPtr handle = CreateTextureProjectionHandle();
        {
            double[] srcExplicitVertices = input.m_srcExplicitVertices.ToArray();
            int[] srcTriangles = input.m_srcTriangles.ToArray();
            TetrahedralizerUtility.SwapElementsByInterval(srcExplicitVertices, 3);
            TetrahedralizerUtility.SwapElementsByInterval(srcTriangles, 3);
            AddTextureProjectionSrcInput(handle, srcExplicitVertices.Length/3, srcExplicitVertices, input.m_srcUVs.ToArray(), srcTriangles.Length/3, srcTriangles);
        }
        {
            double[] desExplicitVertices = input.m_desExplicitVertices.ToArray();
            int[] desTriangles = input.m_desTriangles.ToArray();
            TetrahedralizerUtility.SwapElementsByInterval(desExplicitVertices, 3);
            TetrahedralizerUtility.SwapElementsByInterval(desTriangles, 3);
            AddTextureProjectionDesInput(handle, desExplicitVertices.Length/3, desExplicitVertices, 0, null, input.m_desUVs.ToArray(), desTriangles.Length/3, desTriangles);
        }
        unsafe
        {
            byte** colorBytes = (byte**)Marshal.AllocHGlobal(input.m_texturesUVChannels.Count * sizeof(byte*));
            for(int i=0; i<input.m_texturesUVChannels.Count; i++)
            {
                colorBytes[i] = (byte*)NativeArrayUnsafeUtility.GetUnsafeReadOnlyPtr(input.m_srcTexturesColors[i]);
            }
            AddTextureProjectionTextureInput(handle, input.m_texturesUVChannels.Count, input.m_texturesDimensions.ToArray(), input.m_texturesUVChannels.ToArray(), colorBytes);
            Marshal.FreeHGlobal((IntPtr)colorBytes);

            CalculateTextureProjection(handle);

            double** ptr = GetOutputTexturesColors(handle);
            for(int i=0; i<input.m_texturesUVChannels.Count; i++)
            {
                NativeArray<byte> desTextures = input.m_desTexturesColors[i];
                for(int j=0; j<input.m_texturesDimensions[2*i+0]*input.m_texturesDimensions[2*i+1]; j++)
                {
                    desTextures[3*j+0] = (byte)Math.Round(ptr[i][4*j+0]*255d);
                    desTextures[3*j+1] = (byte)Math.Round(ptr[i][4*j+1]*255d);
                    desTextures[3*j+2] = (byte)Math.Round(ptr[i][4*j+2]*255d);
                }
            }
        }

        DisposeTextureProjectionHandle(handle);
    }
}
