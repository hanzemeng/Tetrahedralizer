#include "texture_projection.hpp"


void TextureProjection::texture_projection(TextureProjectionInput* input, TextureProjectionOutput* output)
{
    // prepare ray caster
    MeshRaycaster mesh_raycaster;
    {
        GenericPointApproximation gpa;
        GenericPointApproximationInput gpa_input;
        gpa_input.m_vertices = input->m_src_vertices;
        gpa_input.m_vertices_count = input->m_src_vertices_count;
        GenericPointApproximationOutput gpa_output;
        gpa.generic_point_approximation(&gpa_input, &gpa_output);
        
        mesh_raycaster.AddMeshRaycasterInput(input->m_src_vertices_count, gpa_output.m_approximate_positions, input->m_src_triangles_count, input->m_src_triangles);
        delete[] gpa_output.m_approximate_positions;
    }
    
    // prepare color
    vector<vector<double4>> src_textures_colors;
    vector<vector<double4>> des_textures_colors;
    {
        for(uint32_t i=0; i<input->m_textures_count; i++)
        {
            uint32_t w = input->m_textures_dimensions[2*i+0];
            uint32_t h = input->m_textures_dimensions[2*i+1];
            src_textures_colors.push_back(vector<double4>(w*h));
            
            for(uint32_t j=0; j<w*h; j++)
            {
                src_textures_colors[i][j] = double4(input->m_textures_colors[i][4*j+0],input->m_textures_colors[i][4*j+1],input->m_textures_colors[i][4*j+2],input->m_textures_colors[i][4*j+3]);
            }
        }
        des_textures_colors = src_textures_colors;
    }
    
    // project texture
    for(uint32_t i=0; i<input->m_des_triangles_count; i++)
    {
        uint32_t t0 = input->m_des_triangles[3*i+0];
        uint32_t t1 = input->m_des_triangles[3*i+1];
        uint32_t t2 = input->m_des_triangles[3*i+2];
        
        double2 uv0(input->m_des_uvs[2*t0+0],input->m_des_uvs[2*t0+1]);
        double2 uv1(input->m_des_uvs[2*t1+0],input->m_des_uvs[2*t1+1]);
        double2 uv2(input->m_des_uvs[2*t2+0],input->m_des_uvs[2*t2+1]);
        double3 p0 = approximate_point(input->m_des_vertices[t0]);
        double3 p1 = approximate_point(input->m_des_vertices[t1]);
        double3 p2 = approximate_point(input->m_des_vertices[t2]);
        
        double3 n = (p1-p0).cross(p2-p1);
        n.normalize();
        double2 luv = uv0.min(uv1).min(uv2);
        double2 tuv = uv0.max(uv1).max(uv2);
        
        for(uint32_t j=0; j<input->m_textures_count; j++)
        {
            uint32_t width(input->m_textures_dimensions[2*j+0]), height(input->m_textures_dimensions[2*j+1]);
            uint32_t uv_channel = input->m_textures_uv_channels[j];
            uint32_t l_x = (uint32_t)((double)width * luv.x);
            uint32_t t_x = (uint32_t)((double)width * tuv.x);
            uint32_t l_y = (uint32_t)((double)height * luv.y);
            uint32_t t_y = (uint32_t)((double)height * tuv.y);
            
            for(uint32_t t_i=l_y; t_i<t_y; t_i++)
            {
                for(uint32_t t_j=l_x; t_j<t_x; t_j++)
                {
                    double2 uv((double)t_j/(double)width, (double)t_i/(double)height);
                    double3 w;
//                    if(!barycentric_weight(uv0,uv1,uv2,uv,w))
//                    {
//                        continue;
//                    }
                    barycentric_weight(uv0,uv1,uv2,uv,w);
                    double3 p = w.x*p0 + w.y*p1 + w.z*p2;
                    
                    uint32_t t_new;
                    double t;
                    if(!mesh_raycaster.Raycast(p, n, t_new, t, w))
                    {
                        continue;
                    }
                    
                    double2 src_uv0(input->m_src_uvs[16*input->m_src_triangles[3*t_new+0]+2*uv_channel + 0], input->m_src_uvs[16*input->m_src_triangles[3*t_new+0]+2*uv_channel + 1]);
                    double2 src_uv1(input->m_src_uvs[16*input->m_src_triangles[3*t_new+1]+2*uv_channel + 0], input->m_src_uvs[16*input->m_src_triangles[3*t_new+1]+2*uv_channel + 1]);
                    double2 src_uv2(input->m_src_uvs[16*input->m_src_triangles[3*t_new+2]+2*uv_channel + 0], input->m_src_uvs[16*input->m_src_triangles[3*t_new+2]+2*uv_channel + 1]);
                    double2 src_uv = w.x*src_uv0 + w.y*src_uv1 + w.z*src_uv2;
                    
                    uint32_t src_j = src_uv.x * width;
                    uint32_t src_i = src_uv.y * height;
                    des_textures_colors[j][t_i*width+t_j] = src_textures_colors[j][src_i*width+src_j];
                }
            }
        }
    }
    
    mesh_raycaster.Dispose();
    
    // copy to output
    {
        output->m_textures_colors = new double*[input->m_textures_count];
        for(uint32_t i=0; i<input->m_textures_count; i++)
        {
            output->m_textures_colors[i] = new double[4*des_textures_colors[i].size()];
            for(uint32_t j=0; j<des_textures_colors[i].size(); j++)
            {
                for(uint32_t k=0; k<4; k++)
                {
                    output->m_textures_colors[i][4*j+k] = des_textures_colors[i][j][k];
                }
            }
        }
    }
}


