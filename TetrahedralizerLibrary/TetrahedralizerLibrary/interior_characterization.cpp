#include "interior_characterization.hpp"


void InteriorCharacterization::interior_characterization(InteriorCharacterizationInput* input, InteriorCharacterizationOutput* output)
{
    vector<double> m_vertices_approximate_positions;
    vector<double> m_polyhedrons_facets_approximate_areas;
    vector<double> m_polyhedrons_facets_approximate_centers;
    vector<double> m_polyhedrons_winding_numbers;
    
    // estimate vertices positions
    {
        GenericPointApproximationInput g_input;
        g_input.m_vertices = input->m_vertices;
        g_input.m_vertices_count = input->m_vertices_count;
        GenericPointApproximationOutput g_output;
        GenericPointApproximation genericPointApproximation;
        genericPointApproximation.generic_point_approximation(&g_input, &g_output);
        
        for(uint32_t i=0; i<3*input->m_vertices_count; i++)
        {
            m_vertices_approximate_positions.push_back(g_output.m_approximate_positions[i]);
        }
        delete[] g_output.m_approximate_positions;
    }
    
    
    // estimate facets areaa and centers
    {
        // facets areas
        double total_area = 0;
        m_polyhedrons_facets_approximate_areas.resize(input->m_polyhedrons_facets_count);
        m_polyhedrons_facets_approximate_centers.resize(3*input->m_polyhedrons_facets_count);
        uint32_t f=0;
        for(uint32_t i=0; i<input->m_polyhedrons_facets_count; i++)
        {
            // store facet vertices to m_vector_i_0
            uint32_t n = input->m_polyhedrons_facets[f];
            m_vector_i_0.clear();
            for(uint32_t j=f+1; j<f+1+n; j++)
            {
                m_vector_i_0.push_back(input->m_polyhedrons_facets[j]);
            }
            f += n+1;

            // calculate facet area
            double area = 0;
            double a11,a12,a13,a21,a22,a23,a31,a32,a33, r1,r2,r3;
            a11 = m_vertices_approximate_positions[3*m_vector_i_0[0]+0];
            a12 = m_vertices_approximate_positions[3*m_vector_i_0[0]+1];
            a13 = m_vertices_approximate_positions[3*m_vector_i_0[0]+2];
            
            for(uint32_t j=2; j<m_vector_i_0.size(); j++)
            {
                a21 = m_vertices_approximate_positions[3*m_vector_i_0[j-1]+0] - a11;
                a22 = m_vertices_approximate_positions[3*m_vector_i_0[j-1]+1] - a12;
                a23 = m_vertices_approximate_positions[3*m_vector_i_0[j-1]+2] - a13;
                a31 = m_vertices_approximate_positions[3*m_vector_i_0[j]+0] - a11;
                a32 = m_vertices_approximate_positions[3*m_vector_i_0[j]+1] - a12;
                a33 = m_vertices_approximate_positions[3*m_vector_i_0[j]+2] - a13;

                r1 = a22*a33 - a23*a32;
                r2 = -a21*a33 + a23*a31;
                r3 = a21*a32 - a22*a31;

                area += r1*r1 + r2*r2 + r3*r3;
            }
            total_area += area;
            m_polyhedrons_facets_approximate_areas[i] = area;
            
            // calculate facet center
            double cx(0),cy(0),cz(0);
            for(uint32_t v : m_vector_i_0)
            {
                cx += m_vertices_approximate_positions[3*v+0];
                cy += m_vertices_approximate_positions[3*v+1];
                cz += m_vertices_approximate_positions[3*v+2];
            }
            cx /= m_vector_i_0.size();
            cy /= m_vector_i_0.size();
            cz /= m_vector_i_0.size();
            m_polyhedrons_facets_approximate_centers[3*i+0] = cx;
            m_polyhedrons_facets_approximate_centers[3*i+1] = cy;
            m_polyhedrons_facets_approximate_centers[3*i+2] = cz;
        }
        // normalize facet area so min cut will work
        for(uint32_t i=0; i<m_polyhedrons_facets_approximate_areas.size(); i++)
        {
            m_polyhedrons_facets_approximate_areas[i] /= total_area;
        }
    }
    
    // estimate polyhedrons winding numbers
    {
        if(nullptr != input->m_polyhedrons_winding_numbers)
        {
            m_polyhedrons_winding_numbers = vector<double>(input->m_polyhedrons_winding_numbers, input->m_polyhedrons_winding_numbers+input->m_polyhedrons_count);
            goto MIN_CUT;
        }
        m_polyhedrons_winding_numbers.resize(input->m_polyhedrons_count);
        uint32_t f=0;
        for(uint32_t i=0; i<input->m_polyhedrons_count; i++)
        {
            // store facets to m_vector_i_0
            uint32_t n = input->m_polyhedrons[f];
            m_vector_i_0.clear();
            for(uint32_t j=f+1; j<f+1+n; j++)
            {
                m_vector_i_0.push_back(input->m_polyhedrons[j]);
            }
            f += n+1;
            
            double px(0),py(0),pz(0);
            for(uint32_t f : m_vector_i_0)
            {
                px += m_polyhedrons_facets_approximate_centers[3*f+0];
                py += m_polyhedrons_facets_approximate_centers[3*f+1];
                pz += m_polyhedrons_facets_approximate_centers[3*f+2];
            }
            px /= m_vector_i_0.size();
            py /= m_vector_i_0.size();
            pz /= m_vector_i_0.size();
            
            double wind = 0;
            for(uint32_t j=0; j<3*input->m_constraints_count; j+=3)
            {
                uint32_t c0(input->m_constraints[j+0]),c1(input->m_constraints[j+1]),c2(input->m_constraints[j+2]);
                
                double
                ax(m_vertices_approximate_positions[3*c0+0] - px),
                ay(m_vertices_approximate_positions[3*c0+1] - py),
                az(m_vertices_approximate_positions[3*c0+2] - pz),
                bx(m_vertices_approximate_positions[3*c1+0] - px),
                by(m_vertices_approximate_positions[3*c1+1] - py),
                bz(m_vertices_approximate_positions[3*c1+2] - pz),
                cx(m_vertices_approximate_positions[3*c2+0] - px),
                cy(m_vertices_approximate_positions[3*c2+1] - py),
                cz(m_vertices_approximate_positions[3*c2+2] - pz);
                
                double
                aa(sqrt(ax*ax + ay*ay + az*az)),
                ba(sqrt(bx*bx + by*by + bz*bz)),
                ca(sqrt(cx*cx + cy*cy + cz*cz));
                
                double y = ax*(by*cz-bz*cy) - ay*(bx*cz-bz*cx) + az*(bx*cy-by*cx);
                double x = aa*ba*ca + (ax*bx+ay*by+az*bz)*ca + (bx*cx+by*cy+bz*cz)*aa + (cx*ax+cy*ay+cz*az)*ba;
                double cur_wind = atan2(y, x);
                cur_wind /= (2*numbers::pi);
                
                wind += cur_wind;
            }
            m_polyhedrons_winding_numbers[i] = wind;
        }
    }
    
    MIN_CUT:
    
    // build graph and calculate min cut
    {
        Polyhedralization polyhedralization(input->m_polyhedrons, input->m_polyhedrons_count, input->m_polyhedrons_facets, input->m_polyhedrons_facets_count);
        GCoptimizationGeneralGraph gc((GCoptimization::SiteID)input->m_polyhedrons_count, 2);

        // site cost
        for(uint32_t i=0; i<input->m_polyhedrons_count; i++)
        {
            gc.setDataCost((GCoptimization::SiteID)i, 0, max(m_polyhedrons_winding_numbers[i],(double)0));
            gc.setDataCost((GCoptimization::SiteID)i, 1, max(1-m_polyhedrons_winding_numbers[i],(double)0));

        }
        // neightbor cost
        for(uint32_t i=0; i<input->m_polyhedrons_facets_count; i++)
        {
            uint32_t n0,n1;
            polyhedralization.get_polyhedron_facet_neighbors(i, n0, n1);
            if(UNDEFINED_VALUE == n0 || UNDEFINED_VALUE == n1)
            {
                continue;
            }
            double w = m_polyhedrons_winding_numbers[n0] - m_polyhedrons_winding_numbers[n1];
            w = m_polyhedrons_facets_approximate_areas[i] * exp(-(w*w));
            gc.setNeighbors((GCoptimization::SiteID)n0, (GCoptimization::SiteID)n1, input->m_min_cut_neighbor_multiplier*w);
        }
        
        gc.swap();
        
        // copy result to output
        output->m_polyhedrons_interior_labels = new uint32_t[input->m_polyhedrons_count];
        for(uint32_t i=0; i<input->m_polyhedrons_count; i++)
        {
            output->m_polyhedrons_interior_labels[i] = gc.whatLabel((GCoptimization::SiteID)i);
        }
        if(nullptr == input->m_polyhedrons_winding_numbers)
        {
            output->m_polyhedrons_winding_numbers = new double[input->m_polyhedrons_count];
            for(uint32_t i=0; i<input->m_polyhedrons_count; i++)
            {
                output->m_polyhedrons_winding_numbers[i] = m_polyhedrons_winding_numbers[i];
            }
        }
    }
}


