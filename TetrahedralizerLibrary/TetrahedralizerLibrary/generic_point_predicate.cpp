#include "generic_point_predicate.hpp"

void GenericPointPredicateHandle::Dispose()
{}

void GenericPointPredicateHandle::AddInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values)
{
    m_vertices = create_vertices(explicit_count, explicit_values, implicit_count, implicit_values);
}

int GenericPointPredicateHandle::Orient3d(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3)
{
    return orient3d(p0, p1, p2, p3, m_vertices.data());
}
bool GenericPointPredicateHandle::IsCollinear(uint32_t p0,uint32_t p1,uint32_t p2)
{
    return is_collinear(p0, p1, p2, m_vertices.data());
}


extern "C" LIBRARY_EXPORT void* CreateGenericPointPredicateHandle()
{
    return new GenericPointPredicateHandle();
}
extern "C" LIBRARY_EXPORT void DisposeGenericPointPredicateHandle(void* handle)
{
    ((GenericPointPredicateHandle*)handle)->Dispose();
    delete (GenericPointPredicateHandle*)handle;
}

extern "C" LIBRARY_EXPORT void AddGenericPointPredicateVertices(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values)
{
    ((GenericPointPredicateHandle*)handle)->AddInput(explicit_count, explicit_values, implicit_count, implicit_values);
}

extern "C" LIBRARY_EXPORT int CalculateOrient3d(void* handle, uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3)
{
    return ((GenericPointPredicateHandle*)handle)->Orient3d(p0, p1, p2, p3);
}

extern "C" LIBRARY_EXPORT bool CalculateIsCollinear(void* handle, uint32_t p0,uint32_t p1,uint32_t p2)
{
    return ((GenericPointPredicateHandle*)handle)->IsCollinear(p0, p1, p2);
}
