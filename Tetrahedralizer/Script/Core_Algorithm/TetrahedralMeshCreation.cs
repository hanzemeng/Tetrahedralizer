using System;
using System.Linq;
using System.Collections.Generic;
using System.Threading.Tasks;
using UnityEngine;
using UnityEngine.Rendering;


namespace Hanzzz.Tetrahedralizer
{
    public class TetrahedralMeshCreation
    {
        public class TetrahedralMeshCreationInput
        {
            public Mesh m_mesh;
            public Polyhedralization m_polyhedralization;
        }
        public class TetrahedralMeshCreationOutput
        {
            public TetrahedralMesh m_tetrahedralMesh;
        }
    
        public void Create(TetrahedralMeshCreationInput input, TetrahedralMeshCreationOutput output)
        {
            List<Vector3> weldedVertices = input.m_mesh.vertices.ToList();
            int[] weldedTriangles = input.m_mesh.triangles;
            TetrahedralizerUtility.RemoveDuplicateVertices(weldedVertices, weldedTriangles);
            Vector3[] vertices = input.m_mesh.vertices;
            int[] triangles = input.m_mesh.triangles;
            int[] uv0IslandsIndexes = input.m_mesh.GetUV0IslandsIndexes();
    
            List<SubMeshDescriptor> subMeshDescriptors = Enumerable.Range(0,input.m_mesh.subMeshCount).Select(i=>input.m_mesh.GetSubMesh(i)).ToList();
    
            using MeshVertexDataMapper meshVertexDataMapper = new MeshVertexDataMapper();
            meshVertexDataMapper.AssignSourceMesh(input.m_mesh);
    
            TetrahedralMesh tetrahedralMesh = ScriptableObject.CreateInstance<TetrahedralMesh>();
            output.m_tetrahedralMesh = tetrahedralMesh;
    
            CreateInternal(vertices, triangles, weldedTriangles, uv0IslandsIndexes, subMeshDescriptors, input.m_polyhedralization, meshVertexDataMapper, tetrahedralMesh, null);
        }
    
        public Task CreateAsync(TetrahedralMeshCreationInput input, TetrahedralMeshCreationOutput output, IProgress<string> progress=null)
        {
            if(null != progress)
            {
                progress.Report("Starting.");
            }
            List<Vector3> weldedVertices = input.m_mesh.vertices.ToList();
            int[] weldedTriangles = input.m_mesh.triangles;
            TetrahedralizerUtility.RemoveDuplicateVertices(weldedVertices, weldedTriangles);
            Vector3[] vertices = input.m_mesh.vertices;
            int[] triangles = input.m_mesh.triangles;
            int[] uv0IslandsIndexes = input.m_mesh.GetUV0IslandsIndexes();
    
            List<SubMeshDescriptor> subMeshDescriptors = Enumerable.Range(0,input.m_mesh.subMeshCount).Select(i=>input.m_mesh.GetSubMesh(i)).ToList();
    
            MeshVertexDataMapper meshVertexDataMapper = new MeshVertexDataMapper();
            meshVertexDataMapper.AssignSourceMesh(input.m_mesh);
    
            TetrahedralMesh tetrahedralMesh = ScriptableObject.CreateInstance<TetrahedralMesh>();
            output.m_tetrahedralMesh = tetrahedralMesh;
    
            return Task.Run(() =>
            {
                CreateInternal(vertices, triangles, weldedTriangles, uv0IslandsIndexes, subMeshDescriptors, input.m_polyhedralization, meshVertexDataMapper, tetrahedralMesh, progress);
            });
        }
    
