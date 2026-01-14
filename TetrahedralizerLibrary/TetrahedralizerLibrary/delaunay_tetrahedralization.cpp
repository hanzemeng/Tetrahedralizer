#include "delaunay_tetrahedralization.hpp"
using namespace std;

void DelaunayTetrahedralizationHandle::Dispose()
{}
void DelaunayTetrahedralizationHandle::AddInput(uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values)
{
    m_vertices = create_vertices(explicit_count, explicit_values, implicit_count, implicit_values);
}

void DelaunayTetrahedralizationHandle::Calculate()
{
    delaunay_tetrahedralization();
}

uint32_t DelaunayTetrahedralizationHandle::GetTetrahedronsCount()
{
    return m_tetrahedrons.size();
}
void DelaunayTetrahedralizationHandle::GetTetrahedrons(uint32_t* out)
{
    write_buffer_with_vector(out, m_tetrahedrons);
}

extern "C" LIBRARY_EXPORT void* CreateDelaunayTetrahedralizationHandle()
{
    return new DelaunayTetrahedralizationHandle();
}
extern "C" LIBRARY_EXPORT void DisposeDelaunayTetrahedralizationHandle(void* handle)
{
    ((DelaunayTetrahedralizationHandle*)handle)->Dispose();
    delete (DelaunayTetrahedralizationHandle*)handle;
}

extern "C" LIBRARY_EXPORT void AddDelaunayTetrahedralizationInput(void* handle, uint32_t explicit_count, double* explicit_values, uint32_t implicit_count, uint32_t* implicit_values)
{
    ((DelaunayTetrahedralizationHandle*)handle)->AddInput(explicit_count, explicit_values, implicit_count, implicit_values);
}

extern "C" LIBRARY_EXPORT void CalculateDelaunayTetrahedralization(void* handle)
{
    ((DelaunayTetrahedralizationHandle*)handle)->Calculate();
}

extern "C" LIBRARY_EXPORT uint32_t GetDelaunayTetrahedralizationTetrahedronsCount(void* handle)
{
    return ((DelaunayTetrahedralizationHandle*)handle)->GetTetrahedronsCount();
}
void GetDelaunayTetrahedralizationTetrahedrons(void* handle, uint32_t* out)
{
    ((DelaunayTetrahedralizationHandle*)handle)->GetTetrahedrons(out);
}


