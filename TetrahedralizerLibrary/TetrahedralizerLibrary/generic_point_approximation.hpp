#ifndef generic_point_approximation_hpp
#define generic_point_approximation_hpp

#include "common_header.h"
#include "common_function.h"

class GenericPointApproximationInput
{
public:
    genericPoint** m_vertices;
    uint32_t m_vertices_count;
};
class GenericPointApproximationOutput
{
public:
    double* m_approximate_positions;
};

class GenericPointApproximation
{
public:
    void generic_point_approximation(GenericPointApproximationInput*, GenericPointApproximationOutput*);
};

class GenericPointApproximationHandle
{
public:
    GenericPointApproximationInput* m_input;
    GenericPointApproximationOutput* m_output;
    GenericPointApproximation* m_genericPointApproximation;
    
    GenericPointApproximationHandle();
    void Dispose();
    
    // number of vertices, then x,y,z of every vertex
    // number of vertices, then 5/9 followed by indices of explicit vertices
    void AddInputVertices(uint32_t, double*, uint32_t, uint32_t*);
    
    void ApproximateGenericPoint();
    
    double* GetOutputApproximation();
};

extern "C" LIBRARY_EXPORT void* CreateGenericPointApproximationHandle();
extern "C" LIBRARY_EXPORT void DisposeGenericPointApproximationHandle(void* handle);

extern "C" LIBRARY_EXPORT void AddGenericPointApproximationVertices(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values);

extern "C" LIBRARY_EXPORT void ApproximateGenericPoint(void* handle);

extern "C" LIBRARY_EXPORT double* GetOutputApproximation(void* handle);

#endif
