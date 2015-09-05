using UnityEngine;
using System.Collections;

/// <summary>
/// 유닛 상태 코드
/// </summary>
public enum eUnitState
{
    None,
    Idle,       /// 대기 
    Move,       /// 이동
    Attack,     /// 공격
    Dead,       /// 사망
}

/// <summary>
/// 공격 타입
/// </summary>
public enum eAttackType
{
    None,       
    Melee,      /// 근거리
    Range,      /// 원거리
}

/// <summary>
/// 데미지 정보
/// </summary>
public class DamageInfo
{
    public float mDamage { get; set; }
}

/// <summary>
/// 유닛 검색 정보
/// </summary>
public class UnitFindInfo
{
    public ePlayerCamp mCamp { get; set; }
    public ePathType mPathType { get; set; }
    public ushort mOid { get; set; }
}

/// <summary>
/// 유닛 스탯
/// </summary>
public class UnitStat 
{
    public float mMoveSpeed { get; set; }           /// 이동 속도
    public float mHP { get; set; }                  /// 체력
    public float mDefence { get; set; }             /// 방어력
    public float mAttackPower { get; set; }         /// 공격력
    public float mAttackRange { get; set; }         /// 공격 범위
    public float mAttackSpeed { get; set; }         /// 공격 속도
    public float mAttackDelay { get; set; }         /// 공격 모션 후 딜레이 (second)
    public eAttackType mAttackType { get; set; }    /// 공격 타입
}
