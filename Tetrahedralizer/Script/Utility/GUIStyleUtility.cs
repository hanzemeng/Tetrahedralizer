#if UNITY_EDITOR

using UnityEngine;
using UnityEditor;

public static class GUIStyleUtility
{
    private static GUIStyle m_centerLabelStyleField;
    public static GUIStyle centerLabelStyle
    {
        get
        {
            if(null == m_centerLabelStyleField)
            {
                m_centerLabelStyleField = new GUIStyle(EditorStyles.label)
                {
                    alignment = TextAnchor.MiddleCenter
                };
            }
            return m_centerLabelStyleField;
        }
    }

    private static GUIStyle m_centerBoldLabelStyleField;
    public static GUIStyle centerBoldLabelStyle
    {
        get
        {
            if(null == m_centerBoldLabelStyleField)
            {
                m_centerBoldLabelStyleField = new GUIStyle(EditorStyles.boldLabel)
                {
                    alignment = TextAnchor.MiddleCenter
                };
            }
            return m_centerBoldLabelStyleField;
        }
    }
}

#endif
