#ifndef texture_projection_hpp
#define texture_projection_hpp

#include "common_header.h"
#include "common_function.h"
#include "generic_point_approximation.hpp"
#include "mesh_raycaster.hpp"

class TextureProjectionInput
{
public:
    genericPoint** m_src_vertices;
    uint32_t m_src_vertices_count;
    float* m_src_uvs; // every vertex has 16 floats, 2 for each uv
    uint32_t* m_src_triangles; // oriented such that triangles point out using right hand rule
    uint32_t m_src_triangles_count; // number of constraints, same as m_src_triangles.size()/3
    
    genericPoint** m_des_vertices;
    uint32_t m_des_vertices_count;
    float* m_des_uvs; // every vertex has 2 floats, so assume des_uvs has 1 channel
    uint32_t* m_des_triangles; // oriented such that triangles point out using right hand rule
    uint32_t m_des_triangles_count; // number of constraints, same as m_des_triangles.size()/3
    
    uint32_t m_textures_count;
    uint32_t* m_textures_dimensions; // x,y for every texture. src and des have same dimensions
    uint32_t* m_textures_uv_channels; // which uv channel the ith texture should use
    double** m_textures_colors; // per texture, 4 double per pixel
};
class TextureProjectionOutput
{
public:
    double** m_textures_colors; // per texture, 4 double per pixel
};
class TextureProjection
{
public:
    void texture_projection(TextureProjectionInput* input, TextureProjectionOutput* output);
};

class TextureProjectionHandle
{
public:
    TextureProjectionInput* m_input;
    TextureProjectionOutput* m_output;
    TextureProjection* m_textureProjection;
    
    TextureProjectionHandle();
    void Dispose();

    // Every uv has 16 floats: <u0, v0>...<u7, v7>. Triangles are oriented such that they point out using right hand rule.
    void AddSrcMeshData(uint32_t explicit_count, double* explicit_values, float* uvs, uint32_t triangle_count, uint32_t* triangles);
    // Every uv has 2 floats: <u0, v0>. Triangles are oriented such that they point out using right hand rule.
    void AddDesMeshData(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, float* uvs, uint32_t triangle_count, uint32_t* triangles);
    // 4 byte per pixel
    void AddTextureData(uint32_t textures_count, uint32_t* textures_dimensions, uint32_t* textures_uv_channels, uint8_t** textures_colors);
    
    void CalculateTextureProjection();
    double** GetOutputTexturesColors();
};

extern "C" LIBRARY_EXPORT void* CreateTextureProjectionHandle();
extern "C" LIBRARY_EXPORT void DisposeTextureProjectionHandle(void* handle);

extern "C" LIBRARY_EXPORT void AddTextureProjectionSrcInput(void* handle, uint32_t explicit_count, double* explicit_values, float* uvs, uint32_t triangle_count, uint32_t* triangles);
extern "C" LIBRARY_EXPORT void AddTextureProjectionDesInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, float* uvs, uint32_t triangle_count, uint32_t* triangles);
extern "C" LIBRARY_EXPORT void AddTextureProjectionTextureInput(void* handle, uint32_t textures_count, uint32_t* textures_dimensions, uint32_t* textures_uv_channels, uint8_t** textures_colors);

extern "C" LIBRARY_EXPORT void CalculateTextureProjection(void* handle);

extern "C" LIBRARY_EXPORT double** GetOutputTexturesColors(void* handle);

#endif