void DelaunayTetrahedralizationHandle::delaunay_tetrahedralization()
{
    uint32_t op2, op3;
    // find op2 and op3, and build the first tetrahedron
    {
        uint32_t p0, p1, p2, p3;
        p0 = 0;
        p1 = 1;
        
        for(p2=2; p2<m_vertices.size(); p2++)
        {
            for(p3=p2+1; p3<m_vertices.size(); p3++)
            {
                if(0 == orient3d(p0,p1,p2,p3,m_vertices.data()))
                {
                    continue;
                }
                goto FOUND_FIRST_TETRAHEDRON;
            }
        }
        throw "input vertices are all on the same plane";

        FOUND_FIRST_TETRAHEDRON:
        if(-1 == orient3d(p0,p1,p2,p3,m_vertices.data()))
        {
            swap(p0,p1);
        }

        uint32_t t0 = add_tetrahedron(p0,p1,p2,p3);
        uint32_t t1 = add_tetrahedron(p0,p2,p1,INFINITE_VERTEX);
        uint32_t t2 = add_tetrahedron(p0,p1,p3,INFINITE_VERTEX);
        uint32_t t3 = add_tetrahedron(p0,p3,p2,INFINITE_VERTEX);
        uint32_t t4 = add_tetrahedron(p1,p2,p3,INFINITE_VERTEX);

        m_neighbors[t0+0] = t1+0;
        m_neighbors[t0+1] = t2+0;
        m_neighbors[t0+2] = t3+0;
        m_neighbors[t0+3] = t4+0;
        m_neighbors[t1+0] = t0+0;
        m_neighbors[t1+1] = t3+2;
        m_neighbors[t1+2] = t2+1;
        m_neighbors[t1+3] = t4+1;
        m_neighbors[t2+0] = t0+1;
        m_neighbors[t2+1] = t1+2;
        m_neighbors[t2+2] = t3+1;
        m_neighbors[t2+3] = t4+2;
        m_neighbors[t3+0] = t0+2;
        m_neighbors[t3+1] = t2+2;
        m_neighbors[t3+2] = t1+1;
        m_neighbors[t3+3] = t4+3;
        m_neighbors[t4+0] = t0+3;
        m_neighbors[t4+1] = t1+3;
        m_neighbors[t4+2] = t2+3;
        m_neighbors[t4+3] = t3+3;

        op2 = p2;
        op3 = p3;
    }

    // insert the rest of vertices
    uint32_t origin_t = 0;
    for(uint32_t i=2; i<m_vertices.size(); i++)
    {
        if(op2 == i || op3 == i)
        {
            continue;
        }
        
        // find a tetrahedron that contains the ith vertex, store the result in origin_t
        {
            if(INFINITE_VERTEX == m_tetrahedrons[origin_t+3])
            {
                origin_t = get_tetrahedron_neighbor(origin_t,0);
            }
            uint32_t f = 0;
            uint32_t last_f = 4;
            while(f < 4)
            {
                if(INFINITE_VERTEX == m_tetrahedrons[origin_t+3])
                {
                    break;
                }

                for(f=0; f<4; f++)
                {
                    if(f == last_f)
                    {
                        continue;
                    }
                    
                    auto[p0,p1,p2] = get_tetrahedron_facet(origin_t, f);
                    if(orient3d(p0,p1,p2,i,m_vertices.data()) < 0)
                    {
                        last_f = m_neighbors[origin_t+f]&3;
                        origin_t = get_tetrahedron_neighbor(origin_t,f);
                        break;
                    }
                }
            }
        }

        // remove tetrahedrons whose circumsphere contains i
        unordered_set<uint32_t> bounding_facets;
        {
            unordered_set<uint32_t> visited_tetrahedrons;
            visited_tetrahedrons.insert(UNDEFINED_VALUE);
            queue<uint32_t> visit_tetrahedrons;
            unordered_set<uint32_t> removed_tetrahedrons;
            visit_tetrahedrons.push(origin_t);
            while(!visit_tetrahedrons.empty())
            {
                uint32_t t = visit_tetrahedrons.front();
                visit_tetrahedrons.pop();

                if(visited_tetrahedrons.end() != visited_tetrahedrons.find(t))
                {
                    continue;
                }
                visited_tetrahedrons.insert(t);

                if(1 != in_sphere(t,i))
                {
                    continue;
                }

                for(uint32_t f=0; f<4; f++)
                {
                    uint32_t n = get_tetrahedron_neighbor(t,f);
                    visit_tetrahedrons.push(n);
                    if(bounding_facets.end() != bounding_facets.find(t+f))
                    {
                        bounding_facets.erase(t+f);
                    }
                    else
                    {
                        bounding_facets.insert(m_neighbors[t+f]);
                    }
                }
                removed_tetrahedrons.insert(t);
            }
            for(uint32_t t : removed_tetrahedrons)
            {
                remove_tetrahedron(t);
            }
        }
        // tetrahedralize hole
        {
            unordered_map<pair<uint32_t,uint32_t>,uint32_t,ii32_hash> neighbors_cache;
            auto connect_neighbor = [&](uint32_t p0, uint32_t p1, uint32_t t)
            {
                uint32_t n = search_int(p0,p1, neighbors_cache);
                if(UNDEFINED_VALUE == n)
                {
                    assign_int(p0, p1, t, neighbors_cache);
                }
                else
                {
                    m_neighbors[t] = n;
                    m_neighbors[n] = t;
                    remove_int(p0, p1, neighbors_cache);
                }
            };
            for(uint32_t t : bounding_facets)
            {
                auto[f0,f1,f2] = get_tetrahedron_facet(t&0xfffffffc, t%4);
                uint32_t nt = add_tetrahedron(i,f0,f1,f2);
                origin_t = nt; // used for searching the next point
                m_neighbors[nt+3] = t;
                m_neighbors[t] = nt+3;

                connect_neighbor(f0,f1,nt+0);
                connect_neighbor(f0,f2,nt+1);
                connect_neighbor(f1,f2,nt+2);
            }
        }
    }

    // remove deleted and infinte tetrahedrons
    for(uint32_t i=0; i<m_tetrahedrons.size(); i+=4)
    {
        if(UNDEFINED_VALUE != m_tetrahedrons[i] && INFINITE_VERTEX != m_tetrahedrons[i+3])
        {
            continue;
        }
        
        uint32_t j = m_tetrahedrons.size() - 4;
        swap(m_tetrahedrons[i+0], m_tetrahedrons[j+0]);
        swap(m_tetrahedrons[i+1], m_tetrahedrons[j+1]);
        swap(m_tetrahedrons[i+2], m_tetrahedrons[j+2]);
        swap(m_tetrahedrons[i+3], m_tetrahedrons[j+3]);
        m_tetrahedrons.pop_back();
        m_tetrahedrons.pop_back();
        m_tetrahedrons.pop_back();
        m_tetrahedrons.pop_back();
        
        i-=4;
    }
}

uint32_t DelaunayTetrahedralizationHandle::get_tetrahedron_neighbor(uint32_t t, uint32_t i)
{
    if(UNDEFINED_VALUE == m_neighbors[t+i])
    {
        return UNDEFINED_VALUE;
    }
    else
    {
        return m_neighbors[t+i] & 0xfffffffc;
    }
}
tuple<uint32_t,uint32_t,uint32_t> DelaunayTetrahedralizationHandle::get_tetrahedron_facet(uint32_t t, uint32_t i)
{
    uint32_t f0 = m_tetrahedrons[t+0];
    uint32_t f1 = m_tetrahedrons[t+1];
    uint32_t f2 = m_tetrahedrons[t+2];
    uint32_t f3 = m_tetrahedrons[t+3];
    switch(i)
    {
        case 0:
            return make_tuple(f0,f1,f2);
        case 1:
            return make_tuple(f1,f0,f3);
        case 2:
            return make_tuple(f0,f2,f3);
        case 3:
            return make_tuple(f2,f1,f3);
        default:
            throw "wrong face index";
    }
}

