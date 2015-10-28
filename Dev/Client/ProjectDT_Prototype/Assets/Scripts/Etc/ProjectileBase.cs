using UnityEngine;
using System.Collections;

/// <summary>
/// 단순 공격을 가하는 발사체 (ex: 활, 총알)
/// 이외의 것은 skill로 해결
/// </summary>
public class ProjectileBase : MonoBehaviour {
    public DamageInfo mDamageInfo { get; set; } /// 데미지 정보
    public UnitBase mOwnerUnit { get; set; }    /// 발사를 한 유닛
    public UnitBase mTargetUnit { get; set; }   /// 공격 타겟 유닛   
                                                
    public bool mUseRotation;                   /// 중간중간에 방향이 틀어지는 경우 (ex. 화살)
    public float mArriveTime;                   /// 몇 초만에 도달할 건가 ( s)
    public float mAngleOffset;                  /// 0 ~ 0.5 (곡사 포일 때, 얼마나 각도를 줄것인가. 0.5일때 45도 방향 발사)
    private float mElapsedTime;                 /// 발사후 경과시간

    private Vector3 mTargetPosition;            /// 타겟 위치
    private Vector3 mOwnerPosition;             /// 발사를 한 유닛의 위치
    private Vector3 mPrePosition;               /// 전 tick에서의 발사체 위치
    
    /// 베지어 곡선 계산에 필요한 변수들
    private Vector3 mMidPoint;                  /// 발사 위치와 타겟 위치 중간지점
    private Vector3 mToTargetDirection;         /// 타겟 으로 향하는 벡터
    void Start()
    {
        //. 위치값 초기화                
        mOwnerPosition = mOwnerUnit.mFirePosition.position;
        mTargetPosition = mTargetUnit.mHitPosition.position;

        //. Angle Offset 보정
        mAngleOffset = Mathf.Clamp(mAngleOffset, 0.0f, 0.5f);

        transform.rotation = Quaternion.Euler(0, 0, -90 * mAngleOffset);

        //. 타겟 방향 벡터 구하기
        mToTargetDirection = ((mTargetPosition - mOwnerPosition)* 100).normalized;

        //. 법선 벡터 구하기
        bool isLeftToRight = (mToTargetDirection.x > 0) ? true : false;
        Vector3 normal = mToTargetDirection;
        normal.z = (isLeftToRight)? -1 : 1;
        normal = Vector3.Cross(mToTargetDirection, normal);

        //. 중간지점 구하기 
        float distance = Vector3.Distance(mTargetPosition, mOwnerPosition);
        Vector3 center = mOwnerPosition + (mToTargetDirection * 0.5f * distance);
        mMidPoint = center + (normal * distance * mAngleOffset);

        Log.Print(eLogFilter.Projectile, string.Format("start {0}, end {1}, dir {2}, midPoint {3}", mOwnerPosition, mTargetPosition, mToTargetDirection, mMidPoint));
        mElapsedTime = 0;
    }

    void Update()
    {
        Vector3 nextPosition = OnBezierCurvePoint(mElapsedTime / mArriveTime);
        mElapsedTime += Time.deltaTime;

        //. 회전 적용
        if(mUseRotation)
        {
            //. 회전각 구하기
            Vector3 nextDir = (nextPosition - transform.position).normalized;
            Vector3 shootDir = (mToTargetDirection.x > 0) ? Vector3.right : Vector3.left;
            transform.rotation = Quaternion.FromToRotation(shootDir, nextDir);

            bool isLeftToRight = (mToTargetDirection.x > 0) ? true : false;
            if (isLeftToRight == false)
                transform.Rotate(Vector3.up, 180.0f);
        }

        //. 도착 시간 후 충돌이 일어나지 않으면 삭제
        if (mElapsedTime >= mArriveTime + R.Const.PROJECTILE_DESTORY_DELAY_TIME)
        {
            Destroy(gameObject);
        }

        mPrePosition = transform.position;
        transform.position = nextPosition;
    }

    /// <summary>
    /// 2차 베지어 곡선 위에 포인트 구하기
    /// </summary>
    /// <param name="timeOffset">시간 변위 Offset (0~1)</param>
    /// <returns></returns>
    Vector3 OnBezierCurvePoint(float timeOffset)
    {
        Vector3 g0 = mOwnerPosition + ((mMidPoint - mOwnerPosition) * timeOffset);
        Vector3 g1 = mMidPoint + ((mTargetPosition - mMidPoint) * timeOffset);

        return g0 + ((g1 - g0) * timeOffset);
    }

    /// <summary>
    /// 발사체와 충돌이 일어 났을 경우
    /// </summary>
    /// <param name="other">충돌 대상</param>
    void OnTriggerEnter2D(Collider2D other)
    {
        //. 타겟 검사
        UnitBase collisionUnit = other.gameObject.GetComponent<UnitBase>();
        if (collisionUnit == mOwnerUnit || collisionUnit != mTargetUnit)
            return;

        if (mTargetUnit == null)
        {
            Destroy(gameObject);
            return;
        }            
        
        Log.Print(eLogFilter.Projectile, "발사체 충돌 했다.");

        //. 대상에게 충돌이 되었을 경우 데미지 준다.
        mTargetUnit.BeAttacked(mDamageInfo);

        //. 발사체 삭제
        Destroy(gameObject);
    }
}
