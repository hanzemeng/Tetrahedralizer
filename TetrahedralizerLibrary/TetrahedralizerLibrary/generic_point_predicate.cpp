#include "generic_point_predicate.hpp"
using namespace std;

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
int GenericPointPredicateHandle::Orient3d(shared_ptr<genericPoint> p0,shared_ptr<genericPoint> p1,shared_ptr<genericPoint> p2,shared_ptr<genericPoint> p3)
{
    return orient3d(p0,p1,p2,p3);
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
extern "C" LIBRARY_EXPORT int CalculateOrient3dExplicit(void* handle, double p0x, double p0y, double p0z, double p1x, double p1y, double p1z, double p2x, double p2y, double p2z, double p3x, double p3y, double p3z)
{
    shared_ptr<genericPoint> p0 = make_shared<explicitPoint3D>(p0x,p0y,p0z);
    shared_ptr<genericPoint> p1 = make_shared<explicitPoint3D>(p1x,p1y,p1z);
    shared_ptr<genericPoint> p2 = make_shared<explicitPoint3D>(p2x,p2y,p2z);
    shared_ptr<genericPoint> p3 = make_shared<explicitPoint3D>(p3x,p3y,p3z);
    return ((GenericPointPredicateHandle*)handle)->Orient3d(p0, p1, p2, p3);
}

extern "C" LIBRARY_EXPORT bool CalculateIsCollinear(void* handle, uint32_t p0,uint32_t p1,uint32_t p2)
{
    return ((GenericPointPredicateHandle*)handle)->IsCollinear(p0, p1, p2);
}