int DelaunayTetrahedralizationHandle::symbolic_perturbation(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3,uint32_t p4)
{
    uint32_t indices[5] = {p0,p1,p2,p3,p4};
    uint32_t swaps = 0;
    uint32_t n = 5;
    uint32_t count;
    do
    {
        count = 0;
        n = n - 1;
        for (uint32_t i = 0; i < n; i++)
        {
            if (indices[i] > indices[i+1])
            {
                swap(indices[i], indices[i+1]);
                count++;
            }
        }
        swaps += count;
    }while(0 != count);

    int oriA = orient3d(indices[1], indices[2], indices[3], indices[4], m_vertices.data());
    if (oriA != 0)
    {
        if((swaps % 2) != 0)
        {
            oriA = -oriA;
        }
        return oriA;
    }

    int oriB = -orient3d(indices[0], indices[2], indices[3], indices[4], m_vertices.data());
    if ((swaps % 2) != 0)
    {
        oriB = -oriB;
    }
    return oriB;
}
int DelaunayTetrahedralizationHandle::in_sphere(uint32_t t,uint32_t p)
{
    int res;
    if(INFINITE_VERTEX != m_tetrahedrons[t+3])
    {
        res = ::in_sphere(m_tetrahedrons[t+0],m_tetrahedrons[t+1],m_tetrahedrons[t+2],m_tetrahedrons[t+3],p,m_vertices.data());
        if(0 != res)
        {
            return res;
        }
        res = symbolic_perturbation(m_tetrahedrons[t+0],m_tetrahedrons[t+1],m_tetrahedrons[t+2],m_tetrahedrons[t+3],p);
        if(0 == res)
        {
            throw "symbolic perturbation failed";
        }
        return res;
    }

    res = orient3d(m_tetrahedrons[t+0],m_tetrahedrons[t+1],m_tetrahedrons[t+2],p,m_vertices.data());
    if(0 != res)
    {
        return res;
    }
    
    uint32_t opposite_p = m_tetrahedrons[(m_neighbors[t+0]&0xfffffffc) + (3-(m_neighbors[t+0]&3))];
    
    res = ::in_sphere(m_tetrahedrons[t+0],m_tetrahedrons[t+1],m_tetrahedrons[t+2],opposite_p,p,m_vertices.data());
    if(0 != res)
    {
        return -res;
    }
    res = symbolic_perturbation(m_tetrahedrons[t+0],m_tetrahedrons[t+1],m_tetrahedrons[t+2],opposite_p,p);
    if(0 == res)
    {
        throw "symbolic perturbation failed";
    }
    return -res;
}

uint32_t DelaunayTetrahedralizationHandle::add_tetrahedron(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3)
{
    uint32_t res;
    if(m_tetrahedrons_gaps.empty())
    {
        res = m_tetrahedrons.size();
        m_tetrahedrons.push_back(p0);
        m_tetrahedrons.push_back(p1);
        m_tetrahedrons.push_back(p2);
        m_tetrahedrons.push_back(p3);
        m_neighbors.push_back(UNDEFINED_VALUE);
        m_neighbors.push_back(UNDEFINED_VALUE);
        m_neighbors.push_back(UNDEFINED_VALUE);
        m_neighbors.push_back(UNDEFINED_VALUE);
    }
    else
    {
        res = m_tetrahedrons_gaps.front();
        m_tetrahedrons_gaps.pop();
        m_tetrahedrons[res+0] = p0;
        m_tetrahedrons[res+1] = p1;
        m_tetrahedrons[res+2] = p2;
        m_tetrahedrons[res+3] = p3;
        m_neighbors[res+0] = UNDEFINED_VALUE;
        m_neighbors[res+1] = UNDEFINED_VALUE;
        m_neighbors[res+2] = UNDEFINED_VALUE;
        m_neighbors[res+3] = UNDEFINED_VALUE;
    }

    return res;
}

void DelaunayTetrahedralizationHandle::remove_tetrahedron(uint32_t t)
{
    m_tetrahedrons_gaps.push(t);
    for(uint32_t i=0; i<4; i++)
    {
        m_tetrahedrons[t+i] = UNDEFINED_VALUE;
        uint32_t n = m_neighbors[t+i];
        if(UNDEFINED_VALUE==n)
        {
            continue;
        }
        m_neighbors[n] = UNDEFINED_VALUE;
        m_neighbors[t+i] = UNDEFINED_VALUE;
    }
}
