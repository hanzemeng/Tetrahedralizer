#include "common_header.h"
#include "common_function.h"
#include "delaunay_tetrahedralization.hpp"
#include "binary_space_partition.hpp"
#include "interior_characterization.hpp"
#include "polyhedralization_tetrahedralization.hpp"

int main(int argc, const char * argv[])
{
    for(int i=0; i<3; i++)
    {
        for(int j=0; j<3; j++)
        {
            for(int k=0; k<3; k++)
            {
                uint32_t a = i;
                uint32_t b = j;
                uint32_t c = k;
                sort_ints(a, b, c);
                
                cout << a << " "  << b << " " << c << endl;
            }
        }
    }
   
    return 0;
}
