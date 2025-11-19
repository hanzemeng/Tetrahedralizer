#include "common_header.h"
#include "common_function.h"
#include "delaunay_tetrahedralization.hpp"
#include "binary_space_partition.hpp"
#include "interior_characterization.hpp"
#include "polyhedralization_tetrahedralization.hpp"

int main(int argc, const char * argv[])
{
    double* explicit_values = new double[24];
    uint32_t* tetrahedrons = new uint32_t[24];
    uint32_t* constraints = new uint32_t[36];
    
    explicit_values[0] = 0.08001075685024261;
    explicit_values[1] = -0.05893407762050629;
    explicit_values[2] = 0;
    explicit_values[3] = -0.08001075685024261;
    explicit_values[4] = -0.05893407762050629;
    explicit_values[5] = 0;
    explicit_values[6] =0.08001075685024261;
    explicit_values[7] =0.05893407762050629;
    explicit_values[8] =0;
    explicit_values[9] =-0.08001075685024261;
    explicit_values[10] =0.05893407762050629;
    explicit_values[11] =0;
    explicit_values[12] =0.08001075685024261;
    explicit_values[13] =-0.05893407762050629;
    explicit_values[14] =0.009999999776482582;
    explicit_values[15] =-0.08001075685024261;
    explicit_values[16] =-0.05893407762050629;
    explicit_values[17] =0.009999999776482582;
    explicit_values[18] =-0.08001075685024261;
    explicit_values[19] =0.05893407762050629;
    explicit_values[20] =0.009999999776482582;
    explicit_values[21] =0.08001075685024261;
    explicit_values[22] =0.05893407762050629;
    explicit_values[23] =0.009999999776482582;
    
    tetrahedrons[0] =7;
    tetrahedrons[1] =6;
    tetrahedrons[2] =2;
    tetrahedrons[3] =0;
    tetrahedrons[4] =7;
    tetrahedrons[5] =6;
    tetrahedrons[6] =0;
    tetrahedrons[7] =4;
    tetrahedrons[8] =6;
    tetrahedrons[9] =3;
    tetrahedrons[10] =0;
    tetrahedrons[11] =1;
    tetrahedrons[12] =6;
    tetrahedrons[13] =5;
    tetrahedrons[14] =0;
    tetrahedrons[15] =4;
    tetrahedrons[16] =6;
    tetrahedrons[17] =3;
    tetrahedrons[18] =2;
    tetrahedrons[19] =0;
    tetrahedrons[20] =6;
    tetrahedrons[21] =5;
    tetrahedrons[22] =1;
    tetrahedrons[23] =0;
    
    
    constraints[0] =4;
    constraints[1] =7;
    constraints[2] =6;
    constraints[3] =4;
    constraints[4] =6;
    constraints[5] =5;
    constraints[6] =0;
    constraints[7] =1;
    constraints[8] =3;
    constraints[9] =0;
    constraints[10] =3;
    constraints[11] =2;
    constraints[12] =0;
    constraints[13] =4;
    constraints[14] =5;
    constraints[15] =0;
    constraints[16] =5;
    constraints[17] =1;
    constraints[18] =1;
    constraints[19] =5;
    constraints[20] =6;
    constraints[21] =1;
    constraints[22] =6;
    constraints[23] =3;
    constraints[24] =3;
    constraints[25] =6;
    constraints[26] =7;
    constraints[27] =3;
    constraints[28] =7;
    constraints[29] =2;
    constraints[30] =2;
    constraints[31] =7;
    constraints[32] =4;
    constraints[33] =2;
    constraints[34] =4;
    constraints[35] =0;
    
    void* BSP = CreateBinarySpacePartitionHandle();
    AddBinarySpacePartitionInput(BSP, 8, explicit_values, 6, tetrahedrons, 12, constraints, false);
    CalculateBinarySpacePartition(BSP);
    
    void* IC = CreateInteriorCharacterizationHandle();
    AddInteriorCharacterizationInput(IC, 8, explicit_values, GetBinarySpacePartitionInsertedVerticesCount(BSP), GetBinarySpacePartitionInsertedVertices(BSP), GetBinarySpacePartitionPolyhedronsCount(BSP), GetBinarySpacePartitionPolyhedrons(BSP),
                                     GetBinarySpacePartitionFacetsCount(BSP), GetBinarySpacePartitionFacets(BSP), GetBinarySpacePartitionFacetsCentroids(BSP), GetBinarySpacePartitionFacetsCentroidsWeights(BSP), 12, constraints, 1.0);
    CalculateInteriorCharacterization(IC);
    DisposeBinarySpacePartitionHandle(BSP);
    DisposeInteriorCharacterizationHandle(IC);
    delete[] explicit_values;
    delete[] tetrahedrons;
    delete[] constraints;
   
    return 0;
}
