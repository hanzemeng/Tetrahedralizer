#include "delaunay_tetrahedralization.h"

void delaunay_tetrahedralization()
{
    m_tetrahedrons.clear();
    while(!m_tetrahedrons_gaps.empty())
    {
        m_tetrahedrons_gaps.pop();
    }
    m_neighbors.clear();

    uint32_t op2, op3;
    // find op2 and op3, and build the first tetrahedron
    {
        uint32_t p0, p1, p2, p3;
        p0 = 0;
        p1 = 1;
        
        for(p2=2; p2<m_vertices_count; p2++)
        {
            for(p3=p2+1; p3<m_vertices_count; p3++)
            {
                if(0 == orient3d(p0,p1,p2,p3))
                {
                    continue;
                }
                goto FOUND_FIRST_TETRAHEDRON;
            }
        }
        throw "input vertices are all on the same plane";
        
        FOUND_FIRST_TETRAHEDRON:
        if(-1 == orient3d(p0,p1,p2,p3))
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
    
    uint32_t origin_t = 0;
    for(uint32_t i=2; i<m_vertices_count; i++)
    {
        if(op2 == i || op3 == i)
        {
            continue;
        }
        
        // find a tetrahedron that contains the ith vertex, store the result in origin_t
        {
            if(INFINITE_VERTEX == m_tetrahedrons[origin_t+3])
            {
                get_tetrahedron_neighbor(origin_t,0,origin_t);
            }
            uint32_t f = 0;
            uint32_t last_f = 4;
            uint32_t p0,p1,p2;
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
                    get_tetrahedron_face(origin_t, f, p0, p1, p2);
                    if(orient3d(p0,p1,p2,i) < 0)
                    {
                        last_f = m_neighbors[origin_t+f]&3;
                        get_tetrahedron_neighbor(origin_t,f,origin_t);
                        break;
                    }
                }
            }
        }

        // m_u_set_i_0 a list of bounding triangles
        // m_u_set_i_1 a list of visited tetrahedrons
        // m_u_set_i_2 a list of tetrahedrons to be removed
        // m_queue_i_0 a list tetrahedrons to be visited
        {
            m_u_set_i_0.clear();
            m_u_set_i_1.clear();
            m_u_set_i_1.insert(UNDEFINED_VALUE);
            m_u_set_i_2.clear();
            m_queue_i_0.push(origin_t);
            while(!m_queue_i_0.empty())
            {
                uint32_t t = m_queue_i_0.front();
                m_queue_i_0.pop();

                if(m_u_set_i_1.end() != m_u_set_i_1.find(t))
                {
                    continue;
                }
                m_u_set_i_1.insert(t);

                if(1 != in_sphere(t,i))
                {
                    continue;
                }

                for(uint32_t f=0; f<4; f++)
                {
                    uint32_t n;
                    get_tetrahedron_neighbor(t,f,n);
                    m_queue_i_0.push(n);
                    if(m_u_set_i_0.end() != m_u_set_i_0.find(t+f))
                    {
                        m_u_set_i_0.erase(t+f);
                    }
                    else
                    {
                        m_u_set_i_0.insert(m_neighbors[t+f]);
                    }
                }
                m_u_set_i_2.insert(t);
            }
            for(uint32_t t : m_u_set_i_2)
            {
                remove_tetrahedron(t);
            }
        }
        
        // tetrahedralize hole
        // m_map_ii_i_0 stores newly created triangles that do not have neighbors
        {
            m_map_ii_i_0.clear();
            for(uint32_t tri : m_u_set_i_0)
            {
                uint32_t f0,f1,f2;
                get_tetrahedron_face(tri,f0,f1,f2);
                uint32_t nt = add_tetrahedron(i,f0,f1,f2);
                origin_t = nt; // used for searching the next point
                m_neighbors[nt+3] = tri;
                m_neighbors[tri] = nt+3;

                tetrahedralize_hole_helper(f0,f1,nt+0);
                tetrahedralize_hole_helper(f0,f2,nt+1);
                tetrahedralize_hole_helper(f1,f2,nt+2);
            }
        }
    }
}

