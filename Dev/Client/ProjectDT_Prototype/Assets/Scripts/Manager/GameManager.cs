using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System;

/// <summary>
/// 유닛 진영 정보
/// </summary>
public class UnitCampData
{
    public UnitLaneData mUpperLane { get; protected set; }  /// 위쪽 라인
    public UnitLaneData mLowerLane { get; protected set; }  /// 아래쪽 라인
    //public UnitLane mThridLane                        /// 얘비 라인

    public UnitCampData()
    {
        mUpperLane = new UnitLaneData();
        mLowerLane = new UnitLaneData();
    }
}

/// <summary>
/// 유닛 라인 정보
/// </summary>
public class UnitLaneData
{
    public Dictionary<ushort, UnitBase> mUnits { get; set; }
    public UnitLaneData()
    {
        mUnits = new Dictionary<ushort, UnitBase>();
    }

    /// <summary>
    /// 가장 가까운 공격 대상 반환
    /// </summary>
    /// <param name="_unit">기준 유닛</param>
    /// <param name="_dampingFactor">범위 감쇠값 (다른 라인 유닛을 검색할때 사용)</param>
    public UnitBase GetCloseAttackTarget(UnitBase _unit, float _dampingFactor = 1.0f)
    {
        //. 검사 범위 계산 min - max 
        Vector3 callerPos = _unit.transform.position;
        int searchDirection = (_unit.mCamp == ePlayerCamp.Right) ? -1 : 1;  //. Right면 오른쪽에서 왼쪽으로 검사
        float range = callerPos.x + (_unit.mStat.mAttackRange * searchDirection);

        float min = Mathf.Min(callerPos.x, range);
        float max = Mathf.Max(callerPos.x, range);

        //. 범위 감쇠값 적용
        if(_dampingFactor != 1.0f)
        {
            float dist = min - max;
            float offset = (dist * _dampingFactor) * 0.5f;
            min += offset;
            max -= offset;
        }

        UnitBase closeTarget = null;
        foreach (var item in mUnits)
        {
            UnitBase enemy = item.Value;
            if (enemy == null)
                continue;

            //. 범위 안에 있나??
            Vector3 enemyPos = enemy.transform.position;
            if (min <= enemyPos.x && enemyPos.x <= max)
            {
                //. 이미 찾았던 유닛이 있으면 누가 더 가까운지 검사
                if (closeTarget != null)
                {
                    float targetDist = Mathf.Abs(closeTarget.transform.position.x - callerPos.x);
                    float enemyDist = Mathf.Abs(enemyPos.x - callerPos.x);
                    if (enemyDist < targetDist)
                        closeTarget = enemy;
                }
                //. 없으면 바로 할당
                else
                {
                    closeTarget = enemy;
                }
            }
        }

        return closeTarget;
    }
}

public class GameManager : MonoBehaviour {
    public static GameManager Instance;
    public GameObject testUnit1;
    public GameObject testUnit2;

    private ushort mTempOId;               /// 스테이지 전용 임시 OId
    private UnitCampData mLeftCamp;        /// 왼쪽 진영 데이터
    private UnitCampData mRightCamp;       /// 오른쪽 진영 데이터
    private ePlayerCamp mLocalPlayerCamp;  /// Local 플레이어 진영

    void Awake()
    {
        if (Instance == null)
            Instance = this;
        else
            Log.PrintError(eLogFilter.GameManager, "Game manager has two instances");
    }

	// Use this for initialization
	void Start () {
        mLocalPlayerCamp = ePlayerCamp.Left;

        mLeftCamp = new UnitCampData();
        mRightCamp = new UnitCampData();
        mTempOId = 1;
	}
	
	// Update is called once per frame
	void Update () {

        //. Unit 생성 테스트 코드
        if(Input.anyKeyDown )
        {
            if (Input.GetKey(KeyCode.UpArrow))
            {
                CreateUnit(1, new PathInfo(ePathType.Upper, ePathDirection.Forward));
                CreateUnit(2, new PathInfo(ePathType.Upper, ePathDirection.Reverse));
            }
            else if(Input.GetKey(KeyCode.DownArrow))
            {
                CreateUnit(1, new PathInfo(ePathType.Lower, ePathDirection.Forward));
                CreateUnit(2, new PathInfo(ePathType.Lower, ePathDirection.Reverse));
            }
        }
	}

    /// <summary>
    /// 아군 or 적군 유닛 라인 데이터 반환
    /// </summary>
    private UnitLaneData GetUnitLane(ePlayerCamp _camp, ePathType _pathType)
    {
        UnitCampData campData = null;
        if (_camp == ePlayerCamp.Left)
            campData = mLeftCamp;
        else if (_camp == ePlayerCamp.Right)
            campData = mRightCamp;

        if (campData == null)
        {
            Log.PrintLog(eLogFilter.GameManager, "Unexpected parameter (_camp)");
            return null;
        }

        if (_pathType == ePathType.Lower)
            return campData.mLowerLane;
        else if (_pathType == ePathType.Upper)
            return campData.mUpperLane;

        Log.PrintError(eLogFilter.GameManager, "Unknown case.. _pathType should be lower or upper.");
        return null;        
    }

    /// <summary>
    /// 적군인가?? (LocalPlayer 기준)
    /// </summary>
    private bool IsEnemyUnit(ePlayerCamp _camp)
    {
        return (mLocalPlayerCamp != _camp);
    }