TextureProjectionHandle::TextureProjectionHandle()
{
    m_input = new TextureProjectionInput();
    m_input->m_src_vertices = nullptr;
    m_input->m_src_vertices_count = 0;
    m_input->m_src_uvs = nullptr;
    m_input->m_src_triangles = nullptr;
    m_input->m_des_vertices = nullptr;
    m_input->m_des_vertices_count = 0;
    m_input->m_des_uvs = nullptr;
    m_input->m_des_triangles = nullptr;
    m_input->m_textures_count = 0;
    m_input->m_textures_dimensions = nullptr;
    m_input->m_textures_uv_channels = nullptr;
    m_input->m_textures_colors = nullptr;
    
    m_output = new TextureProjectionOutput();
    m_output->m_textures_colors = nullptr;
    m_textureProjection = new TextureProjection();
}
void TextureProjectionHandle::Dispose()
{
    delete_vertices(m_input->m_src_vertices, m_input->m_src_vertices_count);
    delete_vertices(m_input->m_des_vertices, m_input->m_des_vertices_count);
    delete[] m_input->m_src_uvs;
    delete[] m_input->m_src_triangles;
    delete[] m_input->m_des_uvs;
    delete[] m_input->m_des_triangles;
    delete[] m_input->m_textures_dimensions;
    delete[] m_input->m_textures_uv_channels;
    for(uint32_t i=0; i<m_input->m_textures_count; i++)
    {
        delete[] m_input->m_textures_colors[i];
        delete[] m_output->m_textures_colors[i];
    }
    delete[] m_input->m_textures_colors;
    delete[] m_output->m_textures_colors;
    delete m_input;
    delete m_output;
    delete m_textureProjection;
}

