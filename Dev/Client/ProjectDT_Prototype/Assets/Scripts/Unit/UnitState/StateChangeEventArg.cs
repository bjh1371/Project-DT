using UnityEngine;
using System.Collections;

/// <summary>
/// 스테이트 변경시 전달되는 인자값
/// </summary>
public class StateChangeEventArg
{
    public float mTime { get; set; }
    public DamageInfo mDamageInfo { get; set; }
    public UnitFindInfo mUnitFindInfo { get; set; }
}
