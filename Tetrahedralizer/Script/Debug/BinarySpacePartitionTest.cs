using System.Linq;
using System.Collections.Generic;
using UnityEngine;

public class BinarySpacePartitionTest : MonoBehaviour
{
    [SerializeField] private MeshFilter m_meshFilter;
    [SerializeField] private TetrahedralizationDrawer m_tetrahedralizationDrawer;
    [SerializeField] private PolyhedralizationDrawer m_polyhedralizationDrawer;


    [ContextMenu("Test")]
    public void Test()
    {
        Mesh mesh = m_meshFilter.sharedMesh;

        List<Vector3> vertices = mesh.vertices.ToList();
        int[] triangles = mesh.triangles;
        TetrahedralizerUtility.RemoveDuplicateVertices(vertices, triangles);
        List<double> verticesUnpack = TetrahedralizerUtility.UnpackVector3s(vertices);

        DelaunayTetrahedralization.DelaunayTetrahedralizationOutput DTOutput = new DelaunayTetrahedralization.DelaunayTetrahedralizationOutput();
        {
            DelaunayTetrahedralization.DelaunayTetrahedralizationInput input = new DelaunayTetrahedralization.DelaunayTetrahedralizationInput();
            input.m_explicitVertices = verticesUnpack;
            
            DelaunayTetrahedralization delaunayTetrahedralization = new DelaunayTetrahedralization();
            delaunayTetrahedralization.CalculateDelaunayTetrahedralization(input, DTOutput);
        }

        //Tetrahedralization tetrahedralization = new Tetrahedralization();
        //tetrahedralization.m_explicitVertices = verticesUnpack;
        //tetrahedralization.m_tetrahedrons = DTOutput.m_tetrahedrons;
        //m_tetrahedralizationDrawer.Draw(tetrahedralization);

        BinarySpacePartition.BinarySpacePartitionOutput BSPOutput = new BinarySpacePartition.BinarySpacePartitionOutput();
        {
            BinarySpacePartition.BinarySpacePartitionInput input = new BinarySpacePartition.BinarySpacePartitionInput();
            input.m_explicitVertices = verticesUnpack;
            input.m_tetrahedrons = DTOutput.m_tetrahedrons;
            input.m_constraints = triangles;
            BinarySpacePartition binarySpacePartition = new BinarySpacePartition();

            binarySpacePartition.CalculateBinarySpacePartition(input, BSPOutput);
        }

        TessellationLabel tessellationLabel = ScriptableObject.CreateInstance<TessellationLabel>();
        {
            InteriorCharacterization.InteriorCharacterizationInput input = new InteriorCharacterization.InteriorCharacterizationInput();
            input.m_explicitVertices = verticesUnpack;
            input.m_implicitVertices = BSPOutput.m_insertedVertices;
            input.m_polyhedrons = BSPOutput.m_polyhedrons;
            input.m_polyhedronsFacets = BSPOutput.m_polyhedronsFacets;
            input.m_constraints = triangles;
            input.m_polyhedronsWindingNumbers = null;
            input.m_minCutNeighborMultiplier = 0.1d;
            InteriorCharacterization.InteriorCharacterizationOutput output = new InteriorCharacterization.InteriorCharacterizationOutput();
            InteriorCharacterization interiorCharacterization = new InteriorCharacterization();
            interiorCharacterization.CalculateInteriorCharacterization(input, output);

            tessellationLabel.m_windingNumbers = output.m_polyhedronsWindingNumbers;
            tessellationLabel.m_interiorLabels = output.m_polyhedronsInteriorLabels;
        }

        ////Polyhedralization polyhedralization = ScriptableObject.CreateInstance<Polyhedralization>();
        ////polyhedralization.m_explicitVertices = verticesUnpack;
        ////polyhedralization.m_implicitVertices = BSPOutput.m_insertedVertices;
        ////polyhedralization.m_polyhedrons = BSPOutput.m_polyhedrons;
        ////polyhedralization.m_polyhedronsFacets = BSPOutput.m_polyhedronsFacets;

        ////m_polyhedralizationDrawer.Draw(polyhedralization, tessellationLabel);

        PolyhedralizationTetrahedralization.PolyhedralizationTetrahedralizationOutput PTOutput = new PolyhedralizationTetrahedralization.PolyhedralizationTetrahedralizationOutput();
        {
            PolyhedralizationTetrahedralization.PolyhedralizationTetrahedralizationInput input = new PolyhedralizationTetrahedralization.PolyhedralizationTetrahedralizationInput();
            
            PolyhedralizationTetrahedralization polyhedralizationTetrahedralization = new PolyhedralizationTetrahedralization();
            input.m_explicitVertices = verticesUnpack;
            input.m_implicitVertices = BSPOutput.m_insertedVertices;
            //input.m_polyhedrons = BSPOutput.m_polyhedrons;
            input.m_polyhedrons = TetrahedralizerUtility.NestedListToFlatList(TetrahedralizerUtility.FlatIListToNestedList(BSPOutput.m_polyhedrons)
            .Where((i,j)=>tessellationLabel.m_interiorLabels[j]!=0).ToList());
            input.m_polyhedronsFacets = BSPOutput.m_polyhedronsFacets;

            polyhedralizationTetrahedralization.CalculatePolyhedralizationTetrahedralization(input, PTOutput);
        }

        {
            FacetAssociation.FacetAssociationInput input = new FacetAssociation.FacetAssociationInput();
            FacetAssociation.FacetAssociationOutput output = new FacetAssociation.FacetAssociationOutput();
            FacetAssociation facetAssociation = new FacetAssociation();

            input.m_explicitVertices = verticesUnpack;
            input.m_implicitVertices = BSPOutput.m_insertedVertices;
            input.m_tetrahedrons = PTOutput.m_tetrahedrons;
            input.m_constraints = triangles;

            facetAssociation.CalculateFacetAssociation(input, output);
        }
        
        //Tetrahedralization tetrahedralization = new Tetrahedralization();
        //tetrahedralization.m_explicitVertices = verticesUnpack;
        //tetrahedralization.m_implicitVertices = BSPOutput.m_insertedVertices;
        //tetrahedralization.m_tetrahedrons = PTOutput.m_tetrahedrons;
        //m_tetrahedralizationDrawer.Draw(tetrahedralization);
       
    }

    [ContextMenu("Clear")]
    public void Clear()
    {
        m_tetrahedralizationDrawer.Clear();
        m_polyhedralizationDrawer.Clear();
    }
}
