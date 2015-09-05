using UnityEngine;
using System.Collections;
using System.Collections.Generic;

/// <summary>
/// 로그 필터
/// </summary>
public enum eLogFilter
{
    Normal,                /// 일반 로그
    GameManager,           /// 게임 매니져
    PathManager,           /// 패스 매니져
    Max,
}   

/// <summary>
/// Log를 관리 하는 객체
/// 현재는 필터만 적용
/// </summary>
public class Log : MonoBehaviour {
    public static Log Instance;
    public bool[] mFilters = new bool[(int)eLogFilter.Max];

    void Awake()
    {
        if (Instance == null)
            Instance = this;
        else
            Debug.LogError("Log has two instances");
    }

    public static void PrintError(eLogFilter _filter, object _message)
    {
        if (Instance.mFilters[(int)_filter] == false)
            return;

        Debug.LogError(_filter.ToString() + " " + _message);
    }

    public static void PrintLog(eLogFilter _filter, object _message)
    {
        if (Instance.mFilters[(int)_filter] == false)
            return;

        Debug.Log(_filter.ToString() + " " + _message);
    }
}
