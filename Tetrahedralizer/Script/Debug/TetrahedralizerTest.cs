using System.Linq;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;

public class TetrahedralizerTest : MonoBehaviour
{
    [SerializeField] private GameObject m_targetGameObject;
    [SerializeField] private TetrahedralizationDrawer m_tetrahedralizationDrawer;
    [SerializeField] private PolyhedralizationDrawer m_polyhedralizationDrawer;
    [SerializeField] private TetrahedralMeshDrawer m_tetrahedralMeshDrawer;

    [SerializeField] private Material m_interiorMaterial;


    [ContextMenu("Tetrahedral Mesh Test")]
    public void TetrahedralMeshTest()
    {
        TetrahedralizedMeshCreation tetrahedralizedMeshCreation = new TetrahedralizedMeshCreation();
        TetrahedralizedMeshCreation.TetrahedralizedMeshCreationInput tetrahedralizedMeshCreationInput = new TetrahedralizedMeshCreation.TetrahedralizedMeshCreationInput();
        TetrahedralizedMeshCreation.TetrahedralizedMeshCreationOutput tetrahedralizedMeshCreationOutput = new TetrahedralizedMeshCreation.TetrahedralizedMeshCreationOutput();

        tetrahedralizedMeshCreationInput.m_mesh = m_targetGameObject.GetComponent<MeshFilter>().sharedMesh;
        tetrahedralizedMeshCreation.Create(tetrahedralizedMeshCreationInput, tetrahedralizedMeshCreationOutput);

        TetrahedralMeshCreation tetrahedralMeshCreation = new TetrahedralMeshCreation();
        TetrahedralMeshCreation.TetrahedralMeshCreationInput tetrahedralMeshCreationInput = new TetrahedralMeshCreation.TetrahedralMeshCreationInput();
        TetrahedralMeshCreation.TetrahedralMeshCreationOutput tetrahedralMeshCreationOutput = new TetrahedralMeshCreation.TetrahedralMeshCreationOutput();
        tetrahedralMeshCreationInput.m_mesh = tetrahedralizedMeshCreationInput.m_mesh;
        tetrahedralMeshCreationInput.m_tetrahedralization = tetrahedralizedMeshCreationOutput.m_tetrahedralization;
        tetrahedralMeshCreation.Create(tetrahedralMeshCreationInput, tetrahedralMeshCreationOutput);
        

        List<Material> materials = new List<Material>();
        m_targetGameObject.GetComponent<MeshRenderer>().GetSharedMaterials(materials);
        materials.Add(m_interiorMaterial);
        m_tetrahedralMeshDrawer.Draw(tetrahedralMeshCreationOutput.m_tetrahedralMesh, materials);
    }
    [ContextMenu("Tetrahedralized Mesh Test")]
    public void TetrahedralizedMeshTest()
    {
        TetrahedralizedMeshCreation tetrahedralizedMeshCreation = new TetrahedralizedMeshCreation();
        TetrahedralizedMeshCreation.TetrahedralizedMeshCreationInput tetrahedralizedMeshCreationInput = new TetrahedralizedMeshCreation.TetrahedralizedMeshCreationInput();
        TetrahedralizedMeshCreation.TetrahedralizedMeshCreationOutput tetrahedralizedMeshCreationOutput = new TetrahedralizedMeshCreation.TetrahedralizedMeshCreationOutput();

        tetrahedralizedMeshCreationInput.m_mesh = m_targetGameObject.GetComponent<MeshFilter>().sharedMesh;
        tetrahedralizedMeshCreation.Create(tetrahedralizedMeshCreationInput, tetrahedralizedMeshCreationOutput);
        m_tetrahedralizationDrawer.Draw(tetrahedralizedMeshCreationOutput.m_tetrahedralization);
    }


