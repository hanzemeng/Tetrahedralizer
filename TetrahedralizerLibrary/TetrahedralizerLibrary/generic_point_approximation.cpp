#include "generic_point_approximation.hpp"

void GenericPointApproximation::generic_point_approximation(GenericPointApproximationInput* input, GenericPointApproximationOutput* output)
{
    output->m_approximate_positions = new double[3*input->m_vertices_count];
    double x,y,z;
    for(uint32_t i=0; i<input->m_vertices_count; i++)
    {
        input->m_vertices[i]->getApproxXYZCoordinates(x, y, z, true);
        output->m_approximate_positions[3*i+0] = x;
        output->m_approximate_positions[3*i+1] = y;
        output->m_approximate_positions[3*i+2] = z;
    }
}

GenericPointApproximationHandle::GenericPointApproximationHandle()
{
    m_input = new GenericPointApproximationInput();
    m_input->m_vertices = nullptr;
    m_input->m_vertices_count = 0;
    m_output = new GenericPointApproximationOutput();
    m_output->m_approximate_positions = nullptr;
    m_genericPointApproximation = new GenericPointApproximation();
}

void GenericPointApproximationHandle::Dispose()
{
    delete_vertices(m_input->m_vertices, m_input->m_vertices_count);
    delete m_input;
    delete[] m_output->m_approximate_positions;
    delete m_output;
    delete m_genericPointApproximation;
}

void GenericPointApproximationHandle::AddInputVertices(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values)
{
    create_vertices(explicit_count, explicit_values, implicit_count, implicit_values, m_input->m_vertices, m_input->m_vertices_count);
}


void GenericPointApproximationHandle::ApproximateGenericPoint()
{
    m_genericPointApproximation->generic_point_approximation(m_input, m_output);
}

double* GenericPointApproximationHandle::GetOutputApproximation()
{
    return m_output->m_approximate_positions;
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

extern "C" LIBRARY_EXPORT void AddGenericPointApproximationVertices(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values)
{
    ((GenericPointApproximationHandle*)handle)->AddInputVertices(explicit_count, explicit_values, implicit_count, implicit_values);
}

extern "C" LIBRARY_EXPORT void ApproximateGenericPoint(void* handle)
{
    ((GenericPointApproximationHandle*)handle)->ApproximateGenericPoint();
}

extern "C" LIBRARY_EXPORT double* GetOutputApproximation(void* handle)
{
    return ((GenericPointApproximationHandle*)handle)->GetOutputApproximation();
}
