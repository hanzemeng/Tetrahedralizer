#ifndef generic_point_predicate_hpp
#define generic_point_predicate_hpp

#include "common_header.h"
#include "common_function.h"


class GenericPointPredicateHandle
{
public:
    void AddInput(uint32_t, double*, uint32_t, uint32_t*);
    void Dispose();
    
    
    int Orient3d(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3);
    int Orient3d(std::shared_ptr<genericPoint> p0,std::shared_ptr<genericPoint> p1,std::shared_ptr<genericPoint> p2,std::shared_ptr<genericPoint> p3);
    bool IsCollinear(uint32_t p0,uint32_t p1,uint32_t p2);

private:
    std::vector<std::shared_ptr<genericPoint>> m_vertices;
};

extern "C" LIBRARY_EXPORT void* CreateGenericPointPredicateHandle();
extern "C" LIBRARY_EXPORT void DisposeGenericPointPredicateHandle(void* handle);

extern "C" LIBRARY_EXPORT void AddGenericPointPredicateVertices(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values);

extern "C" LIBRARY_EXPORT int CalculateOrient3d(void* handle, uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3);
extern "C" LIBRARY_EXPORT int CalculateOrient3dExplicit(void* handle, double p0x, double p0y, double p0z, double p1x, double p1y, double p1z, double p2x, double p2y, double p2z, double p3x, double p3y, double p3z);
extern "C" LIBRARY_EXPORT bool CalculateIsCollinear(void* handle, uint32_t p0,uint32_t p1,uint32_t p2);

#endif
