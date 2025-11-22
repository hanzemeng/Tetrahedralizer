using System;
using System.Linq;
using System.Collections.Generic;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

namespace Hanzzz.Tetrahedralizer
{
    [CreateAssetMenu(fileName = nameof(Polyhedralization), menuName = TetrahedralizerConstant.SCRIPTABLE_OBJECT_PATH + nameof(Polyhedralization))]
    [PreferBinarySerialization]
    public class Polyhedralization : ScriptableObject
    {
        public List<double> m_explicitVertices; // Every 3 doubles are x,y,z of a point. Assuming left hand coordinate.
        public List<int> m_implicitVertices; // 5/9 followed by indexes of m_explicitVertices.
        public List<int> m_polyhedrons; // # of polyhedron facets, followed by facets indexes.
        
        public List<int> m_facets; // # of facets vertices, followed by vertices indexes ordered in cw or ccw.
        public List<int> m_facetsCentroids; // every facet centroid is defined by 3 coplanar explicit vertices
        public List<double> m_facetsCentroidsWeights; // and the weight of the explicit vertices, note the 3 weight is ignored
        public List<int> m_facetsVerticesMapping; // for every vertex in every facet, record # of incident constraints followed by indexes of the constraints
        public List<int> m_facetsCentroidsMapping; // for every facet centroid, record an incident constraint, UNDEFINED_VALUE if no such constraint
        public List<bool> m_facetsPointOut; // only defined for exterior facets, true if the facet points out of its polyhedron
    
    
        public List<(Mesh mesh, Vector3 center)> ToMeshes()
        {
            if(null == m_explicitVertices || 0 == m_explicitVertices.Count)
            {
                return null;
            }
    
            using GenericPointPredicate genericPointPredicate = new GenericPointPredicate(m_explicitVertices,m_implicitVertices);
            List<Vector3> vertices = TetrahedralizerUtility.PackVector3s(GenericPointApproximation.CalculateGenericPointApproximation(m_explicitVertices, m_implicitVertices));
            List<List<int>> polyhedrons = TetrahedralizerUtility.FlatIListToNestedList(m_polyhedrons);
            List<List<int>> polyhedronsFacets = TetrahedralizerUtility.FlatIListToNestedList(m_facets);
    
            List<(Mesh, Vector3)> res = new List<(Mesh, Vector3)>();
    
            List<Vector3> polyVertices = new List<Vector3>();
            List<int> polyTriangles = new List<int>();
            int triangleIndex;
            for(int i=0; i<polyhedrons.Count; i++)
            {
                polyVertices.Clear();
                polyTriangles.Clear();
                triangleIndex = 0;
    
                for(int j=0; j<polyhedrons[i].Count; j++)
                {
                    int facet = polyhedrons[i][j];
    
                    for(int k=0; k<polyhedronsFacets[facet].Count; k++)
                    {
                        polyVertices.Add(vertices[polyhedronsFacets[facet][k]]);
                    }
                    if(FacetPointsOut(facet, i, polyhedronsFacets, polyhedrons, genericPointPredicate))
                    {
                        for(int k=1; k<polyhedronsFacets[facet].Count-1; k++)
                        {
                            polyTriangles.Add(triangleIndex);
                            polyTriangles.Add(triangleIndex+k);
                            polyTriangles.Add(triangleIndex+k+1);
                        }
                    }
                    else
                    {
                        for(int k=1; k<polyhedronsFacets[facet].Count-1; k++)
                        {
                            polyTriangles.Add(triangleIndex+k+1);
                            polyTriangles.Add(triangleIndex+k);
                            polyTriangles.Add(triangleIndex);
                        }
                    }
                    triangleIndex += polyhedronsFacets[facet].Count;
                }
    
                Vector3 center = TetrahedralizerUtility.CenterVertices(polyVertices);
                Mesh mesh = new Mesh();
                mesh.vertices = polyVertices.ToArray();
                mesh.triangles = polyTriangles.ToArray();
                mesh.RecalculateBounds();
                mesh.RecalculateNormals();
                mesh.RecalculateTangents();
    
                res.Add((mesh,center));
            }
    
            return res;
        }
        public (Mesh mesh, Vector3 center) ToMesh()
        {
            if(null == m_explicitVertices || 0 == m_explicitVertices.Count)
            {
                return (null, Vector3.zero);
            }
    
            List<Vector3> vertices = TetrahedralizerUtility.PackVector3s(GenericPointApproximation.CalculateGenericPointApproximation(m_explicitVertices, m_implicitVertices));
            List<List<int>> polyhedrons = TetrahedralizerUtility.FlatIListToNestedList(m_polyhedrons);
            List<List<int>> polyhedronsFacets = TetrahedralizerUtility.FlatIListToNestedList(m_facets);
            bool[] shouldDrawFacets = GetFacetsExteriorFlags();
    
            List<Vector3> meshVertices = new List<Vector3>();
            List<List<int>> facetsVerticesIndexes = new List<List<int>>();
            List<int> polyTriangles = new List<int>();
            int triangleIndex = 0;
            for(int i=0; i<polyhedrons.Count; i++)
            {
                for(int j=0; j<polyhedrons[i].Count; j++)
                {
                    int facet = polyhedrons[i][j];
                    if(!shouldDrawFacets[facet])
                    {
                        continue;
                    }
    
                    facetsVerticesIndexes.Add(new List<int>());
                    for(int k=0; k<polyhedronsFacets[facet].Count; k++)
                    {
                        facetsVerticesIndexes[^1].Add(meshVertices.Count);
                        meshVertices.Add(vertices[polyhedronsFacets[facet][k]]);
                    }
                    if(m_facetsPointOut[facet])
                    {
                        for(int k=1; k<polyhedronsFacets[facet].Count-1; k++)
                        {
                            polyTriangles.Add(triangleIndex);
                            polyTriangles.Add(triangleIndex+k);
                            polyTriangles.Add(triangleIndex+k+1);
                        }
                    }
                    else
                    {
                        for(int k=1; k<polyhedronsFacets[facet].Count-1; k++)
                        {
                            polyTriangles.Add(triangleIndex+k+1);
                            polyTriangles.Add(triangleIndex+k);
                            polyTriangles.Add(triangleIndex);
                        }
                    }
                    triangleIndex += polyhedronsFacets[facet].Count;
                }
            }
    
            Vector3 center = Vector3.zero;
            Mesh mesh = new Mesh();
            mesh.indexFormat = UnityEngine.Rendering.IndexFormat.UInt32;
            mesh.vertices = meshVertices.ToArray();
            mesh.triangles = polyTriangles.ToArray();
            mesh.RecalculateBounds();
            mesh.RecalculateNormals();
            mesh.RecalculateTangents();
    
            return (mesh, center);
        }
    