void delaunay_tetrahedralization(const unordered_set<uint32_t>& input_vertices)
{
    m_tetrahedrons.clear();
    while(!m_tetrahedrons_gaps.empty())
    {
        m_tetrahedrons_gaps.pop();
    }
    m_neighbors.clear();
    
    m_vector_i_0.clear();
    for(uint32_t v : input_vertices)
    {
        m_vector_i_0.push_back(v);
    }

    uint32_t op2, op3;
    // find op2 and op3, and build the first tetrahedron
    {
        uint32_t p0, p1, p2, p3;
        p0 = m_vector_i_0[0];
        p1 = m_vector_i_0[1];
        
        for(uint32_t i=2; i<m_vector_i_0.size(); i++)
        {
            p2 = m_vector_i_0[i];
            for(uint32_t j=i+1; j<m_vector_i_0.size(); j++)
            {
                p3 = m_vector_i_0[j];
                if(0 == orient3d(p0,p1,p2,p3))
                {
                    continue;
                }
                goto FOUND_FIRST_TETRAHEDRON;
            }
        }
        throw "input vertices are all on the same plane";

        FOUND_FIRST_TETRAHEDRON:
        if(-1 == orient3d(p0,p1,p2,p3))
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
    
    uint32_t origin_t = 0;
    for(uint32_t i=2; i<m_vector_i_0.size(); i++)
    {
        if(op2 == m_vector_i_0[i] || op3 == m_vector_i_0[i])
        {
            continue;
        }
        
        // find a tetrahedron that contains the ith vertex, store the result in origin_t
        {
            if(INFINITE_VERTEX == m_tetrahedrons[origin_t+3])
            {
                get_tetrahedron_neighbor(origin_t,0,origin_t);
            }
            uint32_t f = 0;
            uint32_t last_f = 4;
            uint32_t p0,p1,p2;
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
                    get_tetrahedron_face(origin_t, f, p0, p1, p2);
                    if(orient3d(p0,p1,p2,m_vector_i_0[i]) < 0)
                    {
                        last_f = m_neighbors[origin_t+f]&3;
                        get_tetrahedron_neighbor(origin_t,f,origin_t);
                        break;
                    }
                }
            }
        }

        // m_u_set_i_0 a list of bounding triangles
        // m_u_set_i_1 a list of visited tetrahedrons
        // m_u_set_i_2 a list of tetrahedrons to be removed
        // m_queue_i_0 a list tetrahedrons to be visited
        {
            m_u_set_i_0.clear();
            m_u_set_i_1.clear();
            m_u_set_i_1.insert(UNDEFINED_VALUE);
            m_u_set_i_2.clear();
            m_queue_i_0.push(origin_t);
            while(!m_queue_i_0.empty())
            {
                uint32_t t = m_queue_i_0.front();
                m_queue_i_0.pop();

                if(m_u_set_i_1.end() != m_u_set_i_1.find(t))
                {
                    continue;
                }
                m_u_set_i_1.insert(t);

                if(1 != in_sphere(t,m_vector_i_0[i]))
                {
                    continue;
                }

                for(uint32_t f=0; f<4; f++)
                {
                    uint32_t n;
                    get_tetrahedron_neighbor(t,f,n);
                    m_queue_i_0.push(n);
                    if(m_u_set_i_0.end() != m_u_set_i_0.find(t+f))
                    {
                        m_u_set_i_0.erase(t+f);
                    }
                    else
                    {
                        m_u_set_i_0.insert(m_neighbors[t+f]);
                    }
                }
                m_u_set_i_2.insert(t);
            }
            for(uint32_t t : m_u_set_i_2)
            {
                remove_tetrahedron(t);
            }
        }
        
        // tetrahedralize hole
        // m_map_ii_i_0 stores newly created triangles that do not have neighbors
        {
            m_map_ii_i_0.clear();
            for(uint32_t tri : m_u_set_i_0)
            {
                uint32_t f0,f1,f2;
                get_tetrahedron_face(tri,f0,f1,f2);
                uint32_t nt = add_tetrahedron(m_vector_i_0[i],f0,f1,f2);
                origin_t = nt; // used for searching the next point
                m_neighbors[nt+3] = tri;
                m_neighbors[tri] = nt+3;

                tetrahedralize_hole_helper(f0,f1,nt+0);
                tetrahedralize_hole_helper(f0,f2,nt+1);
                tetrahedralize_hole_helper(f1,f2,nt+2);
            }
        }
    }
}


int in_sphere(uint32_t t,uint32_t p)
{
    int res;
    if(INFINITE_VERTEX != m_tetrahedrons[t+3])
    {
        res = in_sphere(m_tetrahedrons[t+0],m_tetrahedrons[t+1],m_tetrahedrons[t+2],m_tetrahedrons[t+3],p);
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

    res = orient3d(m_tetrahedrons[t+0],m_tetrahedrons[t+1],m_tetrahedrons[t+2],p);
    if(0 != res)
    {
        return res;
    }
    uint32_t opposite_p;
    get_tetrahedron_opposite_vertex(m_neighbors[t+0],opposite_p);
    
    res = in_sphere(m_tetrahedrons[t+0],m_tetrahedrons[t+1],m_tetrahedrons[t+2],opposite_p,p);
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

int symbolic_perturbation(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3,uint32_t p4)
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

    int oriA = orient3d(indices[1], indices[2], indices[3], indices[4]);
    if (oriA != 0)
    {
        if((swaps % 2) != 0)
        {
            oriA = -oriA;
        }
        return oriA;
    }

    int oriB = -orient3d(indices[0], indices[2], indices[3], indices[4]);
    if ((swaps % 2) != 0)
    {
        oriB = -oriB;
    }
    return oriB;
}


uint32_t add_tetrahedron(uint32_t p0,uint32_t p1,uint32_t p2,uint32_t p3)
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

void remove_tetrahedron(uint32_t t)
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

void tetrahedralize_hole_helper(uint32_t p0, uint32_t p1, uint32_t t)
{
    sort_ints(p0,p1);
    map<pair<uint32_t,uint32_t>,uint32_t>::iterator it;
    if(m_map_ii_i_0.end() != (it=m_map_ii_i_0.find({p0,p1})))
    {
        uint32_t n = it->second;
        m_neighbors[t] = n;
        m_neighbors[n] = t;
        m_map_ii_i_0.erase(it);
    }
    else
    {
        m_map_ii_i_0[{p0,p1}] = t;
    }
}
