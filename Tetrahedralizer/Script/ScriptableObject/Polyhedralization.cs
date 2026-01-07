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
        public List<Facet> m_facets;
        public List<Segment> m_segments;

        public List<int> m_facetsCentroidsMapping; // for every facet centroid, record an incident constraint, UNDEFINED_VALUE if no such constraint
        public List<bool> m_facetsPointOut; // only defined for exterior facets, true if the facet points out of its polyhedron
    
    
        public List<(Mesh mesh, Vector3 center)> ToMeshes()
        {
            if(null == m_explicitVertices || 0 == m_explicitVertices.Count)
            {
                return null;
            }
    
            using GenericPointPredicate genericPointPredicate = new GenericPointPredicate(m_explicitVertices,m_implicitVertices);
            GenericPointApproximation genericPointApproximation = new GenericPointApproximation();
            List<Vector3> vertices = TetrahedralizerUtility.PackVector3s(genericPointApproximation.CalculateGenericPointApproximation(m_explicitVertices, m_implicitVertices));
            List<List<int>> polyhedrons = TetrahedralizerUtility.FlatIListToNestedList(m_polyhedrons);
            List<List<int>> polyhedronsFacets = GetFacetsVertices();

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
    
            GenericPointApproximation genericPointApproximation = new GenericPointApproximation();
            List<Vector3> vertices = TetrahedralizerUtility.PackVector3s(genericPointApproximation.CalculateGenericPointApproximation(m_explicitVertices, m_implicitVertices));
            List<List<int>> polyhedrons = TetrahedralizerUtility.FlatIListToNestedList(m_polyhedrons);
            List<List<int>> polyhedronsFacets = GetFacetsVertices();
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

        public int GetVerticesCount()
        {
            return m_explicitVertices.Count/3 + TetrahedralizerUtility.CountFlatIListElements(m_implicitVertices);
        }
        public int GetPolyhedronsCount()
        {
            return TetrahedralizerUtility.CountFlatIListElements(m_polyhedrons);
        }
    
        public void CalculateFacetsOrients()
        {
            using GenericPointPredicate genericPointPredicate = new GenericPointPredicate(m_explicitVertices, m_implicitVertices);
            List<List<int>> polyhedrons = TetrahedralizerUtility.FlatIListToNestedList(m_polyhedrons);
            List<List<int>> polyhedronsFacets = GetFacetsVertices();
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

        public void CalculateFacetsIncidentPolyhedrons()
        {
            List<int> facetsIncident = Enumerable.Repeat(TetrahedralizerConstant.UNDEFINED_VALUE, 2*m_facets.Count).ToList();
            List<List<int>> polyhedrons = TetrahedralizerUtility.FlatIListToNestedList(m_polyhedrons);
            for(int i=0; i<polyhedrons.Count; i++)
            {
                List<int> polyhedron = polyhedrons[i];
                foreach(int f in polyhedron)
                {
                    if(TetrahedralizerConstant.UNDEFINED_VALUE == facetsIncident[2*f+0])
                    {
                        facetsIncident[2*f+0] = i;
                    }
                    else
                    {
                        facetsIncident[2*f+1] = i;
                    }
                }
            }
            for(int i=0; i<facetsIncident.Count/2; i++)
            {
                m_facets[i].ip0 = facetsIncident[2*i+0];
                m_facets[i].ip1 = facetsIncident[2*i+1];
            }
        }
    
        public void RemoveUnusedData(bool removeExplicitVertices)
        {
            // remove facets
            {
                List<List<int>> polyhedrons = TetrahedralizerUtility.FlatIListToNestedList(m_polyhedrons);
                List<bool> usedFacets = Enumerable.Repeat(false,m_facets.Count).ToList();
                polyhedrons.ForEach(i=>i.ForEach(j=>usedFacets[j]=true));
                List<Facet> newFacets = new List<Facet>();
                List<int> newFacetsCentroidsMapping = new List<int>();
                int[] mapping = new int[m_facets.Count];
                for(int i=0; i<m_facets.Count; i++)
                {
                    if(!usedFacets[i])
                    {
                        continue;
                    }
                    mapping[i] = newFacets.Count;
                    newFacets.Add(m_facets[i]);
                    newFacetsCentroidsMapping.Add(m_facetsCentroidsMapping[i]);
                }
                foreach(List<int> polyhedron in polyhedrons)
                {
                    for(int i=0; i<polyhedron.Count; i++)
                    {
                        polyhedron[i] = mapping[polyhedron[i]];
                    }
                }
                m_polyhedrons = TetrahedralizerUtility.NestedListToFlatList(polyhedrons);
                m_facets = newFacets;
                m_facetsCentroidsMapping = newFacetsCentroidsMapping;
            }
            // remove segments
            {
                List<bool> usedSegments = Enumerable.Repeat(false,m_segments.Count).ToList();
                m_facets.ForEach(i=>i.segments.ForEach(j=>usedSegments[j]=true));
                List<Segment> newSegments = new List<Segment>();
                int[] mapping = new int[m_segments.Count];
                for(int i=0; i<m_segments.Count; i++)
                {
                    if(!usedSegments[i])
                    {
                        continue;
                    }
                    mapping[i] = newSegments.Count;
                    newSegments.Add(m_segments[i]);
                }
                foreach(Facet facet in m_facets)
                {
                    for(int i=0; i<facet.segments.Count; i++)
                    {
                        facet.segments[i] = mapping[facet.segments[i]];
                    }
                }
                m_segments = newSegments;
            }
            //remove implicit vertices
            {
                int explicitCount = m_explicitVertices.Count/3;
                List<List<int>> implicitVertices = TetrahedralizerUtility.FlatIListToNestedList(m_implicitVertices);
                List<bool> usedImplicitVertices = Enumerable.Repeat(false,implicitVertices.Count).ToList();
                List<List<int>> newImplicitVertices = new List<List<int>>();
                int[] mapping = new int[implicitVertices.Count];
                foreach(Segment segment in m_segments)
                {
                    if(segment.e0>=explicitCount)
                    {
                        usedImplicitVertices[segment.e0-explicitCount] = true;
                    }
                    if(segment.e1>=explicitCount)
                    {
                        usedImplicitVertices[segment.e1-explicitCount] = true;
                    }
                }

                for(int i=0; i<implicitVertices.Count; i++)
                {
                    if(!usedImplicitVertices[i])
                    {
                        continue;
                    }
                    mapping[i] = newImplicitVertices.Count;
                    newImplicitVertices.Add(implicitVertices[i]);
                }
                foreach(Segment segment in m_segments)
                {
                    if(segment.e0>=explicitCount)
                    {
                        segment.e0 = mapping[segment.e0-explicitCount] + explicitCount;
                    }
                    if(segment.e1>=explicitCount)
                    {
                        segment.e1 = mapping[segment.e1-explicitCount] + explicitCount;
                    }
                }
                m_implicitVertices = TetrahedralizerUtility.NestedListToFlatList(newImplicitVertices);
            }
            if(!removeExplicitVertices)
            {
                return;
            }
            //remove explicit vertices
            {
                int explicitCount = m_explicitVertices.Count/3;
                List<bool> usedExplicitVertices = Enumerable.Repeat(false,m_explicitVertices.Count/3).ToList();
                List<double> newExplicitVertices = new List<double>();
                int[] mapping = new int[m_explicitVertices.Count/3];

                foreach(Facet facet in m_facets)
                {
                    usedExplicitVertices[facet.p0] = true;
                    usedExplicitVertices[facet.p1] = true;
                    usedExplicitVertices[facet.p2] = true;
                }
                foreach(Segment segment in m_segments)
                {
                    if(segment.e0<explicitCount)
                    {
                        usedExplicitVertices[segment.e0] = true;
                    }
                    if(segment.e1<explicitCount)
                    {
                        usedExplicitVertices[segment.e1] = true;
                    }
                    usedExplicitVertices[segment.p0] = true;
                    usedExplicitVertices[segment.p1] = true;
                    if(TetrahedralizerConstant.UNDEFINED_VALUE != segment.p2)
                    {
                        usedExplicitVertices[segment.p2] = true;
                        usedExplicitVertices[segment.p3] = true;
                        usedExplicitVertices[segment.p4] = true;
                        usedExplicitVertices[segment.p5] = true;
                    }
                }
                List<List<int>> implicitVertices = TetrahedralizerUtility.FlatIListToNestedList(m_implicitVertices);
                implicitVertices.ForEach(i=>i.ForEach(j=>usedExplicitVertices[j]=true));

                for(int i=0; i<explicitCount; i++)
                {
                    if(!usedExplicitVertices[i])
                    {
                        continue;
                    }
                    mapping[i] = newExplicitVertices.Count/3;
                    newExplicitVertices.Add(m_explicitVertices[3*i+0]);
                    newExplicitVertices.Add(m_explicitVertices[3*i+1]);
                    newExplicitVertices.Add(m_explicitVertices[3*i+2]);
                }

                int implicitVerticesDiff = explicitCount-newExplicitVertices.Count/3;
                foreach(Facet facet in m_facets)
                {
                    facet.p0 = mapping[facet.p0];
                    facet.p1 = mapping[facet.p1];
                    facet.p2 = mapping[facet.p2];
                }
                foreach(Segment segment in m_segments)
                {
                    if(segment.e0<explicitCount)
                    {
                        segment.e0 = mapping[segment.e0];
                    }
                    else
                    {
                        segment.e0 -= implicitVerticesDiff;
                    }
                    if(segment.e1<explicitCount)
                    {
                        segment.e1 = mapping[segment.e1];
                    }
                    else
                    {
                        segment.e1 -= implicitVerticesDiff;
                    }
                    segment.p0 = mapping[segment.p0];
                    segment.p1 = mapping[segment.p1];
                    if(TetrahedralizerConstant.UNDEFINED_VALUE != segment.p2)
                    {
                        segment.p2 = mapping[segment.p2];
                        segment.p3 = mapping[segment.p3];
                        segment.p4 = mapping[segment.p4];
                        segment.p5 = mapping[segment.p5];
                    }
                }
                
                foreach(List<int> implicitVertex in implicitVertices)
                {
                    for(int i=0; i<implicitVertex.Count; i++)
                    {
                        implicitVertex[i] = mapping[implicitVertex[i]];
                    }
                }
                m_implicitVertices = TetrahedralizerUtility.NestedListToFlatList(implicitVertices);
                m_explicitVertices = newExplicitVertices;
            }
        }
        public void Assign(Polyhedralization polyhedralization)
        {
            m_explicitVertices = polyhedralization.m_explicitVertices;
            m_implicitVertices = polyhedralization.m_implicitVertices;
            m_polyhedrons = polyhedralization.m_polyhedrons;
            m_facets = polyhedralization.m_facets;
            m_segments = polyhedralization.m_segments;
            m_facetsCentroidsMapping = polyhedralization.m_facetsCentroidsMapping;
            m_facetsPointOut = polyhedralization.m_facetsPointOut;
        }
    
        private List<List<int>> GetFacetsVertices()
        {
            List<List<int>> res = new List<List<int>>();
            List<Segment> segments = new List<Segment>();
            foreach(Facet facet in m_facets)
            {
                segments.Clear();
                foreach(int s in facet.segments)
                {
                    segments.Add(m_segments[s]);
                }
                Segment.SortSegments(segments);
                res.Add(Segment.GetSegmentsVertices(segments));
            }
            return res;
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
            EditorGUILayout.LabelField($"Facets Count: {m_so.m_facets.Count}");
        }
    }
    #endif
    
}