        public List<int> GetExteriorFacets()
        {
            HashSet<int> facets = new HashSet<int>();
            List<List<int>> polyhedrons = TetrahedralizerUtility.FlatIListToNestedList(m_polyhedrons);
            foreach(List<int> ints in polyhedrons)
            {
                foreach(int i in ints)
                {
                    if(facets.Contains(i))
                    {
                        facets.Remove(i);
                    }
                    else
                    {
                        facets.Add(i);
                    }
                }
            }
    
            return facets.ToList();
        }
        public bool[] GetFacetsExteriorFlags()
        {
            bool[] res = Enumerable.Repeat(false, m_facets.Count).ToArray();
            GetExteriorFacets().ForEach(i=>res[i]=true);
            return res;
        }
    
        public void CalculateFacetsOrients()
        {
            using GenericPointPredicate genericPointPredicate = new GenericPointPredicate(m_explicitVertices, m_implicitVertices);
            List<List<int>> polyhedrons = TetrahedralizerUtility.FlatIListToNestedList(m_polyhedrons);
            List<List<int>> polyhedronsFacets = TetrahedralizerUtility.FlatIListToNestedList(m_facets);
            m_facetsPointOut = Enumerable.Repeat(false,polyhedronsFacets.Count).ToList();
            bool[] isExteriorFacets = GetFacetsExteriorFlags();
    
            for(int i=0; i<polyhedrons.Count; i++)
            {
                for(int j=0; j<polyhedrons[i].Count; j++)
                {
                    int facet = polyhedrons[i][j];
                    if(!isExteriorFacets[facet])
                    {
                        continue;
                    }
                    m_facetsPointOut[facet] = FacetPointsOut(facet, i, polyhedronsFacets, polyhedrons, genericPointPredicate);
                }
            }
        }
    
