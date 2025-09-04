using System.Collections.Generic;


public class TetrahedralizationPolyhedralization
{
    public class TetrahedralizationPolyhedralizationInput
    {
        public List<int> m_tetrahedrons; // Every 4 ints is a tetrahedron. Curl around the first 3 points and your thumb points toward the 4th point. Assuming left hand coordinate.
    }
    public class TetrahedralizationPolyhedralizationOutput
    {
        public List<int> m_polyhedrons; // # of polyhedron facets, followed by facets indexes.
        public List<int> m_polyhedronsFacets; // # of facets vertices, followed by vertices indexes ordered in cw or ccw.
    }


    public void CalculateTetrahedralizationPolyhedralization(TetrahedralizationPolyhedralizationInput input, TetrahedralizationPolyhedralizationOutput output)
    {
        Dictionary<(int,int,int), int> facetMapping = new Dictionary<(int, int, int), int>();

        void ProcessFacet(int p0, int p1, int p2)
        {
            TetrahedralizerUtility.SortInt3(ref p0, ref p1, ref p2);
            (int,int,int) tuple = (p0,p1,p2);

            int index;
            if(!facetMapping.TryGetValue(tuple, out index))
            {
                facetMapping[tuple] = output.m_polyhedronsFacets.Count / 4;
                index = output.m_polyhedronsFacets.Count / 4;
                output.m_polyhedronsFacets.Add(3);
                output.m_polyhedronsFacets.Add(p0);
                output.m_polyhedronsFacets.Add(p1);
                output.m_polyhedronsFacets.Add(p2);
            }

            output.m_polyhedrons.Add(index);
        }

        output.m_polyhedrons = new List<int>();
        output.m_polyhedronsFacets = new List<int>();
        for(int i=0; i<input.m_tetrahedrons.Count; i+=4)
        {
            int p0 = input.m_tetrahedrons[i+0];
            int p1 = input.m_tetrahedrons[i+1];
            int p2 = input.m_tetrahedrons[i+2];
            int p3 = input.m_tetrahedrons[i+3];

            output.m_polyhedrons.Add(4);
            ProcessFacet(p0,p1,p2);
            ProcessFacet(p1,p0,p3);
            ProcessFacet(p0,p2,p3);
            ProcessFacet(p2,p1,p3);
        }
    }
}
