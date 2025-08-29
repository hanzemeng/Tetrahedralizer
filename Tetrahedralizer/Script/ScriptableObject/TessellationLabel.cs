using System.Collections.Generic;
using UnityEngine;
#if UNITY_EDITOR
using UnityEditor;
#endif

[CreateAssetMenu(fileName = SHORT_NAME, menuName = TetrahedralizerLibraryConstant.SCRIPTABLE_OBJECT_PATH + SHORT_NAME)]
[PreferBinarySerialization]
public class TessellationLabel : ScriptableObject
{
    public const string SHORT_NAME = "TessellationLabel_SO";

    public List<double> m_windingNumbers;
    public List<int> m_interiorLabels; // 0 means exterior, otherwise interior
}


#if UNITY_EDITOR

[CustomEditor(typeof(TessellationLabel))]
public class TessellationLabelEditor : Editor
{
    private TessellationLabel m_so;


    private void OnEnable()
    {
        m_so = (TessellationLabel)target;
    }

    public override void OnInspectorGUI()
    {
        //base.OnInspectorGUI();

        int cnt = null == m_so.m_windingNumbers ? 0 : m_so.m_windingNumbers.Count;
        EditorGUILayout.LabelField($"Winding Numbers Count: {cnt}");
        
        cnt = null == m_so.m_interiorLabels ? 0 : m_so.m_interiorLabels.Count;
        EditorGUILayout.LabelField($"Interior Labels Count: {cnt}");
    }
}

#endif
