using System.Collections.Generic;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

[CreateAssetMenu(fileName = SHORT_NAME, menuName = TetrahedralizerConstant.SCRIPTABLE_OBJECT_PATH + SHORT_NAME)]
[PreferBinarySerialization]
public class Polyhedralization : ScriptableObject
{
    public const string SHORT_NAME = "Polyhedralization_SO";

    public List<double> m_explicitVertices; // Every 3 doubles are x,y,z of a point. Assuming left hand coordinate.
    public List<int> m_implicitVertices; // 5/9 followed by indexes of m_explicitVertices.
    public List<int> m_polyhedrons; // # of polyhedron facets, followed by facets indexes.
    public List<int> m_polyhedronsFacets;  // # of facets vertices, followed by vertices indexes ordered in cw or ccw.
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
        EditorGUILayout.LabelField($"Polyhedrons Count: {m_so.m_polyhedrons.Count}");
        EditorGUILayout.LabelField($"Polyhedrons' Facets Count: {m_so.m_polyhedronsFacets.Count}");
    }
}

#endif
