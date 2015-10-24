using UnityEngine;
using System.Collections;

/// <summary>
/// 종족 Enum값.. 종족 이름 딴거 없나?? 
/// </summary>
public enum eTribe
{
    Human,      /// 인간
    Animal,     /// 동물
    Fish,       /// 물고기
}

/// <summary>
/// 플레이어들 진영
/// </summary>
public enum ePlayerCamp
{
    Left,       /// 왼쪽 진영
    Right,      /// 오른쪽 진영
}

/// <summary>
/// Animation Event 코드값
/// </summary>
public enum eAnimationEvent
{
    AnimationEnd,   /// 애니메이션 종류
    Fire,           /// 발사
    Attack,         /// 공격
}

/// <summary>
/// 전역 상수값들 선언
/// 내부 클래스는 카테고리처럼 사용
/// </summary>
public class R
{
    /// <summary>
    /// 일반 상수 값들
    /// </summary>
    public class Const
    {
        public static int INDEX_NONE = -1;

        public static ushort INVALID_OBJECT_ID = 0;

        public static float PROJECTILE_DESTORY_DELAY_TIME = 0.5f;   /// 발사체가 충돌없이 도달했을 때, 얼마후에 Destory할것인가(second) 
    }
}
