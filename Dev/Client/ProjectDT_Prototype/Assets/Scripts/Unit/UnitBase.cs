using UnityEngine;
using System.Collections;

public class UnitBase : MonoBehaviour {

	public float MoveSpeed = 1.5f;
	public float AttackSpeed = 0.15f;
	public float AttackDelay = 10.0f;
	public int HP = 40;
	public int Defence = 1;
	public int AttackPower = 1;
	public int AttackRange = 1;
	public eAttackType AttackType = eAttackType.None;

    public ushort mOId { get; set; }                    /// 오브젝트 아이디
    public ePlayerCamp mCamp { get; set; }              /// 유닛 소속 진영
    
    public UnitStat mStat { get; protected set; }       /// 유닛 스탯
    public PathInfo mPathInfo { get; protected set; }   /// 패스 정보
    public UnitState mState { get; protected set; }     /// 유닛 상태
                                                         
    private Rigidbody2D mRigidBody;                     /// 유닛 이동을 위한 객체

    public GameObject mProjectileObj;                   /// 발사체 오브젝트 ( 레인지 유닛일때 사용 )
    public Transform mFirePosition;                     /// 발사 위치  ( 레인지 유닛일때 사용 )
    public Transform mHitPosition;                      /// 투사체가 박히는 위치

    void Awake()
    {
        mState = new MoveState(this);
        mRigidBody = GetComponent<Rigidbody2D>();
    }

	// Use this for initialization
	void Start () {

	}
	
	// Update is called once per frame
	void Update ()
    {
        if(mState != null)
            mState.Update();
	}

    void FixedUpdate()
    {
    }

    void LateUpdate()
    {
        if (mState != null)
            mState.PostUpdate();
    }

    void OnDrawGizmos()
    {
        if (mState != null && mState.GetCode() == eUnitState.Attack)
        {
            AttackState attackState = mState as AttackState;
            UnitBase targetUnit = GameManager.Instance.FindUnit(attackState.mTargetUnitInfo);
            if(targetUnit != null)
            {
                Gizmos.color = Color.blue;
                Gizmos.DrawLine(transform.position, targetUnit.transform.position);
            }
        }
    }

    /// <summary>
    /// 사망 처리 시작
    /// </summary>
    public void BeginDestroy()
    {
        Log.Print(eLogFilter.Normal, string.Format("unit is dead({0})", mOId));
        CancelInvoke();
        Destroy(gameObject);
    }
    
    /// <summary>
    /// 유닛 초기화
    /// </summary>
    public void InitUnit(PathInfo _pathInfo)
    {
		UnitStat unitStat = new UnitStat();

		unitStat.mMoveSpeed 	= MoveSpeed;
		unitStat.mAttackSpeed 	= AttackSpeed;
		unitStat.mAttackDelay 	= AttackDelay;
		unitStat.mHP 			= HP;
		unitStat.mDefence 		= Defence;
		unitStat.mAttackPower 	= AttackPower;
		unitStat.mAttackRange 	= AttackRange;
		unitStat.mAttackType 	= AttackType;

		mStat = unitStat;
        mPathInfo = _pathInfo;

        //. 캐릭터 방향, 진영 설정.
        if(mPathInfo.pathDirection == ePathDirection.Forward)
        {
            mCamp = ePlayerCamp.Left;
            transform.Rotate(Vector3.up, 0.0f);
        }
        else
        {
            mCamp = ePlayerCamp.Right;
            transform.Rotate(Vector3.up, 180.0f);
        }

        //. 시작 포지션 설정
        transform.position = PathManager.Instance.GetStartPosition(_pathInfo.pathType, _pathInfo.pathDirection);

        //. 다음 포지션 미리 계산.
        PathManager.Instance.SetNextPathStep(mPathInfo, transform.position);

        //. 상태 변경
        ChangeState(eUnitState.Move);
    }

    /// <summary>
    /// 공격 받을때 처리
    /// </summary>
    public void BeAttacked(DamageInfo _info)
    {
        //. 데미지 계산
        float damageValue = _info.mDamage - mStat.mDefence;
        mStat.mHP -= damageValue;
        Log.Print(eLogFilter.Normal, string.Format(@"unit is attacked oId:{0} hp:{1} defence:{2} damageInfo{3}", mOId, mStat.mHP, mStat.mDefence, _info.mDamage));

        //. 피격 이펙트 재생

        //. 피격 사운드 재생

        //. 사망 처리
        if( mStat.mHP < 0 )
        {
            ChangeState(eUnitState.Dead);
            return;
        }
    }

    /// <summary>
    /// 상태 변경
    /// </summary>
    public void ChangeState(eUnitState _eUnitState, StateChangeEventArg _arg = null)
    {
        if (mState == null)
            return;

        if (mState.GetCode() == _eUnitState)
            return;

        UnitState newUnitState = CreateUnitState(_eUnitState);

        mState.OnStateExit();
        mState = newUnitState;
        mState.OnStateEnter(_arg);
    }

    /// <summary>
    /// 상태 생성 코드 
    /// 자식 클래스에서 스테이트 추가될게 있다면 여기서 변경
    /// </summary>
    public virtual UnitState CreateUnitState(eUnitState _eUnitState)
    {
        switch(_eUnitState)
        {
            case eUnitState.Attack: return new AttackState(this);
            case eUnitState.Dead: return new DeadState(this);
            case eUnitState.Idle: return new IdleState(this);
            case eUnitState.Move: return new MoveState(this);
        }

        Log.PrintError(eLogFilter.Normal, "invalid parameter (CreateUnitState)");
        return null;
    }

    /// <summary>
    /// 애니메이션 이벤트 핸들러
    /// </summary>
    /// <param name="_eAnimEvent">이벤트 종류</param>
    public void OnAnimationEvent(eAnimationEvent _eAnimEvent)
    {
        if (mState != null)
            mState.OnAnimationEvent(_eAnimEvent);
    }

    /// <summary>
    /// 이동 시작
    /// </summary>
    public void StartMove()
    {
        mRigidBody.velocity = mStat.mMoveSpeed * Time.deltaTime * mPathInfo.moveDirection;
    }

    /// <summary>
    /// 이동 멈춤
    /// </summary>
    public void StopMove()
    {
        mRigidBody.velocity = Vector2.zero;
    }

    /// <summary>
    /// 데미지 정보 반환 (나중에 버프 같은것도 여기서 계산)
    /// </summary>
    public DamageInfo GetDamangeInfo()
    {
        DamageInfo damageInfo = new DamageInfo();
        damageInfo.mDamage = mStat.mAttackPower;
        return damageInfo;
    }

    public bool IsDead()
    {
        return mState.GetCode() == eUnitState.Dead;
    }
}
