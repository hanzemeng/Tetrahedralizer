using System;
using System.Linq;
using System.Collections.Generic;
using System.Threading.Tasks;
using Unity.Collections;
using UnityEngine;
using UnityEngine.Rendering;

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

        List<SubMeshDescriptor> subMeshDescriptors = Enumerable.Range(0,input.m_mesh.subMeshCount).Select(i=>input.m_mesh.GetSubMesh(i)).ToList();

        MeshTriangleFinder meshTriangleFinder = new MeshTriangleFinder();
        meshTriangleFinder.AssignSourceMesh(input.m_mesh);
        MeshVertexDataMapper meshVertexDataMapper = new MeshVertexDataMapper();
        meshVertexDataMapper.AssignSourceMesh(input.m_mesh);

        TetrahedralMesh tetrahedralMesh = ScriptableObject.CreateInstance<TetrahedralMesh>();
        output.m_tetrahedralMesh = tetrahedralMesh;

        //CreateInternal(vertices, triangles, subMeshDescriptors, input.m_tetrahedralization.m_tetrahedrons, input.m_tetrahedralization.m_explicitVertices, input.m_tetrahedralization.m_implicitVertices, weldedTriangles, meshTriangleFinder, meshVertexDataMapper, tetrahedralMesh);
    }

    public Task CreateAsync(TetrahedralMeshCreationInput input, TetrahedralMeshCreationOutput output, IProgress<string> progress=null)
    {
        //Debug.Log(1+input.m_mesh.GetUV0IslandIndexes().Max());

        List<Vector3> weldedVertices = input.m_mesh.vertices.ToList();
        int[] weldedTriangles = input.m_mesh.triangles;
        TetrahedralizerUtility.RemoveDuplicateVertices(weldedVertices, weldedTriangles);
        Vector3[] vertices = input.m_mesh.vertices;
        int[] triangles = input.m_mesh.triangles;
        int[] uv0IslandsIndexes = input.m_mesh.GetUV0IslandsIndexes();

        List<SubMeshDescriptor> subMeshDescriptors = Enumerable.Range(0,input.m_mesh.subMeshCount).Select(i=>input.m_mesh.GetSubMesh(i)).ToList();

        MeshTriangleFinder meshTriangleFinder = new MeshTriangleFinder();
        meshTriangleFinder.AssignSourceMesh(input.m_mesh);
        MeshVertexDataMapper meshVertexDataMapper = new MeshVertexDataMapper();
        meshVertexDataMapper.AssignSourceMesh(input.m_mesh);

        TetrahedralMesh tetrahedralMesh = ScriptableObject.CreateInstance<TetrahedralMesh>();
        output.m_tetrahedralMesh = tetrahedralMesh;

        CreateInternal(vertices, triangles, uv0IslandsIndexes, subMeshDescriptors, input.m_polyhedralization.m_polyhedronsFacets, input.m_polyhedralization.m_explicitVertices, input.m_polyhedralization.m_implicitVertices, weldedTriangles, meshVertexDataMapper, tetrahedralMesh, progress);
        //return Task.Run(() =>
        //{
        //    CreateInternal(vertices, triangles, uv0IslandsIndexes, subMeshDescriptors, input.m_polyhedralization.m_polyhedronsFacets, input.m_polyhedralization.m_explicitVertices, input.m_polyhedralization.m_implicitVertices, weldedTriangles, meshVertexDataMapper, tetrahedralMesh, progress);
        //});
        return null;
    }

    private void CreateInternal(Vector3[] vertices, int[] triangles, int[] uv0IslandsIndexes, List<SubMeshDescriptor> subMeshDescriptors, List<int> facets, List<double> explicitVertices, List<int> implicitVertices, int[] weldedTriangles, MeshVertexDataMapper meshVertexDataMapper, TetrahedralMesh tetrahedralMesh, IProgress<string> progress=null)
    {
        if(null != progress)
        {
            progress.Report("Associate facets.");
        }
        List<List<List<int>>> facetsAssociations;
        FacetAssociation.FacetAssociationOutput FAOutput = new FacetAssociation.FacetAssociationOutput();
        {
            FacetAssociation.FacetAssociationInput FAInput = new FacetAssociation.FacetAssociationInput();
            FacetAssociation facetAssociation = new FacetAssociation();
            FAInput.m_explicitVertices = explicitVertices;
            FAInput.m_implicitVertices = implicitVertices;
            FAInput.m_facets = facets;
            FAInput.m_constraints = weldedTriangles;
            facetAssociation.CalculateFacetAssociation(FAInput, FAOutput);

            facetsAssociations = FAOutput.m_facetsVerticeMapping;
        }

        if(null != progress)
        {
            progress.Report("Approximate implicit vertices.");
        }
        List<Vector3> approximatedVertices;
        {
            GenericPointApproximation.GenericPointApproximationOutput GPAOutput = new GenericPointApproximation.GenericPointApproximationOutput();
            GenericPointApproximation.GenericPointApproximationInput GPAInput = new GenericPointApproximation.GenericPointApproximationInput();
            GenericPointApproximation genericPointApproximation = new GenericPointApproximation();
            GPAInput.m_explicitVertices = explicitVertices;
            GPAInput.m_implicitVertices = implicitVertices;
            genericPointApproximation.CalculateGenericPointApproximation(GPAInput, GPAOutput);

            approximatedVertices = TetrahedralizerUtility.PackVector3s(GPAOutput.m_approximatePositions);
        }

        if(null != progress)
        {
            progress.Report("Remap vertex data.");
        }
        int originalSubmeshesCount = subMeshDescriptors.Count;
        List<int> originalTrianglesSubmeshes = Enumerable.Range(0,triangles.Count()/3).Select(i=>originalSubmeshesCount).ToList();
        for(int i=0; i<originalSubmeshesCount; i++)
        {
            SubMeshDescriptor subMeshDescriptor = subMeshDescriptors[i];
            for(int j=subMeshDescriptor.indexStart; j<subMeshDescriptor.indexStart+subMeshDescriptor.indexCount; j+=3)
            {
                originalTrianglesSubmeshes[j/3] = i;
            }
        }

        //List<int> resultTrianglesSubmeshes = new List<int>();

        //int FAOutputIndex = 0;
        
        //void ProcessFacet(int p0, int p1, int p2)
        //{
        //    int c0 = FAOutput.m_tetrahedronsFacetsMapping[FAOutputIndex++];
        //    int c1 = FAOutput.m_tetrahedronsFacetsMapping[FAOutputIndex++];
        //    int c2 = FAOutput.m_tetrahedronsFacetsMapping[FAOutputIndex++];

        //    // vertices not on constraints, find the closest triangle with a good normal alignment
        //    if(TetrahedralizerConstant.UNDEFINED_VALUE == c0 || TetrahedralizerConstant.UNDEFINED_VALUE == c1 || TetrahedralizerConstant.UNDEFINED_VALUE == c2 ||
        //       originalTrianglesSubmeshes[c0] != originalTrianglesSubmeshes[c1] ||
        //       originalTrianglesSubmeshes[c1] != originalTrianglesSubmeshes[c2])
        //    {
        //        Vector3 v0 = approximatedVertices[p0];
        //        Vector3 v1 = approximatedVertices[p1];
        //        Vector3 v2 = approximatedVertices[p2];
        //        Vector3 pointNormal = Vector3.Cross(v1-v0,v2-v1).normalized;
        //        c0 = meshTriangleFinder.FindClosestTriangle(v0, pointNormal, 0.95d);
        //        c1 = meshTriangleFinder.FindClosestTriangle(v1, pointNormal, 0.95d);
        //        c2 = meshTriangleFinder.FindClosestTriangle(v2, pointNormal, 0.95d);
        //    }
        //    // if still has no matching triangle, just add default value
        //    if( c0 < 0 || c1 < 0 || c2 < 0 || 
        //        originalTrianglesSubmeshes[c0] != originalTrianglesSubmeshes[c1] ||
        //        originalTrianglesSubmeshes[c1] != originalTrianglesSubmeshes[c2])
        //    {
        //        meshVertexDataMapper.AddDefaultValue(approximatedVertices[p0]);
        //        meshVertexDataMapper.AddDefaultValue(approximatedVertices[p1]);
        //        meshVertexDataMapper.AddDefaultValue(approximatedVertices[p2]);
        //        resultTrianglesSubmeshes.Add(originalSubmeshesCount);
        //        return;
        //    }

        //    void BarycentricWeight(int t0,int t1,int t2,int p)
        //    {
                
        //    }

        //    BarycentricWeight(triangles[3*c0+0],triangles[3*c0+1],triangles[3*c0+2], p0);
        //    BarycentricWeight(triangles[3*c1+0],triangles[3*c1+1],triangles[3*c1+2], p1);
        //    BarycentricWeight(triangles[3*c2+0],triangles[3*c2+1],triangles[3*c2+2], p2);
        //    resultTrianglesSubmeshes.Add(originalTrianglesSubmeshes[c0]);
        //}

        int[] ps = new int[3];
        double[] ts = new double[3];
        List<List<int>> polyhedronFacets = TetrahedralizerUtility.FlatIListToNestedList(facets);
        int[] uv0IslandsFrequency = new int[uv0IslandsIndexes.Max()+1];
        for(int i=0; i<facetsAssociations.Count; i++)
        {
            Array.Clear(uv0IslandsFrequency, 0, uv0IslandsFrequency.Length);
            facetsAssociations[i].ForEach(j=>j.ForEach(k=>uv0IslandsFrequency[uv0IslandsIndexes[k]]++));

            for(int j=0; j<facetsAssociations[i].Count; j++)
            {
                int t = -1;
                int tMax = -1;
                for(int k=0; k<facetsAssociations[i][j].Count; k++)
                {
                    int uvIsland = uv0IslandsIndexes[facetsAssociations[i][j][k]];
                    if(uv0IslandsFrequency[uvIsland] > tMax && facetsAssociations[i].All(l=>l.Any(m=>uvIsland==uv0IslandsIndexes[m])))
                    {
                        t = facetsAssociations[i][j][k];
                        tMax = uv0IslandsFrequency[uvIsland];
                    }
                }
                if(t < 0)
                {
                    meshVertexDataMapper.AddDefaultValue(approximatedVertices[polyhedronFacets[i][j]]);
                    continue;
                }
                
                ps[0] = triangles[3*t+0];
                ps[1] = triangles[3*t+1];
                ps[2] = triangles[3*t+2];
                TetrahedralizerUtility.BarycentricWeight(vertices[ps[0]],vertices[ps[1]],vertices[ps[2]],approximatedVertices[polyhedronFacets[i][j]], out ts[0], out ts[1], out ts[2]);
                meshVertexDataMapper.InterpolateVertexData(3, approximatedVertices[polyhedronFacets[i][j]],ps,ts);
            }
            for(int j=0; j<facetsAssociations[i].Count; j++)
            {
                int t = -1;
                int tMax = -1;
                for(int k=0; k<facetsAssociations[i][j].Count; k++)
                {
                    int uvIsland = uv0IslandsIndexes[facetsAssociations[i][j][k]];
                    if(uv0IslandsFrequency[uvIsland] > tMax && facetsAssociations[i].All(l=>l.Any(m=>uvIsland==uv0IslandsIndexes[m])))
                    {
                        t = facetsAssociations[i][j][k];
                        tMax = uv0IslandsFrequency[uvIsland];
                    }
                }
                if(t < 0)
                {
                    meshVertexDataMapper.AddDefaultValue(approximatedVertices[polyhedronFacets[i][j]]);
                    continue;
                }
                
                ps[0] = triangles[3*t+0];
                ps[1] = triangles[3*t+1];
                ps[2] = triangles[3*t+2];
                TetrahedralizerUtility.BarycentricWeight(vertices[ps[0]],vertices[ps[1]],vertices[ps[2]],approximatedVertices[polyhedronFacets[i][j]], out ts[0], out ts[1], out ts[2]);
                meshVertexDataMapper.InterpolateVertexData(3, approximatedVertices[polyhedronFacets[i][j]],ps,ts);
            }
        }

        List<int> polyTriangles = new List<int>();
        int triangleIndex = 0;
        for(int i=0; i<polyhedronFacets.Count; i++)
        {
            for(int j=1; j<polyhedronFacets[i].Count-1; j++)
            {
                polyTriangles.Add(triangleIndex);
                polyTriangles.Add(triangleIndex+j);
                polyTriangles.Add(triangleIndex+j+1);
            }
            triangleIndex += polyhedronFacets[i].Count;
            for(int j=1; j<polyhedronFacets[i].Count-1; j++)
            {
                polyTriangles.Add(triangleIndex);
                polyTriangles.Add(triangleIndex+j+1);
                polyTriangles.Add(triangleIndex+j);
            }
            triangleIndex += polyhedronFacets[i].Count;
        }

        Mesh mesh = meshVertexDataMapper.MakeMesh();
        mesh.triangles = polyTriangles.ToArray();
        mesh.RecalculateBounds();
        mesh.RecalculateNormals();
        mesh.RecalculateTangents();

        // temp
        MeshFilter meshFilter = new GameObject().AddComponent<MeshFilter>();
        meshFilter.gameObject.AddComponent<MeshRenderer>();
        meshFilter.mesh = mesh;

        //meshVertexDataMapper.MakeTetrahedralMesh(tetrahedralMesh);
        //tetrahedralMesh.SetFacetsSubmeshes(resultTrianglesSubmeshes);
    }
}
