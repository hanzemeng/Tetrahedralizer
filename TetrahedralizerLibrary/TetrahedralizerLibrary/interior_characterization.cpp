#include "interior_characterization.hpp"
using namespace std;

void InteriorCharacterizationHandle::interior_characterization()
{
    vector<vector<vector<uint32_t>>> facets_vertices_mapping;
    m_facets_centroids_mapping = vector<uint32_t>(m_facets.size(), UNDEFINED_VALUE);
    vector<double> facets_approximated_areas;
    
    // calculate some data
    {
        vector<double3> vertices_approximated_positions;
        for(uint32_t i=0; i<m_vertices.size(); i++)
        {
            vertices_approximated_positions.push_back(approximate_point(m_vertices[i]));
        }
        double total_area = 0.0;
        for(uint32_t i=0; i<m_facets.size(); i++)
        {
            vector<uint32_t> vs = m_facets[i].get_sorted_vertices(m_segments);
            double area = 0.0;
            double3 p0 = vertices_approximated_positions[vs[0]];
            for(uint32_t j=1; j<vs.size()-1; j++)
            {
                double3 p1 = vertices_approximated_positions[vs[j]];
                double3 p2 = vertices_approximated_positions[vs[j+1]];
                area += (p1-p0).cross(p2-p0).length();
            }
            facets_approximated_areas.push_back(area);
            total_area += area;
        }
        for(uint32_t i=0; i<facets_approximated_areas.size(); i++)
        {
            facets_approximated_areas[i] /= total_area;
        }
    }
    
    for(uint32_t i=0; i<m_facets.size(); i++)
    {
        shared_ptr<genericPoint> centroid = make_shared<implicitPoint3D_BPT>
        (m_vertices[m_facets[i].p0]->toExplicit3D(), m_vertices[m_facets[i].p1]->toExplicit3D(), m_vertices[m_facets[i].p2]->toExplicit3D(),
         m_facets[i].w0, m_facets[i].w1);
        
//        for(uint32_t j=0; j<m_constraints.size(); j+=3)
//        {
//            uint32_t c0 = m_constraints[j+0];
//            uint32_t c1 = m_constraints[j+1];
//            uint32_t c2 = m_constraints[j+2];
//            if(UNDEFINED_VALUE == c0)
//            {
//                continue;
//            }
//            if(!m_facets[i].is_coplanar_constraint(c0, c1, c2, m_vertices))
//            {
//                continue;
//            }
//            if(genericPoint::pointInTriangle(*centroid,*m_vertices[c0],*m_vertices[c1],*m_vertices[c2]))
//            {
//                m_facets_centroids_mapping[i] = j/3;
//                break;
//            }
//        }
        
        for(uint32_t c : m_facets[i].constrains)
        {
            if(3*c >= m_constraints.size())
            {
                continue;
            }
            uint32_t c0 = m_constraints[3*c+0];
            uint32_t c1 = m_constraints[3*c+1];
            uint32_t c2 = m_constraints[3*c+2];
            if(genericPoint::pointInTriangle(*centroid,*m_vertices[c0],*m_vertices[c1],*m_vertices[c2]))
            {
                m_facets_centroids_mapping[i] = c;
                break;
            }
        }
    }

    GCoptimizationGeneralGraph gc((GCoptimization::SiteID)m_polyhedrons.size()+1, 2);
    // build graph and calculate min cut
    {
        // polyhedron cost
        gc.setDataCost((GCoptimization::SiteID)m_polyhedrons.size(), 1, 1.0); // ghost polyhedron
        for(uint32_t i=0; i<m_polyhedrons.size(); i++)
        {
            double out_area = 0.0; // area of facets point out
            double in_area = 0.0;
            
            for(uint32_t j=0; j<m_polyhedrons[i].size(); j++)
            {
                int orient = 0;
                uint32_t f = m_polyhedrons[i][j];
                uint32_t t = m_facets_centroids_mapping[f];
                if(UNDEFINED_VALUE == t)
                {
                    continue;
                }
                
                uint32_t t0 = m_constraints[3*t+0];
                uint32_t t1 = m_constraints[3*t+1];
                uint32_t t2 = m_constraints[3*t+2];
                for(uint32_t k=0; k<m_polyhedrons[i].size(); k++)
                {
                    if(k == j)
                    {
                        continue;
                    }
                    uint32_t nf = m_polyhedrons[i][k];
                    for(uint32_t v : m_facets[nf].get_sorted_vertices(m_segments))
                    {
                        if(0 != (orient=orient3d(t0,t1,t2,v, m_vertices.data())))
                        {
                            break;
                        }
                    }
                }
                if(orient > 0)
                {
                    in_area += facets_approximated_areas[f];
                }
                else if(orient < 0)
                {
                    out_area += facets_approximated_areas[f];
                }
//                else
//                {
//                    throw "wtf";
//                }
            }
            gc.setDataCost((GCoptimization::SiteID)i, 0, out_area);
            gc.setDataCost((GCoptimization::SiteID)i, 1, m_polyhedron_in_multiplier * in_area);
        }
        
        // neighbor cost
        vector<double> polyhedrons_to_ghost_weight = vector<double>(m_polyhedrons.size(), -1.0); // total area of facets connected to the ghost polyhedron, negative means not connectd
        for(uint32_t i=0; i<m_facets.size(); i++)
        {
            if(UNDEFINED_VALUE != m_facets_centroids_mapping[i])
            {
                continue;
            }

            uint32_t n0 = m_facets[i].ip0;
            uint32_t n1 = m_facets[i].ip1;
            if(n0 == UNDEFINED_VALUE)
            {
                swap(n0,n1);
            }

            if(UNDEFINED_VALUE == n1)
            {
                if(polyhedrons_to_ghost_weight[n0] < 0.0)
                {
                    polyhedrons_to_ghost_weight[n0] = facets_approximated_areas[i];
                }
                else
                {
                    polyhedrons_to_ghost_weight[n0] += facets_approximated_areas[i];
                }
                continue;
            }
            gc.setNeighbors((GCoptimization::SiteID)n0, (GCoptimization::SiteID)n1, facets_approximated_areas[i]);
        }
        for(uint32_t i=0; i<m_polyhedrons.size(); i++)
        {
            if(polyhedrons_to_ghost_weight[i] > 0.0)
            {
                gc.setNeighbors((GCoptimization::SiteID)i, (GCoptimization::SiteID)m_polyhedrons.size(), polyhedrons_to_ghost_weight[i]);
            }
        }
        
        gc.swap();
    }
    
    // produce output
    {
        for(uint32_t i=0; i<m_polyhedrons.size(); i++)
        {
            m_polyhedrons_labels.push_back(gc.whatLabel((GCoptimization::SiteID)i));
        }
        
        for(uint32_t i=0; i<facets_vertices_mapping.size(); i++)
        {
            for(uint32_t j=0; j<facets_vertices_mapping[i].size(); j++)
            {
                m_facets_vertices_mapping.push_back(facets_vertices_mapping[i][j].size());
                for(uint32_t k=0; k<facets_vertices_mapping[i][j].size(); k++)
                {
                    m_facets_vertices_mapping.push_back(facets_vertices_mapping[i][j][k]);
                }
            }
        }
    }
}


