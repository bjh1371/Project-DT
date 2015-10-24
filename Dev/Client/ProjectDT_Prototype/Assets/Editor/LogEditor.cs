using UnityEngine;
using UnityEditor;
using System.Collections;
using System;

/// <summary>
/// Log 커스텀 에디터 
/// Runtime에서 LogFilter Check 동적으로 하기 위해 
/// </summary>
[CustomEditor(typeof(Log))]
public class LogEditor : Editor
{
    string[] mEnumNames = Enum.GetNames(typeof(eLogFilter));

    /// <summary>
    /// Inspector GUI 변경
    /// Enum값 대로 체크박스 노출되도록 수정
    /// </summary>
    public override void OnInspectorGUI()
    {
        if (Log.Instance == null)
            return;

        // 체크 박스 생성.
        foreach(eLogFilter value in Enum.GetValues(typeof(eLogFilter)))
        {
            int i = (int)value;
            Log.Instance.mFilters[i] = EditorGUILayout.Toggle(mEnumNames[i], Log.Instance.mFilters[i]);            
        }
            
        // GUI 변경 되었으면 타겟을 다시 렌더링하도록 dirty 마크.
        if (GUI.changed)
            EditorUtility.SetDirty(target);
    }   
}