InteriorCharacterizationHandle::InteriorCharacterizationHandle()
{
    m_input = new InteriorCharacterizationInput();
    m_input->m_vertices_count = 0;
    m_input->m_polyhedrons_count = 0;
    m_input->m_polyhedrons_facets_count = 0;
    m_input->m_constraints_count = 0;
    m_input->m_vertices = nullptr;
    m_input->m_polyhedrons = nullptr;
    m_input->m_polyhedrons_facets = nullptr;
    m_input->m_constraints = nullptr;
    m_output = new InteriorCharacterizationOutput();
    m_output->m_polyhedrons_winding_numbers = nullptr;
    m_output->m_polyhedrons_interior_labels = nullptr;
    m_interiorCharacterization = new InteriorCharacterization();
}
void InteriorCharacterizationHandle::Dispose()
{
    delete_vertices(m_input->m_vertices, m_input->m_vertices_count);
    delete[] m_input->m_polyhedrons;
    delete[] m_input->m_polyhedrons_facets;
    delete[] m_input->m_constraints;
    delete[] m_input->m_polyhedrons_winding_numbers;
    delete m_input;
    delete[] m_output->m_polyhedrons_winding_numbers;
    delete[] m_output->m_polyhedrons_interior_labels;
    delete m_output;
    delete m_interiorCharacterization;
}

