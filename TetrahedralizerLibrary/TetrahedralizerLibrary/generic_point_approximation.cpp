#include "generic_point_approximation.hpp"

void GenericPointApproximationHandle::generic_point_approximation()
{
    double x,y,z;
    for(uint32_t i=0; i<m_vertices.size(); i++)
    {
        m_vertices[i]->getApproxXYZCoordinates(x, y, z, true);
        m_approximated_positions.push_back(x);
        m_approximated_positions.push_back(y);
        m_approximated_positions.push_back(z);
    }
}

void GenericPointApproximationHandle::Dispose()
{}
void GenericPointApproximationHandle::AddInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values)
{
    m_vertices = create_vertices(explicit_count, explicit_values, implicit_count, implicit_values);
}


void GenericPointApproximationHandle::Calculate()
{
    generic_point_approximation();
}

double* GenericPointApproximationHandle::GetApproximation()
{
    return m_approximated_positions.data();
}

extern "C" LIBRARY_EXPORT void* CreateGenericPointApproximationHandle()
{
    return new GenericPointApproximationHandle();
}
extern "C" LIBRARY_EXPORT void DisposeGenericPointApproximationHandle(void* handle)
{
    ((GenericPointApproximationHandle*)handle)->Dispose();
    delete (GenericPointApproximationHandle*)handle;
}

extern "C" LIBRARY_EXPORT void AddGenericPointApproximationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values)
{
    ((GenericPointApproximationHandle*)handle)->AddInput(explicit_count, explicit_values, implicit_count, implicit_values);
}

extern "C" LIBRARY_EXPORT void CalculateGenericPointApproximation(void* handle)
{
    ((GenericPointApproximationHandle*)handle)->Calculate();
}

extern "C" LIBRARY_EXPORT double* GetGenericPointApproximationPositions(void* handle)
{
    return ((GenericPointApproximationHandle*)handle)->GetApproximation();
}