        private void CreateInternal(
            Vector3[] vertices, int[] triangles, int[] weldedTriangles, int[] uv0IslandsIndexes, List<SubMeshDescriptor> subMeshDescriptors,
            Polyhedralization polyhedralization, MeshVertexDataMapper meshVertexDataMapper, TetrahedralMesh tetrahedralMesh, IProgress<string> progress=null)
        {
            List<Vector3> approximatedVertices = TetrahedralizerUtility.PackVector3s(GenericPointApproximation.CalculateGenericPointApproximation(polyhedralization.m_explicitVertices, polyhedralization.m_implicitVertices));
            List<List<int>> nestedPolyhedrons = TetrahedralizerUtility.FlatIListToNestedList(polyhedralization.m_polyhedrons);
            List<List<int>> nestedFacets = TetrahedralizerUtility.FlatIListToNestedList(polyhedralization.m_facets);
            List<List<List<int>>> facetsAssociations = nestedFacets.Select(i=>i.Select(j=>new List<int>()).ToList()).ToList();
            {
                int index = 0;
                for(int i=0; i<nestedFacets.Count; i++)
                {
                    for(int j=0; j<nestedFacets[i].Count; j++)
                    {
                        int n = polyhedralization.m_facetsVerticesMapping[index++];
                        for(int k=0; k<n; k++)
                        {
                            facetsAssociations[i][j].Add(polyhedralization.m_facetsVerticesMapping[index++]);
                        }
                    }
                }
            }
            
            if(null != progress)
            {
                progress.Report("Tetrahedralizing polyhedrons.");
            }
            List<int> tetrahedrons;
            {
                PolyhedralizationTetrahedralization.PolyhedralizationTetrahedralizationOutput PTOutput = new PolyhedralizationTetrahedralization.PolyhedralizationTetrahedralizationOutput();
                PolyhedralizationTetrahedralization.PolyhedralizationTetrahedralizationInput PTInput = new PolyhedralizationTetrahedralization.PolyhedralizationTetrahedralizationInput();
                PolyhedralizationTetrahedralization polyhedralizationTetrahedralization = new PolyhedralizationTetrahedralization();
                PTInput.m_explicitVertices = polyhedralization.m_explicitVertices;
                PTInput.m_implicitVertices = polyhedralization.m_implicitVertices;
                PTInput.m_polyhedrons = polyhedralization.m_polyhedrons;
                PTInput.m_facets = polyhedralization.m_facets;
                PTInput.m_facetsCentroids = polyhedralization.m_facetsCentroids;
                PTInput.m_facetsCentroidsWeights = polyhedralization.m_facetsCentroidsWeights;
                polyhedralizationTetrahedralization.CalculatePolyhedralizationTetrahedralization(PTInput, PTOutput);
                //Debug.Log(PTOutput.m_insertedFacetsCentroids.Count);
                //Debug.Log(PTOutput.m_insertedPolyhedronsCentroids.Count);
                Vector3[] polyhedronsFacetsCentroids = Enumerable.Repeat(Vector3.zero, nestedFacets.Count).ToArray();
                Vector3 ApproximateCentroid(int f)
                {
                    if(Vector3.zero != polyhedronsFacetsCentroids[f])
                    {
                        return polyhedronsFacetsCentroids[f];
                    }
                    float w0 = (float)polyhedralization.m_facetsCentroidsWeights[2*f+0];
                    float w1 = (float)polyhedralization.m_facetsCentroidsWeights[2*f+1];
                    float w2 = 1f-w0-w1;
                    Vector3 center = w0 * approximatedVertices[polyhedralization.m_facetsCentroids[3*f+0]] +
                                     w1 * approximatedVertices[polyhedralization.m_facetsCentroids[3*f+1]] +
                                     w2 * approximatedVertices[polyhedralization.m_facetsCentroids[3*f+2]];
                    polyhedronsFacetsCentroids[f] = center;
                    return polyhedronsFacetsCentroids[f];
                }
    
                foreach(int f in PTOutput.m_insertedFacetsCentroids)
                {
                    int i = approximatedVertices.Count;
                    approximatedVertices.Add(ApproximateCentroid(f));
                    nestedFacets[f].Add(i);
                    if(TetrahedralizerConstant.UNDEFINED_VALUE != polyhedralization.m_facetsCentroidsMapping[f])
                    {
                        facetsAssociations[f].Add(new List<int>{polyhedralization.m_facetsCentroidsMapping[f]});
                    }
                    else
                    {
                        facetsAssociations[f].Add(new List<int>());
                    }
                }
                foreach(int p in PTOutput.m_insertedPolyhedronsCentroids)
                {
                    Vector3 center = nestedPolyhedrons[p].Aggregate(Vector3.zero,(i,j)=>i+=ApproximateCentroid(j)) / (float)nestedPolyhedrons[p].Count;
                    approximatedVertices.Add(center);
                }
                tetrahedrons = PTOutput.m_tetrahedrons;
            }
            
            if(null != progress)
            {
                progress.Report("Remapping vertex data.");
            }
            int originalSubmeshesCount = subMeshDescriptors.Count;
            List<int> originalTrianglesSubmeshes = Enumerable.Range(0,triangles.Count()/3).Select(i=>originalSubmeshesCount).ToList();
            List<int> resultTrianglesSubmeshes = new List<int>();
            for(int i=0; i<originalSubmeshesCount; i++)
            {
                SubMeshDescriptor subMeshDescriptor = subMeshDescriptors[i];
                for(int j=subMeshDescriptor.indexStart; j<subMeshDescriptor.indexStart+subMeshDescriptor.indexCount; j+=3)
                {
                    originalTrianglesSubmeshes[j/3] = i;
                }
            }
    
            List<int> neighbors = Enumerable.Repeat(TetrahedralizerConstant.UNDEFINED_VALUE, tetrahedrons.Count).ToList();
            Dictionary<(int,int,int),int> neighborsRecords = new Dictionary<(int, int, int), int>();
            List<List<int>> verticesIncidentFacets = Enumerable.Range(0,approximatedVertices.Count).Select(i=>new List<int>()).ToList();
            for(int i=0; i<nestedFacets.Count; i++)
            {
                for(int j=0; j<nestedFacets[i].Count; j++)
                {
                    verticesIncidentFacets[nestedFacets[i][j]].Add(i);
                }
            }
    
            int[] uv0IslandsFrequency = new int[uv0IslandsIndexes.Max()+1];
            int[] ps = new int[3];
            double[] ts = new double[3];
            // returns the triangle that contains the facet vertex
            int MapVertexData(int facet, int vertex)
            {
                int t = -1;
                for(int i=0; i<facetsAssociations[facet][vertex].Count; i++)
                {
                    int uvIsland = uv0IslandsIndexes[facetsAssociations[facet][vertex][i]];
                    if(uv0IslandsFrequency[uvIsland] != facetsAssociations[facet].Count)
                    {
                        continue;
                    }
                    if(t<0)
                    {
                        t = facetsAssociations[facet][vertex][i];
                        continue;
                    }
                    if(uv0IslandsIndexes[t] == uvIsland)
                    {
                        continue;
                    }
                    
                    // if the another triangle is on a different uv island, then use the uv island incidented by the facet centroid
                    if(TetrahedralizerConstant.UNDEFINED_VALUE == polyhedralization.m_facetsCentroidsMapping[facet] || uvIsland != uv0IslandsIndexes[polyhedralization.m_facetsCentroidsMapping[facet]])
                    {
                        continue;
                    }
                    t = facetsAssociations[facet][vertex][i];
                }
    
                if(t < 0)
                {
                    meshVertexDataMapper.AddDefaultValue(approximatedVertices[nestedFacets[facet][vertex]]);
                }
                else
                {
                    ps[0] = triangles[3*t+0];
                    ps[1] = triangles[3*t+1];
                    ps[2] = triangles[3*t+2];
                    TetrahedralizerUtility.BarycentricWeight(vertices[ps[0]],vertices[ps[1]],vertices[ps[2]],approximatedVertices[nestedFacets[facet][vertex]],out ts[0],out ts[1],out ts[2]);
                    meshVertexDataMapper.InterpolateVertexData(3, approximatedVertices[nestedFacets[facet][vertex]],ps,ts);
                }
                
                return t;
            }
            void ProcessFacet(int index, int p0, int p1, int p2)
            {
                {
                    int t0 = p0;
                    int t1 = p1;
                    int t2 = p2;
                    Sorter.SortInt3(ref t0, ref t1, ref t2);
                    if(neighborsRecords.TryGetValue((t0,t1,t2), out int n))
                    {
                        neighbors[index] = n;
                        neighbors[n] = index;
                        neighborsRecords.Remove((t0,t1,t2));
                    }
                    else
                    {
                        neighborsRecords[(t0,t1,t2)] = index;
                    }
                }
    
                {
                    int facet = TetrahedralizerConstant.UNDEFINED_VALUE;
                    foreach(int f in verticesIncidentFacets[p0])
                    {
                        if(verticesIncidentFacets[p1].Contains(f) && verticesIncidentFacets[p2].Contains(f))
                        {
                            facet = f;
                            break;
                        }
                    }
                    if(TetrahedralizerConstant.UNDEFINED_VALUE == facet)
                    {
                        meshVertexDataMapper.AddDefaultValue(approximatedVertices[p0]);
                        meshVertexDataMapper.AddDefaultValue(approximatedVertices[p1]);
                        meshVertexDataMapper.AddDefaultValue(approximatedVertices[p2]);
                        resultTrianglesSubmeshes.Add(originalSubmeshesCount);
                        return;
                    }
                    //if(1 != commonFacet.Count)
                    //{
                    //    throw new Exception();
                    //}
                    Array.Clear(uv0IslandsFrequency, 0, uv0IslandsFrequency.Length);
                    for(int i=0; i<uv0IslandsFrequency.Length; i++)
                    {
                        int sum = 0;
                        for(int j=0; j<facetsAssociations[facet].Count; j++)
                        {
                            for(int k=0; k<facetsAssociations[facet][j].Count; k++)
                            {
                                if(i == uv0IslandsIndexes[facetsAssociations[facet][j][k]])
                                {
                                    sum++;
                                    break;
                                }
                            }
                        }
                        uv0IslandsFrequency[i] = sum;
                    }
    
                    int t0 = MapVertexData(facet, nestedFacets[facet].FindIndex(i=>i==p0));
                    int t1 = MapVertexData(facet, nestedFacets[facet].FindIndex(i=>i==p1));
                    int t2 = MapVertexData(facet, nestedFacets[facet].FindIndex(i=>i==p2));
    
                    if(t0 < 0 || t1 < 0 || t2 < 0)
                    {
                        resultTrianglesSubmeshes.Add(originalSubmeshesCount);
                    }
                    else
                    {
                        //if(originalTrianglesSubmeshes[t0] != originalTrianglesSubmeshes[t1] || originalTrianglesSubmeshes[t0] != originalTrianglesSubmeshes[t2] || originalTrianglesSubmeshes[t1]!=originalTrianglesSubmeshes[t2])
                        //{
                        //    throw new Exception();
                        //}
                        resultTrianglesSubmeshes.Add(originalTrianglesSubmeshes[t0]);
                    }
                }
            }
    
            for(int i=0; i<tetrahedrons.Count; i+=4)
            {
                int t0 = tetrahedrons[i+0];
                int t1 = tetrahedrons[i+1];
                int t2 = tetrahedrons[i+2];
                int t3 = tetrahedrons[i+3];
    
                ProcessFacet(i+0,t0,t2,t1);
                ProcessFacet(i+1,t0,t1,t3);
                ProcessFacet(i+2,t0,t3,t2);
                ProcessFacet(i+3,t1,t2,t3);
            }
            
            meshVertexDataMapper.MakeTetrahedralMesh(tetrahedralMesh);
            tetrahedralMesh.SetFacetsSubmeshes(resultTrianglesSubmeshes);
            tetrahedralMesh.SetNeighbors(neighbors);
    
            meshVertexDataMapper.Dispose();
            if(null != progress)
            {
                progress.Report("Finishing up.");
            }
        }
    }
    
}