    /// <summary>
    /// 유닛 생성 통지
    /// </summary>
    public void OnCreateUnitNotify(UnitBase _unit)
    {
        string log = string.Format("OnCreateUnit camp({0}), pathType({1})", _unit.mCamp, _unit.mPathInfo.pathType);
        Log.PrintLog(eLogFilter.GameManager, log);
        UnitLaneData unitLane = GetUnitLane(_unit.mCamp, _unit.mPathInfo.pathType);

        //. Dict에 추가
        if (unitLane != null)
        {
            unitLane.mUnits.Add(mTempOId, _unit);
            _unit.mOId = mTempOId;
            mTempOId++;
        }
    }
    
    /// <summary>
    /// 유닛 제거 통지
    /// </summary>
    public void OnDeadUnitNotify(UnitBase _unit)
    {
        string log  = string.Format("OnDestroyUnit camp({0}), pathType({1})", _unit.mCamp, _unit.mPathInfo.pathType);
        Log.PrintLog(eLogFilter.GameManager, log);
        UnitLaneData unitLane = GetUnitLane(_unit.mCamp, _unit.mPathInfo.pathType);

        //. Dict 에서 제거
        if (unitLane != null)
            unitLane.mUnits.Remove(_unit.mOId);
    }

    /// <summary>
    /// 유닛 만들기
    /// </summary>
    public void CreateUnit(int _unitClassID, PathInfo _pathInfo)
    {
        //. Todo 유닛 목록에서 classID 맞는 스탯 정보 Get

        //. 임시 테스트 데이터
        UnitStat unitstat = null;
        PathInfo unitpath = null;
        GameObject template = null;
        if( _unitClassID == 1 )
        {
            unitstat = new UnitStat();
            unitstat.mMoveSpeed = UnityEngine.Random.Range(0.5f, 3.0f);
            unitstat.mAttackSpeed = UnityEngine.Random.Range(0.5f, 2.0f);
            unitstat.mAttackDelay = UnityEngine.Random.Range(0.0f, 1.0f);
            unitstat.mHP = UnityEngine.Random.Range(40, 15);
            unitstat.mDefence = 1;
            unitstat.mAttackPower = 3;
            unitstat.mAttackRange = 1;
            unitstat.mAttackType = eAttackType.Melee;

            template = testUnit1;
            unitpath = _pathInfo;
        }
        else
        {
            unitstat = new UnitStat();
            unitstat.mMoveSpeed = UnityEngine.Random.Range(0.5f, 3.0f);
            unitstat.mAttackSpeed = UnityEngine.Random.Range(0.5f, 2.0f);
            unitstat.mAttackDelay = UnityEngine.Random.Range(0.0f, 1.0f);
            unitstat.mHP = UnityEngine.Random.Range(40, 15);
            unitstat.mDefence = 1;
            unitstat.mAttackPower = 3;
            unitstat.mAttackRange = 1;
            unitstat.mAttackType = eAttackType.Melee;

            template = testUnit2;
            unitpath = _pathInfo;
        }

        GameObject unit = Instantiate(template);
        UnitBase unitInstance = unit.GetComponent<UnitBase>();
        unitInstance.InitUnit(unitstat, unitpath);       

        //. 생성 통지
        OnCreateUnitNotify(unitInstance);
    }

    /// <summary>
    /// 유닛 검색
    /// </summary>
    public UnitBase FindUnit(UnitFindInfo _findInfo)
    {
        UnitLaneData unitLane = GetUnitLane(_findInfo.mCamp, _findInfo.mPathType);
        if (unitLane == null)
            return null;

        UnitBase result = null;
        unitLane.mUnits.TryGetValue(_findInfo.mOid, out result);
        return result;
    }
   
    /// <summary>
    /// 공격 대상 검색
    /// </summary>
    /// <param name="_caller">호출한 유닛</param>
    /// <returns>공격 대상 Unit, 없으면 null</returns>
    public UnitBase FindAttackTarget(UnitBase _caller)
    {
        //. 적군 유닛 진영 
        ePlayerCamp eEnemyCamp; //. caller 기준에서 적군 진영
        if (_caller.mCamp == ePlayerCamp.Left)
            eEnemyCamp = ePlayerCamp.Right;
        else
            eEnemyCamp = ePlayerCamp.Left;

        UnitLaneData sameUnitLane = GetUnitLane(eEnemyCamp, _caller.mPathInfo.pathType);
        if(sameUnitLane == null)
            return null;

        UnitBase closeTarget = sameUnitLane.GetCloseAttackTarget(_caller);

        //. 레인지 유닛이면 다른 라인도 검사
        if(_caller.mStat.mAttackType == eAttackType.Range)
        {
            ePathType eOtherPathType = (_caller.mPathInfo.pathType == ePathType.Lower) ? ePathType.Upper : ePathType.Lower;
            UnitLaneData otherUnitLane = GetUnitLane(eEnemyCamp, eOtherPathType);
            if (otherUnitLane == null)
                return closeTarget;

            const float dampingFactor = 0.8f;   //. 범위 체크 감쇠값 (임시)
            UnitBase otherCloseTarget = otherUnitLane.GetCloseAttackTarget(_caller, dampingFactor);
            if (otherCloseTarget == null)
                return closeTarget;

            //. 같은 라인에 없었으면 바로 리턴
            if (closeTarget == null)
                return otherCloseTarget;

            //. 누가 더 가까운가??
            float otherLaneDist = Mathf.Abs(otherCloseTarget.transform.position.x - _caller.transform.position.x);
            float sameLaneDist = Mathf.Abs(closeTarget.transform.position.x - _caller.transform.position.x);
            if (otherLaneDist < sameLaneDist)
                closeTarget = otherCloseTarget;
        }

        return closeTarget;
    }
}
