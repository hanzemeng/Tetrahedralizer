namespace Hanzzz.Tetrahedralizer
{

using UnityEngine;

[ExecuteInEditMode]
public class TetrahedralizerTest : MonoBehaviour
{
    public MeshFilter targetMeshFilter;
    public TetrahedralMeshDrawer tetrahedralMeshDrawer;
    public Tetrahedralizer tetrahedralizer;
    public bool remapVertexData;
    public float degenerateTetrahedronRatio;

    private void OnEnable()
    {
        tetrahedralizer = new Tetrahedralizer();
    }

    [ContextMenu("Test")]
    public void Test()
    {
        TetrahedralizedMesh tm0 = ScriptableObject.CreateInstance<TetrahedralizedMesh>();
        TetrahedralMesh tm1 = ScriptableObject.CreateInstance<TetrahedralMesh>();

        tetrahedralizer.SetSettings(new Tetrahedralizer.Settings(remapVertexData,degenerateTetrahedronRatio));
        tetrahedralizer.MeshToTetrahedralizedMesh(targetMeshFilter.sharedMesh, tm0);
        tetrahedralizer.TetrahedralizedMeshToTetrahedralMesh(tm0, tm1);
        tetrahedralMeshDrawer.tetrahedralMesh = tm1;
        tetrahedralMeshDrawer.UpdateMesh();
    }

    [ContextMenu("Clear")]
    public void Clear()
    {
        tetrahedralMeshDrawer.tetrahedralMesh = null;
        tetrahedralMeshDrawer.UpdateMesh();
    }

    //[ContextMenu("Save Mesh to File")]
    //public void SavePointsToFile()
    //{
    //    Vector3[] vs = targetMeshFilter.sharedMesh.vertices;
    //    Dictionary<Vector3, int> mapping = new Dictionary<Vector3, int>();
    //    foreach(Vector3 v in vs)
    //    {
    //        if(mapping.ContainsKey(v))
    //        {
    //            continue;
    //        }
    //        mapping[v] = mapping.Count; 
    //    }
    //    int[] ts = targetMeshFilter.sharedMesh.triangles;
    //    for(int i=0; i<ts.Length; i++)
    //    {
    //        ts[i] = mapping[vs[ts[i]]];
    //    }
    //    foreach(var kvp in mapping)
    //    {
    //        vs[kvp.Value] = kvp.Key;
    //    }

    //    using (StreamWriter sw = new StreamWriter("/Users/hanzemeng/Desktop/test"))
    //    {
    //        sw.WriteLine($"{mapping.Count} {ts.Length/3}");
    //        for(int i=0; i<mapping.Count; i++)
    //        {
    //            sw.WriteLine($"{vs[i].x} {vs[i].z} {vs[i].y}"); // dumbass Unity that uses left hand coordinate system
    //        }
    //        for(int i=0; i<ts.Length; i+=3)
    //        {
    //            if(ts[i+0] == ts[i+2] || ts[i+0] == ts[i+1] || ts[i+2] == ts[i+1])
    //            {
    //                Debug.Log("error");
    //            }
    //            sw.WriteLine($"{ts[i+0]} {ts[i+2]} {ts[i+1]}"); // dumbass Unity that uses left hand coordinate system
    //        }
    //    }
    //}


    //[ContextMenu("Load Polyhedrons from File")]
    //public void LoadPolyhedronsFromFile()
    //{
    //    Clear();

    //    List<Vector3> vertices = targetMeshFilter.sharedMesh.vertices.ToList();
    //    Dictionary<Vector3, int> mapping = new Dictionary<Vector3, int>();
    //    foreach(Vector3 v in vertices)
    //    {
    //        if(mapping.ContainsKey(v))
    //        {
    //            continue;
    //        }
    //        mapping[v] = mapping.Count;
    //    }
    //    foreach(var kvp in mapping)
    //    {
    //        vertices[kvp.Value] = kvp.Key;
    //    }
    //    vertices.RemoveRange(mapping.Count,vertices.Count-mapping.Count);

    //    List<int> triangles = new List<int>();
    //    string line;
    //    using(StreamReader sr = new StreamReader("/Users/hanzemeng/Desktop/out"))
    //    {
    //        line = sr.ReadLine();
    //        int newVerticesCount = int.Parse(line);
    //        for(int i = 0; i<newVerticesCount; i++)
    //        {
    //            line = sr.ReadLine();
    //            string[] ps = line.Split(' ');
    //            vertices.Add(new Vector3((float)double.Parse(ps[1]),(float)double.Parse(ps[3]),(float)double.Parse(ps[2])));
    //        }

    //        line = sr.ReadLine();
    //        int polyhedronCount = int.Parse(line);
    //        List<List<int>> polyhedron = new();
    //        for(int i = 0; i<polyhedronCount; i++)
    //        {
    //            Vector3 polyhedronCenter = Vector3.zero;

    //            line = sr.ReadLine();
    //            int facetCount = int.Parse(line);

    //            for(int j = 0; j<facetCount; j++)
    //            {
    //                if(polyhedron.Count == j)
    //                {
    //                    polyhedron.Add(new List<int>());
    //                }
    //                else
    //                {
    //                    polyhedron[j].Clear();
    //                }

    //                Vector3 facetCenter = Vector3.zero;

    //                line = sr.ReadLine();
    //                string[] ps = line.Split(' ');
    //                int verticesCount = int.Parse(ps[0]);
    //                for(int k = 0; k<verticesCount; k++)
    //                {
    //                    polyhedron[j].Add(int.Parse(ps[k+1]));

    //                    facetCenter += vertices[polyhedron[j][k]];
    //                }
    //                facetCenter /= verticesCount;
    //                polyhedronCenter += facetCenter;
    //                polyhedron[j].Add(vertices.Count);
    //                vertices.Add(facetCenter);
    //            }
    //            polyhedronCenter /= facetCount;

    //            if(i < revealPolyhedronRange.x || i >= revealPolyhedronRange.y)
    //            {
    //                continue;
    //            }

    //            for(int j = 0; j<facetCount; j++)
    //            {
    //                int facetOrient = PredicatesWrapper.Orient3D(vertices[polyhedron[j][0]],vertices[polyhedron[j][1]],vertices[polyhedron[j][^1]], polyhedronCenter);
    //                int center = polyhedron[j][^1];
    //                int n = polyhedron[j].Count-1;

    //                for(int k = 0; k<n; k++)
    //                {
    //                    triangles.Add(polyhedron[j][k]);
    //                    triangles.Add(polyhedron[j][(k+1)%n]);
    //                    triangles.Add(center);
    //                    triangles.Add(polyhedron[j][(k+1)%n]);
    //                    triangles.Add(polyhedron[j][k]);
    //                    triangles.Add(center);
    //                }
    //            }
    //        }
    //    }

    //    Transform t = Instantiate(tetrahedronPrefab, tetrahedronsParent).transform;
    //    MeshFilter meshFilter = t.GetComponent<MeshFilter>();
    //    Mesh mesh = new Mesh();
    //    mesh.indexFormat = UnityEngine.Rendering.IndexFormat.UInt32;
    //    mesh.vertices = vertices.ToArray();
    //    mesh.triangles = triangles.ToArray();
    //    mesh.RecalculateNormals();
    //    mesh.RecalculateTangents();
    //    meshFilter.mesh = mesh;
    //}
}

}
