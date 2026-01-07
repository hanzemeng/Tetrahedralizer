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
        /// <summary>
        /// Calculates a tetrahedral mesh from a mesh.
        /// </summary>
        /// <param name="mesh">A mesh to be tetrahedralized.</param>
        /// <param name="polyhedralization">The corresponding polyhedralization of the mesh.</param>
        /// <returns>
        /// A tetrahedral mesh.
        /// </returns>
        public TetrahedralMesh Create(Mesh mesh, Polyhedralization polyhedralization)
        {
            var syncRes = CreateInternalSync(mesh);
    
            CreateInternal(syncRes.vertices, syncRes.triangles, syncRes.uv0IslandsIndexes, syncRes.subMeshDescriptors, polyhedralization, syncRes.meshVertexDataMapper, syncRes.tetrahedralMesh);
            return syncRes.tetrahedralMesh;
        }
        /// <summary>
        /// Calculates a tetrahedral mesh from a mesh.
        /// </summary>
        /// <param name="mesh">A mesh to be tetrahedralized.</param>
        /// <param name="polyhedralization">The corresponding polyhedralization of the mesh.</param>
        /// <returns>
        /// A tetrahedral mesh.
        /// </returns>
        public Task<TetrahedralMesh> CreateAsync(Mesh mesh, Polyhedralization polyhedralization, IProgress<string> progress=null)
        {
            progress?.Report("Starting.");
            var syncRes = CreateInternalSync(mesh);
    
            return Task.Run(() =>
            {
                CreateInternal(syncRes.vertices, syncRes.triangles, syncRes.uv0IslandsIndexes, syncRes.subMeshDescriptors, polyhedralization, syncRes.meshVertexDataMapper, syncRes.tetrahedralMesh, progress);
                return syncRes.tetrahedralMesh;
            });
        }
    
        private (Vector3[] vertices, int[] triangles, int[] uv0IslandsIndexes, List<SubMeshDescriptor> subMeshDescriptors, MeshVertexDataMapper meshVertexDataMapper, TetrahedralMesh tetrahedralMesh)
        CreateInternalSync(Mesh mesh)
        {
            Vector3[] vertices = mesh.vertices;
            int[] triangles = mesh.triangles;
            int[] uv0IslandsIndexes = mesh.GetUV0IslandsIndexes();
    
            List<SubMeshDescriptor> subMeshDescriptors = Enumerable.Range(0,mesh.subMeshCount).Select(i=>mesh.GetSubMesh(i)).ToList();
            MeshVertexDataMapper meshVertexDataMapper = new MeshVertexDataMapper();
            meshVertexDataMapper.AssignSourceMesh(mesh);
    
            TetrahedralMesh tetrahedralMesh = ScriptableObject.CreateInstance<TetrahedralMesh>();

            return (vertices, triangles, uv0IslandsIndexes, subMeshDescriptors, meshVertexDataMapper, tetrahedralMesh);
        }
        private void CreateInternal(
            Vector3[] vertices, int[] triangles, int[] uv0IslandsIndexes, List<SubMeshDescriptor> subMeshDescriptors,
            Polyhedralization polyhedralization, MeshVertexDataMapper meshVertexDataMapper, TetrahedralMesh tetrahedralMesh, IProgress<string> progress=null)
        {
            GenericPointApproximation genericPointApproximation = new GenericPointApproximation();
            List<Vector3> approximatedVertices = TetrahedralizerUtility.PackVector3s(genericPointApproximation.CalculateGenericPointApproximation(polyhedralization.m_explicitVertices, polyhedralization.m_implicitVertices));
            List<List<int>> nestedPolyhedrons = TetrahedralizerUtility.FlatIListToNestedList(polyhedralization.m_polyhedrons);
            List<List<int>> verticesIncidentFacets = Enumerable.Range(0,approximatedVertices.Count).Select(i=>new List<int>()).ToList();
            
            {
                HashSet<int> facetVertices = new HashSet<int>();
                for(int i=0; i<polyhedralization.m_facets.Count; i++)
                {
                    facetVertices.Clear();
                    foreach(int s in polyhedralization.m_facets[i].segments)
                    {
                        facetVertices.Add(polyhedralization.m_segments[s].e0);
                        facetVertices.Add(polyhedralization.m_segments[s].e1);
                    }
                    foreach(int v in facetVertices)
                    {
                        verticesIncidentFacets[v].Add(i);
                    }
                }
            }

            progress?.Report("Tetrahedralizing polyhedrons.");
            List<int> tetrahedrons;
            {
                PolyhedralizationTetrahedralization polyhedralizationTetrahedralization = new PolyhedralizationTetrahedralization();
                var ptRes = polyhedralizationTetrahedralization.CalculatePolyhedralizationTetrahedralization(polyhedralization.m_explicitVertices,polyhedralization.m_implicitVertices,
                polyhedralization.m_polyhedrons,polyhedralization.m_facets,polyhedralization.m_segments);
                foreach(int f in ptRes.insertedFacetsCentroids)
                {
                    double w0 = polyhedralization.m_facets[f].w0;
                    double w1 = polyhedralization.m_facets[f].w1;
                    double w2 = 1d-w1-w0;
                    Vector3 center = (float)w0*approximatedVertices[polyhedralization.m_facets[f].p0] + 
                                     (float)w1*approximatedVertices[polyhedralization.m_facets[f].p1] + 
                                     (float)w2*approximatedVertices[polyhedralization.m_facets[f].p2];
                    approximatedVertices.Add(center);
                    verticesIncidentFacets.Add(new List<int>{f});
                }

                HashSet<int> polyhedronVertices = new HashSet<int>();
                foreach(int p in ptRes.insertedPolyhedronsCentroids)
                {
                    polyhedronVertices.Clear();
                    foreach(int f in nestedPolyhedrons[p])
                    {
                        foreach(int s in polyhedralization.m_facets[f].segments)
                        {
                            polyhedronVertices.Add(polyhedralization.m_segments[s].e0);
                            polyhedronVertices.Add(polyhedralization.m_segments[s].e1);
                        }
                    }

                    Vector3 center = Vector3.zero;
                    foreach(int v in polyhedronVertices)
                    {
                        center += approximatedVertices[v];
                    }
                    center /= (float)polyhedronVertices.Count;
                    approximatedVertices.Add(center);
                    verticesIncidentFacets.Add(new List<int>{TetrahedralizerConstant.UNDEFINED_VALUE});
                }
                tetrahedrons = ptRes.tetrahedrons;
            }
            
            progress?.Report("Remapping vertex data.");
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
    
            int[] ps = new int[3];
            double[] ts = new double[3];
            
            void MapVertexData(int facet, int vertex)
            {
                int t = polyhedralization.m_facetsCentroidsMapping[facet];
                if(t < 0)
                {
                    meshVertexDataMapper.AddDefaultValue(approximatedVertices[vertex]);
                }
                else
                {
                    ps[0] = triangles[3*t+0];
                    ps[1] = triangles[3*t+1];
                    ps[2] = triangles[3*t+2];
                    TetrahedralizerUtility.BarycentricWeight(vertices[ps[0]],vertices[ps[1]],vertices[ps[2]],approximatedVertices[vertex],out ts[0],out ts[1],out ts[2]);
                    meshVertexDataMapper.InterpolateVertexData(3, approximatedVertices[vertex],ps,ts);
                }
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
    
                    MapVertexData(facet, p0);
                    MapVertexData(facet, p1);
                    MapVertexData(facet, p2);
                    int t = polyhedralization.m_facetsCentroidsMapping[facet];
                    if(TetrahedralizerConstant.UNDEFINED_VALUE==t)
                    {
                        resultTrianglesSubmeshes.Add(originalSubmeshesCount);
                    }
                    else
                    {
                        resultTrianglesSubmeshes.Add(originalTrianglesSubmeshes[t]);
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
            progress?.Report("Finishing up.");
        }
    }
    
}