    public void OldTest()
    {
        Mesh mesh = m_targetGameObject.GetComponent<MeshFilter>().sharedMesh;
        List<Material> materials = new List<Material>();
        m_targetGameObject.GetComponent<MeshRenderer>().GetMaterials(materials);
        materials.Add(m_interiorMaterial);
        
        List<Vector3> weldedVertices = mesh.vertices.ToList();
        int[] weldedTriangles = mesh.triangles;
        Vector3[] vertices = mesh.vertices;
        int[] triangles = mesh.triangles;
        List<List<int>> mapping = TetrahedralizerUtility.RemoveDuplicateVertices(weldedVertices, weldedTriangles);
        List<double> weldedVerticesUnpack = TetrahedralizerUtility.UnpackVector3s(weldedVertices);

        DelaunayTetrahedralization.DelaunayTetrahedralizationOutput DTOutput = new DelaunayTetrahedralization.DelaunayTetrahedralizationOutput();
        {
            DelaunayTetrahedralization.DelaunayTetrahedralizationInput input = new DelaunayTetrahedralization.DelaunayTetrahedralizationInput();
            DelaunayTetrahedralization delaunayTetrahedralization = new DelaunayTetrahedralization();
            input.m_explicitVertices = weldedVerticesUnpack;
            delaunayTetrahedralization.CalculateDelaunayTetrahedralization(input, DTOutput);
        }

        //Tetrahedralization tetrahedralization = new Tetrahedralization();
        //tetrahedralization.m_explicitVertices = verticesUnpack;
        //tetrahedralization.m_tetrahedrons = DTOutput.m_tetrahedrons;
        //m_tetrahedralizationDrawer.Draw(tetrahedralization);

        BinarySpacePartition.BinarySpacePartitionOutput BSPOutput = new BinarySpacePartition.BinarySpacePartitionOutput();
        {
            BinarySpacePartition.BinarySpacePartitionInput input = new BinarySpacePartition.BinarySpacePartitionInput();
            BinarySpacePartition binarySpacePartition = new BinarySpacePartition();
            input.m_explicitVertices = weldedVerticesUnpack;
            input.m_tetrahedrons = DTOutput.m_tetrahedrons;
            input.m_constraints = weldedTriangles;
            binarySpacePartition.CalculateBinarySpacePartition(input, BSPOutput);
        }

        TessellationLabel tessellationLabel = ScriptableObject.CreateInstance<TessellationLabel>();
        {
            InteriorCharacterization.InteriorCharacterizationInput input = new InteriorCharacterization.InteriorCharacterizationInput();
            InteriorCharacterization.InteriorCharacterizationOutput output = new InteriorCharacterization.InteriorCharacterizationOutput();
            InteriorCharacterization interiorCharacterization = new InteriorCharacterization();
            input.m_explicitVertices = weldedVerticesUnpack;
            input.m_implicitVertices = BSPOutput.m_insertedVertices;
            input.m_polyhedrons = BSPOutput.m_polyhedrons;
            input.m_polyhedronsFacets = BSPOutput.m_polyhedronsFacets;
            input.m_constraints = weldedTriangles;
            input.m_polyhedronsWindingNumbers = null;
            input.m_minCutNeighborMultiplier = 0.1d;
            interiorCharacterization.CalculateInteriorCharacterization(input, output);

            tessellationLabel.m_windingNumbers = output.m_polyhedronsWindingNumbers;
            tessellationLabel.m_interiorLabels = output.m_polyhedronsInteriorLabels;
        }

        ////Polyhedralization polyhedralization = ScriptableObject.CreateInstance<Polyhedralization>();
        ////polyhedralization.m_explicitVertices = verticesUnpack;
        ////polyhedralization.m_implicitVertices = BSPOutput.m_insertedVertices;
        ////polyhedralization.m_polyhedrons = BSPOutput.m_polyhedrons;
        ////polyhedralization.m_polyhedronsFacets = BSPOutput.m_polyhedronsFacets;
        ////m_polyhedralizationDrawer.Draw(polyhedralization);

        PolyhedralizationTetrahedralization.PolyhedralizationTetrahedralizationOutput PTOutput = new PolyhedralizationTetrahedralization.PolyhedralizationTetrahedralizationOutput();
        {
            PolyhedralizationTetrahedralization.PolyhedralizationTetrahedralizationInput input = new PolyhedralizationTetrahedralization.PolyhedralizationTetrahedralizationInput();
            PolyhedralizationTetrahedralization polyhedralizationTetrahedralization = new PolyhedralizationTetrahedralization();
            input.m_explicitVertices = weldedVerticesUnpack;
            input.m_implicitVertices = BSPOutput.m_insertedVertices;
            //input.m_polyhedrons = BSPOutput.m_polyhedrons;
            input.m_polyhedrons = TetrahedralizerUtility.NestedListToFlatList(TetrahedralizerUtility.FlatIListToNestedList(BSPOutput.m_polyhedrons)
            .Where((i,j)=>tessellationLabel.m_interiorLabels[j]!=0).ToList());
            input.m_polyhedronsFacets = BSPOutput.m_polyhedronsFacets;
            polyhedralizationTetrahedralization.CalculatePolyhedralizationTetrahedralization(input, PTOutput);
        }

        //Tetrahedralization tetrahedralization = new Tetrahedralization();
        //tetrahedralization.m_explicitVertices = verticesUnpack;
        //tetrahedralization.m_implicitVertices = BSPOutput.m_insertedVertices;
        //tetrahedralization.m_tetrahedrons = PTOutput.m_tetrahedrons;
        //m_tetrahedralizationDrawer.Draw(tetrahedralization);

        FacetAssociation.FacetAssociationOutput FAOutput = new FacetAssociation.FacetAssociationOutput();
        {
            FacetAssociation.FacetAssociationInput input = new FacetAssociation.FacetAssociationInput();
            FacetAssociation facetAssociation = new FacetAssociation();
            input.m_explicitVertices = weldedVerticesUnpack;
            input.m_implicitVertices = BSPOutput.m_insertedVertices;
            input.m_tetrahedrons = PTOutput.m_tetrahedrons;
            input.m_constraints = weldedTriangles;

            facetAssociation.CalculateFacetAssociation(input, FAOutput);
        }
        
        List<Vector3> approximatedVertices;
        {
            GenericPointApproximation.GenericPointApproximationOutput output = new GenericPointApproximation.GenericPointApproximationOutput();
            GenericPointApproximation.GenericPointApproximationInput input = new GenericPointApproximation.GenericPointApproximationInput();
            GenericPointApproximation genericPointApproximation = new GenericPointApproximation();
            input.m_explicitVertices = weldedVerticesUnpack;
            input.m_implicitVertices = BSPOutput.m_insertedVertices;
            genericPointApproximation.CalculateGenericPointApproximation(input, output);

            approximatedVertices = TetrahedralizerUtility.PackDoubles(output.m_approximatePositions);
        }

        int originalSubmeshesCount = mesh.subMeshCount;
        List<int> originalTrianglesSubmeshes = Enumerable.Range(0,triangles.Count()/3).Select(i=>originalSubmeshesCount).ToList();
        for(int i=0; i<originalSubmeshesCount; i++)
        {
            SubMeshDescriptor subMeshDescriptor = mesh.GetSubMesh(i);
            //Debug.Log($"{subMeshDescriptor.indexStart}_{subMeshDescriptor.indexCount}");
            for(int j=subMeshDescriptor.indexStart; j<subMeshDescriptor.indexStart+subMeshDescriptor.indexCount; j+=3)
            {
                originalTrianglesSubmeshes[j/3] = i;
            }
        }

        MeshVertexDataMapper meshVertexDataMapper = new MeshVertexDataMapper();
        meshVertexDataMapper.AssignSourceMesh(mesh);
        int UNDEFINED_VALUE = -1;
        List<int> resultTrianglesSubmeshes = new List<int>();

        int FAOutputIndex = 0;
        int[] ps = new int[3];
        double[] ts = new double[3];
        void ProcessFacet(int p0, int p1, int p2)
        {
            int c0 = FAOutput.m_tetrahedronsFacetsMapping[FAOutputIndex++];
            int c1 = FAOutput.m_tetrahedronsFacetsMapping[FAOutputIndex++];
            int c2 = FAOutput.m_tetrahedronsFacetsMapping[FAOutputIndex++];

            if(UNDEFINED_VALUE == c0 || UNDEFINED_VALUE == c1 || UNDEFINED_VALUE == c2 ||
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
                double triArea = Vector3.Cross(vertices[t1] - vertices[t0], vertices[t2] - vertices[t0]).magnitude;
                Vector3 ep0 = vertices[t0] - approximatedVertices[p];
                Vector3 ep1 = vertices[t1] - approximatedVertices[p];
                Vector3 ep2 = vertices[t2] - approximatedVertices[p];
                ps[0] = t0;
                ps[1] = t1;
                ps[2] = t2;
                ts[0] = Vector3.Cross(ep1,ep2).magnitude / triArea;
                ts[1] = Vector3.Cross(ep2,ep0).magnitude / triArea;
                ts[2] = Vector3.Cross(ep0,ep1).magnitude / triArea;
                meshVertexDataMapper.InterpolateVertexData(3, approximatedVertices[p],ps,ts);
            }

            BarycentricWeight(triangles[3*c0+0],triangles[3*c0+1],triangles[3*c0+2], p0);
            BarycentricWeight(triangles[3*c1+0],triangles[3*c1+1],triangles[3*c1+2], p1);
            BarycentricWeight(triangles[3*c2+0],triangles[3*c2+1],triangles[3*c2+2], p2);
            resultTrianglesSubmeshes.Add(originalTrianglesSubmeshes[c0]);
        }

        for(int i=0; i<PTOutput.m_tetrahedrons.Count; i+=4)
        {
            int p0 = PTOutput.m_tetrahedrons[i+0];
            int p1 = PTOutput.m_tetrahedrons[i+1];
            int p2 = PTOutput.m_tetrahedrons[i+2];
            int p3 = PTOutput.m_tetrahedrons[i+3];

            ProcessFacet(p0,p1,p3);
            ProcessFacet(p1,p0,p2);
            ProcessFacet(p0,p3,p2);
            ProcessFacet(p3,p1,p2);
        }

        TetrahedralMesh tetrahedralMesh = ScriptableObject.CreateInstance<TetrahedralMesh>();
        meshVertexDataMapper.MakeTetrahedralMesh(tetrahedralMesh);
        tetrahedralMesh.SetFacetsSubmeshes(resultTrianglesSubmeshes);

        m_tetrahedralMeshDrawer.Draw(tetrahedralMesh, materials);
       
        Debug.Log("DONE");
    }

    [ContextMenu("Clear")]
    public void Clear()
    {
        m_tetrahedralizationDrawer.Clear();
        m_polyhedralizationDrawer.Clear();
        m_tetrahedralMeshDrawer.Clear();
    }
}