void InteriorCharacterizationHandle::Dispose()
{}

void InteriorCharacterizationHandle::AddInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, FacetInteropData* facets, uint32_t segments_count, SegmentInteropData* segments, uint32_t constraints_count, uint32_t* constraints, double polyhedron_in_multiplier)
{
    m_vertices = create_vertices(explicit_count, explicit_values, implicit_count, implicit_values);
    m_polyhedrons = flat_array_to_nested_vector(polyhedrons, polyhedrons_count);
    for(uint32_t i=0; i<facets_count; i++)
    {
        m_facets.push_back(facets[i].to_facet());
    }
    for(uint32_t i=0; i<segments_count; i++)
    {
        m_segments.push_back(segments[i].to_segment());
    }
    m_constraints = create_constraints(constraints_count, constraints, m_vertices.data(), true);
    m_polyhedron_in_multiplier = polyhedron_in_multiplier;
}

void InteriorCharacterizationHandle::Calculate()
{
    interior_characterization();
}

uint32_t* InteriorCharacterizationHandle::GetPolyhedronsLabels()
{
    return m_polyhedrons_labels.data();
}
uint32_t* InteriorCharacterizationHandle::GetFacetsVerticesMapping()
{
    return m_facets_vertices_mapping.data();
}
uint32_t* InteriorCharacterizationHandle::GetFacetsCentroidsMapping()
{
    return m_facets_centroids_mapping.data();
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

extern "C" LIBRARY_EXPORT void AddInteriorCharacterizationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values, uint32_t polyhedrons_count, uint32_t* polyhedrons, uint32_t facets_count, FacetInteropData* facets, uint32_t segments_count, SegmentInteropData* segments, uint32_t constraints_count, uint32_t* constraints, double polyhedron_in_multiplier)
{
    ((InteriorCharacterizationHandle*)handle)->AddInput(explicit_count, explicit_values, implicit_count, implicit_values, polyhedrons_count, polyhedrons, facets_count, facets, segments_count, segments, constraints_count, constraints, polyhedron_in_multiplier);
}

extern "C" LIBRARY_EXPORT void CalculateInteriorCharacterization(void* handle)
{
    ((InteriorCharacterizationHandle*)handle)->Calculate();
}

extern "C" LIBRARY_EXPORT uint32_t* GetInteriorCharacterizationPolyhedronsLabels(void* handle)
{
    return ((InteriorCharacterizationHandle*)handle)->GetPolyhedronsLabels();
}
extern "C" LIBRARY_EXPORT uint32_t* GetInteriorCharacterizationFacetsVerticesMapping(void* handle)
{
    return ((InteriorCharacterizationHandle*)handle)->GetFacetsVerticesMapping();
}
extern "C" LIBRARY_EXPORT uint32_t* GetInteriorCharacterizationFacetsCentroidsMapping(void* handle)
{
    return ((InteriorCharacterizationHandle*)handle)->GetFacetsCentroidsMapping();
}