        public int GetVerticesCount()
        {
            return m_explicitVertices.Count/3 + TetrahedralizerUtility.CountFlatIListElements(m_implicitVertices);
        }
        public int GetPolyhedronsCount()
        {
            return TetrahedralizerUtility.CountFlatIListElements(m_polyhedrons);
        }
    
        public void Assign(Polyhedralization polyhedralization)
        {
            m_explicitVertices = polyhedralization.m_explicitVertices;
            m_implicitVertices = polyhedralization.m_implicitVertices;
            m_polyhedrons = polyhedralization.m_polyhedrons;
            m_facets = polyhedralization.m_facets;
            m_facetsCentroids = polyhedralization.m_facetsCentroids;
            m_facetsCentroidsWeights = polyhedralization.m_facetsCentroidsWeights;
            m_facetsVerticesMapping = polyhedralization.m_facetsVerticesMapping;
            m_facetsCentroidsMapping = polyhedralization.m_facetsCentroidsMapping;
            m_facetsPointOut = polyhedralization.m_facetsPointOut;
        }
    
        private bool FacetPointsOut(int facet, int polyhedron, List<List<int>> polyhedronsFacets, List<List<int>> polyhedrons, GenericPointPredicate genericPointPredicate)
        {
            List<int> facetVertices = polyhedronsFacets[facet];
            int p0,p1,p2;
            p0=p1=p2=-1; // find non collinear points
            for(int i=0; i<facetVertices.Count; i++)
            {
                int p = 0==i ? facetVertices.Count-1:i-1;
                int n = facetVertices.Count-1==i ? 0:i+1;
    
                if(!genericPointPredicate.IsCollinear(facetVertices[p],facetVertices[i],facetVertices[n]))
                {
                    p0=facetVertices[p];
                    p1=facetVertices[i];
                    p2=facetVertices[n];
                    break;
                }
            }
            if(p0<0)
            {
                throw new Exception();
            }
    
            for(int i=0; i<polyhedrons[polyhedron].Count; i++)
            {
                if(facet == polyhedrons[polyhedron][i])
                {
                    continue;
                }
    
                int f = polyhedrons[polyhedron][i];
                for(int j=0; j<polyhedronsFacets[f].Count; j++)
                {
                    if(facetVertices.Contains(polyhedronsFacets[f][j]))
                    {
                        continue;
                    }
    
                    int o = genericPointPredicate.Orient3d(p0,p1,p2,polyhedronsFacets[f][j]);
                    if(0==o)
                    {
                        continue;
                    }
                    return o<0;
                }
            }
            throw new Exception();
        }
    }
    
    
    #if UNITY_EDITOR
    [CustomEditor(typeof(Polyhedralization))]
    public class PolyhedralizationEditor : Editor
    {
        private Polyhedralization m_so;
    
    
        private void OnEnable()
        {
            m_so = (Polyhedralization)target;
        }
    
        public override void OnInspectorGUI()
        {
            //base.OnInspectorGUI();
    
            if(null == m_so.m_explicitVertices)
            {
                EditorGUILayout.LabelField($"Vertices Count: {0}");
                EditorGUILayout.LabelField($"Polyhedrons Count: {0}");
                EditorGUILayout.LabelField($"Polyhedrons' Facets Count: {0}");
                return;
            }
    
            EditorGUILayout.LabelField($"Vertices Count: {m_so.m_explicitVertices.Count/3 + TetrahedralizerUtility.CountFlatIListElements(m_so.m_implicitVertices)}");
            EditorGUILayout.LabelField($"Polyhedrons Count: {TetrahedralizerUtility.CountFlatIListElements(m_so.m_polyhedrons)}");
            EditorGUILayout.LabelField($"Facets Count: {TetrahedralizerUtility.CountFlatIListElements(m_so.m_facets)}");
        }
    }
    #endif
    
}