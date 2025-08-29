using System.Collections.Generic;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

[CreateAssetMenu(fileName = SHORT_NAME, menuName = TetrahedralizerLibraryConstant.SCRIPTABLE_OBJECT_PATH + SHORT_NAME)]
[PreferBinarySerialization]
public class Tetrahedralization : ScriptableObject
{
    public const string SHORT_NAME = "Tetrahedralization_SO";

    public List<double> m_explicitVertices; // Every 3 doubles are x,y,z of a point. Assuming left hand coordinate.
    public List<int> m_implicitVertices; // 5/9 followed by indexes of m_explicitVertices
    public List<int> m_tetrahedrons; // Every 4 ints is a tetrahedron. Curl around the first 3 points and your thumb points toward the 4th point. Assuming left hand coordinate.
}


#if UNITY_EDITOR

[CustomEditor(typeof(Tetrahedralization))]
public class TetrahedralizationEditor : Editor
{
    private Tetrahedralization m_so;


    private void OnEnable()
    {
        m_so = (Tetrahedralization)target;
    }

    public override void OnInspectorGUI()
    {
        //base.OnInspectorGUI();

        if(null == m_so.m_explicitVertices)
        {
            EditorGUILayout.LabelField($"Vertices Count: {0}");
            EditorGUILayout.LabelField($"Tetrahedrons Count: {0}");
            return;
        }

        EditorGUILayout.LabelField($"Vertices Count: {m_so.m_explicitVertices.Count/3 + TetrahedralizerLibraryUtility.CountFlatIListElements(m_so.m_implicitVertices)}");
        EditorGUILayout.LabelField($"Tetrahedrons Count: {m_so.m_tetrahedrons.Count / 4}");
    }
}

#endif
