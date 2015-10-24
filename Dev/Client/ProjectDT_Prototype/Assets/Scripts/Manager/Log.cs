using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System;

/// <summary>
/// 로그 필터
/// </summary>
public enum eLogFilter : int
{
    Normal,                /// 일반 로그
    GameManager,           /// 게임 매니져
    PathManager,           /// 패스 매니져
    Projectile,            /// 발사체                       
}   

/// <summary>
/// Log를 관리 하는 객체
/// 현재는 필터만 적용
/// </summary>
public class Log : MonoBehaviour {
    public static Log Instance;
    public bool[] mFilters;

    void Awake()
    {
        if (Instance == null)
        {
            Instance = this;
            mFilters = new bool[Enum.GetValues(typeof(eLogFilter)).Length];
        }
        else
            Debug.LogError("Log has two instances");
    }

    public static void PrintError(eLogFilter _filter, object _message)
    {
        if (Instance.mFilters[(int)_filter] == false)
            return;

        Debug.LogError(_filter.ToString() + " " + _message);
    }

    public static void Print(eLogFilter _filter, object _message)
    {
        if (Instance.mFilters[(int)_filter] == false)
            return;

        Debug.Log(_filter.ToString() + " " + _message);
    }

    public static void Print(object _message)
    {
        Print(eLogFilter.Normal, _message);
    }
}