void InteriorCharacterizationHandle::AddInteriorCharacterizationInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t polyhedrons_facets_count, uint32_t* polyhedrons_facets, uint32_t constraints_count, uint32_t* constraints, double* polyhedrons_winding_numbers, double min_cut_neighbor_multiplier)
{
    create_vertices(explicit_count, explicit_values, implicit_count, implicit_values, m_input->m_vertices, m_input->m_vertices_count);

    vector<uint32_t> vec = read_flat_vector(polyhedrons_count, polyhedrons);
    vector_to_array(vec , m_input->m_polyhedrons);
    m_input->m_polyhedrons_count = polyhedrons_count;
    
    vec =  read_flat_vector(polyhedrons_facets_count, polyhedrons_facets);
    vector_to_array(vec , m_input->m_polyhedrons_facets);
    m_input->m_polyhedrons_facets_count = polyhedrons_facets_count;
    
    m_input->m_constraints = new uint32_t[3*constraints_count];
    for(uint32_t i=0; i<3*constraints_count; i++)
    {
        m_input->m_constraints[i] = constraints[i];
    }
    m_input->m_constraints_count = constraints_count;
    
    if(nullptr != polyhedrons_winding_numbers)
    {
        m_input->m_polyhedrons_winding_numbers = new double[polyhedrons_count];
        for(uint32_t i=0; i<polyhedrons_count; i++)
        {
            m_input->m_polyhedrons_winding_numbers[i] = polyhedrons_winding_numbers[i];
        }
    }
    
    m_input->m_min_cut_neighbor_multiplier = min_cut_neighbor_multiplier;
}

void InteriorCharacterizationHandle::CalculateInteriorCharacterization()
{
    m_interiorCharacterization->interior_characterization(m_input, m_output);
}

double* InteriorCharacterizationHandle::GetOutputPolyhedronsWindingNumbers()
{
    return m_output->m_polyhedrons_winding_numbers;
}
uint32_t* InteriorCharacterizationHandle::GetOutputPolyhedronsLabels()
{
    return m_output->m_polyhedrons_interior_labels;
}


extern "C" LIBRARY_EXPORT void* CreateInteriorCharacterizationHandle()
{
    return new InteriorCharacterizationHandle();
}
extern "C" LIBRARY_EXPORT void DisposeInteriorCharacterizationHandle(void* handle)
{
    ((InteriorCharacterizationHandle*)handle)->Dispose();
    delete (InteriorCharacterizationHandle*)handle;
}

extern "C" LIBRARY_EXPORT void AddInteriorCharacterizationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t polyhedrons_facets_count, uint32_t* polyhedrons_facets, uint32_t constraints_count, uint32_t* constraints, double* polyhedrons_winding_numbers, double min_cut_neighbor_multiplier)
{
    ((InteriorCharacterizationHandle*)handle)->AddInteriorCharacterizationInput(explicit_count, explicit_values, implicit_count, implicit_values, polyhedrons_count, polyhedrons, polyhedrons_facets_count, polyhedrons_facets, constraints_count, constraints, polyhedrons_winding_numbers, min_cut_neighbor_multiplier);
}

extern "C" LIBRARY_EXPORT void CalculateInteriorCharacterization(void* handle)
{
    ((InteriorCharacterizationHandle*)handle)->CalculateInteriorCharacterization();
}

extern "C" LIBRARY_EXPORT double* GetOutputPolyhedronsWindingNumbers(void* handle)
{
    return ((InteriorCharacterizationHandle*)handle)->GetOutputPolyhedronsWindingNumbers();
}
extern "C" LIBRARY_EXPORT uint32_t* GetOutputPolyhedronsLabels(void* handle)
{
    return ((InteriorCharacterizationHandle*)handle)->GetOutputPolyhedronsLabels();
}
