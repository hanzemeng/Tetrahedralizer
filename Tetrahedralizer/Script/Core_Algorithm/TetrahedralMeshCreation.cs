using System;
using System.Linq;
using System.Collections.Generic;
using System.Threading.Tasks;
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

        //return Task.Run(() =>
        //{
        //    CreateInternal(vertices, triangles, subMeshDescriptors, input.m_tetrahedralization.m_tetrahedrons, input.m_tetrahedralization.m_explicitVertices, input.m_tetrahedralization.m_implicitVertices, weldedTriangles, meshTriangleFinder, meshVertexDataMapper, tetrahedralMesh, progress);
        //});
        return null;
    }

    private void CreateInternal(Vector3[] vertices, int[] triangles, List<SubMeshDescriptor> subMeshDescriptors, List<int> tetrahedrons, List<double> explicitVertices, List<int> implicitVertices, int[] weldedTriangles, MeshTriangleFinder meshTriangleFinder, MeshVertexDataMapper meshVertexDataMapper, TetrahedralMesh tetrahedralMesh, IProgress<string> progress=null)
    {
        if(null != progress)
        {
            progress.Report("Associate facets.");
        }
        FacetAssociation.FacetAssociationOutput FAOutput = new FacetAssociation.FacetAssociationOutput();
        FacetAssociation.FacetAssociationInput FAInput = new FacetAssociation.FacetAssociationInput();
        {
            FacetAssociation facetAssociation = new FacetAssociation();
            FAInput.m_explicitVertices = explicitVertices;
            FAInput.m_implicitVertices = implicitVertices;
            FAInput.m_tetrahedrons = tetrahedrons;
            FAInput.m_constraints = weldedTriangles;

            facetAssociation.CalculateFacetAssociation(FAInput, FAOutput);
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

            approximatedVertices = TetrahedralizerUtility.PackDoubles(GPAOutput.m_approximatePositions);
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

        List<int> resultTrianglesSubmeshes = new List<int>();

        int FAOutputIndex = 0;
        int[] ps = new int[3];
        double[] ts = new double[3];
        void ProcessFacet(int p0, int p1, int p2)
        {
            int c0 = FAOutput.m_tetrahedronsFacetsMapping[FAOutputIndex++];
            int c1 = FAOutput.m_tetrahedronsFacetsMapping[FAOutputIndex++];
            int c2 = FAOutput.m_tetrahedronsFacetsMapping[FAOutputIndex++];

            // vertices not on constraints, find the closest triangle with a good normal alignment
            if(TetrahedralizerConstant.UNDEFINED_VALUE == c0 || TetrahedralizerConstant.UNDEFINED_VALUE == c1 || TetrahedralizerConstant.UNDEFINED_VALUE == c2 ||
               originalTrianglesSubmeshes[c0] != originalTrianglesSubmeshes[c1] ||
               originalTrianglesSubmeshes[c1] != originalTrianglesSubmeshes[c2])
            {
                Vector3 v0 = approximatedVertices[p0];
                Vector3 v1 = approximatedVertices[p1];
                Vector3 v2 = approximatedVertices[p2];
                Vector3 pointNormal = Vector3.Cross(v1-v0,v2-v1).normalized;
                c0 = meshTriangleFinder.FindClosestTriangle(v0, pointNormal, 0.95d);
                c1 = meshTriangleFinder.FindClosestTriangle(v1, pointNormal, 0.95d);
                c2 = meshTriangleFinder.FindClosestTriangle(v2, pointNormal, 0.95d);
            }
            // if still has no matching triangle, just add default value
            if( c0 < 0 || c1 < 0 || c2 < 0 || 
                originalTrianglesSubmeshes[c0] != originalTrianglesSubmeshes[c1] ||
                originalTrianglesSubmeshes[c1] != originalTrianglesSubmeshes[c2])
            {
                meshVertexDataMapper.AddDefaultValue(approximatedVertices[p0]);
                meshVertexDataMapper.AddDefaultValue(approximatedVertices[p1]);
                meshVertexDataMapper.AddDefaultValue(approximatedVertices[p2]);
                resultTrianglesSubmeshes.Add(originalSubmeshesCount);
                return;
            }

            void BarycentricWeight(int t0,int t1,int t2,int p)
            {
                TetrahedralizerUtility.BarycentricWeight(vertices[t0],vertices[t1],vertices[t2],approximatedVertices[p],
                out ts[0], out ts[1], out ts[2]);
                ps[0] = t0;
                ps[1] = t1;
                ps[2] = t2;
                meshVertexDataMapper.InterpolateVertexData(3, approximatedVertices[p],ps,ts);
            }

            BarycentricWeight(triangles[3*c0+0],triangles[3*c0+1],triangles[3*c0+2], p0);
            BarycentricWeight(triangles[3*c1+0],triangles[3*c1+1],triangles[3*c1+2], p1);
            BarycentricWeight(triangles[3*c2+0],triangles[3*c2+1],triangles[3*c2+2], p2);
            resultTrianglesSubmeshes.Add(originalTrianglesSubmeshes[c0]);
        }

        for(int i=0; i<tetrahedrons.Count; i+=4)
        {
            int p0 = tetrahedrons[i+0];
            int p1 = tetrahedrons[i+1];
            int p2 = tetrahedrons[i+2];
            int p3 = tetrahedrons[i+3];

            ProcessFacet(p0,p1,p3);
            ProcessFacet(p1,p0,p2);
            ProcessFacet(p0,p3,p2);
            ProcessFacet(p3,p1,p2);
        }

        meshVertexDataMapper.MakeTetrahedralMesh(tetrahedralMesh);
        tetrahedralMesh.SetFacetsSubmeshes(resultTrianglesSubmeshes);
    }
}
