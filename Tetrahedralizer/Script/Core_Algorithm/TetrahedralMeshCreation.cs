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
        public List<Material> m_materials;
        public List<List<(Texture2D,List<string>,int)>> m_textures; // texture, shader properties names, uv channel
        public Polyhedralization m_polyhedralization;
    }
    public class TetrahedralMeshCreationOutput
    {
        public List<Material> m_materials;
        public List<List<Texture2D>> m_textures;
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
        MeshVertexDataMapper meshVertexDataMapper = new MeshVertexDataMapper();
        meshVertexDataMapper.AssignSourceMesh(input.m_mesh);

        TetrahedralMesh tetrahedralMesh = ScriptableObject.CreateInstance<TetrahedralMesh>();
        output.m_tetrahedralMesh = tetrahedralMesh;

        output.m_materials = new List<Material>();
        output.m_textures = new List<List<Texture2D>>();
        for(int i=0; i<input.m_materials.Count; i++)
        {
            Material material = new Material(input.m_materials[i]);
            List<(Texture2D, List<string>)> textures = input.m_textures[i].Select(i=>(i.Item1.DuplicateWithUninitializedPixels(),i.Item2)).ToList();
            textures.ForEach(i=>i.Item2.ForEach(j=>material.SetTexture(j,i.Item1)));
            output.m_materials.Add(material);
            output.m_textures.Add(textures.Select(i=>i.Item1).ToList());
        }

        List<Color[]> srcTexturesColors = input.m_textures.SelectMany(i=>i.Select(j=>j.Item1.GetPixels())).ToList();
        List<Color[]> desTexturesColors = output.m_textures.SelectMany(i=>i.Select(j=>j.GetPixels())).ToList();
        List<(int,int)> texturesDimensions = output.m_textures.SelectMany(i=>i.Select(j=>(j.width,j.height))).ToList();
        List<int> texturesUVChannels = input.m_textures.SelectMany(i=>i.Select(j=>j.Item3)).ToList();

        List<List<Vector2>> srcUVs = Enumerable.Repeat(new List<Vector2>(),8).ToList();
        for(int i=0; i<8; i++)
        {
            int j = (int)VertexAttribute.TexCoord0+i;
            if(input.m_mesh.HasVertexAttribute((VertexAttribute)j))
            {
                input.m_mesh.GetUVs(i, srcUVs[i]);
            }
        }
        List<List<Vector2>> facetsUVs = input.m_polyhedralization.GetFacetsUVs();
        bool[] isExteriorFacets = input.m_polyhedralization.GetFacetsExteriorFlags();
        
        List<List<int>> facets = TetrahedralizerUtility.FlatIListToNestedList(input.m_polyhedralization.m_polyhedronsFacets);
        List<double> desVertices = GenericPointApproximation.CalculateGenericPointApproximation(input.m_polyhedralization.m_explicitVertices, input.m_polyhedralization.m_implicitVertices);

        for(int i=0; i<facetsUVs.Count; i++)
        {
            if(!isExteriorFacets[i])
            {
                continue;
            }
            for(int j=1; j<facetsUVs[i].Count-1; j++)
            {
                Vector2 uv0 = facetsUVs[i][0];
                Vector2 uv1 = facetsUVs[i][j];
                Vector2 uv2 = facetsUVs[i][j+1];
                
                Vector3 p0 = new Vector3((float)desVertices[3*facets[i][0]+0],(float)desVertices[3*facets[i][0]+1],(float)desVertices[3*facets[i][0]+2]);
                Vector3 p1 = new Vector3((float)desVertices[3*facets[i][j]+0],(float)desVertices[3*facets[i][j]+1],(float)desVertices[3*facets[i][j]+2]);
                Vector3 p2 = new Vector3((float)desVertices[3*facets[i][j+1]+0],(float)desVertices[3*facets[i][j+1]+1],(float)desVertices[3*facets[i][j+1]+2]);
                Vector3 n = input.m_polyhedralization.m_polyhedronsFacetsPointOut[i] ? Vector3.Cross(p1-p0,p2-p1) : -Vector3.Cross(p1-p0,p2-p1);

            }
        }

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