void TextureProjectionHandle::AddSrcMeshData(uint32_t explicit_count, double* explicit_values, float* uvs, uint32_t triangle_count, uint32_t* triangles)
{
    create_vertices(explicit_count, explicit_values, 0, nullptr, m_input->m_src_vertices, m_input->m_src_vertices_count);
    m_input->m_src_uvs = duplicate_array(uvs, 16*m_input->m_src_vertices_count);
    m_input->m_src_triangles_count = triangle_count;
    m_input->m_src_triangles = duplicate_array(triangles, 3*triangle_count);
}
void TextureProjectionHandle::AddDesMeshData(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, float* uvs, uint32_t triangle_count, uint32_t* triangles)
{
    create_vertices(explicit_count, explicit_values, implicit_count, implicit_values, m_input->m_des_vertices, m_input->m_des_vertices_count);
    m_input->m_des_uvs = duplicate_array(uvs, 2*m_input->m_des_vertices_count);
    m_input->m_des_triangles_count = triangle_count;
    m_input->m_des_triangles = duplicate_array(triangles, 3*triangle_count);
}
void TextureProjectionHandle::AddTextureData(uint32_t textures_count, uint32_t* textures_dimensions, uint32_t* textures_uv_channels, uint8_t** textures_colors)
{
    m_input->m_textures_count = textures_count;
    m_input->m_textures_dimensions = duplicate_array(textures_dimensions, 2*textures_count);
    m_input->m_textures_uv_channels = duplicate_array(textures_uv_channels, textures_count);
    m_input->m_textures_colors = new double*[textures_count];
    for(uint32_t i=0; i<textures_count; i++)
    {
        uint32_t n = textures_dimensions[2*i+0]*textures_dimensions[2*i+1];
        m_input->m_textures_colors[i] = new double[4*n];
        for(uint32_t j=0; j<n; j++)
        {
            m_input->m_textures_colors[i][4*j+0] = (double)(textures_colors[i][3*j+0])/ 255.0;
            m_input->m_textures_colors[i][4*j+1] = (double)(textures_colors[i][3*j+1])/ 255.0;
            m_input->m_textures_colors[i][4*j+2] = (double)(textures_colors[i][3*j+2])/ 255.0;
            m_input->m_textures_colors[i][4*j+3] = 1.0;
        }
    }
}

void TextureProjectionHandle::CalculateTextureProjection()
{
    m_textureProjection->texture_projection(m_input, m_output);
}
double** TextureProjectionHandle::GetOutputTexturesColors()
{
    return m_output->m_textures_colors;
}

extern "C" LIBRARY_EXPORT void* CreateTextureProjectionHandle()
{
    return new TextureProjectionHandle();
}
extern "C" LIBRARY_EXPORT void DisposeTextureProjectionHandle(void* handle)
{
    ((TextureProjectionHandle*)handle)->Dispose();
    delete (TextureProjectionHandle*)handle;
}

extern "C" LIBRARY_EXPORT void AddTextureProjectionSrcInput(void* handle, uint32_t explicit_count, double* explicit_values, float* uvs, uint32_t triangle_count, uint32_t* triangles)
{
    ((TextureProjectionHandle*)handle)->AddSrcMeshData(explicit_count, explicit_values, uvs, triangle_count, triangles);
}
extern "C" LIBRARY_EXPORT void AddTextureProjectionDesInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, float* uvs, uint32_t triangle_count, uint32_t* triangles)
{
    ((TextureProjectionHandle*)handle)->AddDesMeshData(explicit_count, explicit_values, implicit_count, implicit_values, uvs, triangle_count, triangles);
}
extern "C" LIBRARY_EXPORT void AddTextureProjectionTextureInput(void* handle, uint32_t textures_count, uint32_t* textures_dimensions, uint32_t* textures_uv_channels, uint8_t** textures_colors)
{
    ((TextureProjectionHandle*)handle)->AddTextureData(textures_count, textures_dimensions, textures_uv_channels, textures_colors);
}

extern "C" LIBRARY_EXPORT void CalculateTextureProjection(void* handle)
{
    ((TextureProjectionHandle*)handle)->CalculateTextureProjection();
}

extern "C" LIBRARY_EXPORT double** GetOutputTexturesColors(void* handle)
{
    return ((TextureProjectionHandle*)handle)->m_output->m_textures_colors;
}
