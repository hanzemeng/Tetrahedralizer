#ifndef generic_point_approximation_hpp
#define generic_point_approximation_hpp

#include "common_header.h"
#include "common_function.h"

class GenericPointApproximationHandle
{
public:
    void Dispose();
    void AddInput(uint32_t, double*, uint32_t, uint32_t*);
    
    void Calculate();
    
    double* GetApproximation();
    
private:
    void generic_point_approximation();
    std::vector<std::shared_ptr<genericPoint>> m_vertices;
    std::vector<double> m_approximated_positions;
};

extern "C" LIBRARY_EXPORT void* CreateGenericPointApproximationHandle();
extern "C" LIBRARY_EXPORT void DisposeGenericPointApproximationHandle(void* handle);

extern "C" LIBRARY_EXPORT void AddGenericPointApproximationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values);

extern "C" LIBRARY_EXPORT void CalculateGenericPointApproximation(void* handle);

extern "C" LIBRARY_EXPORT double* GetGenericPointApproximationPositions(void* handle);

#endif
