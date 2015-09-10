using UnityEngine;
using System.Collections;

public class MoveState : UnitState {

    public MoveState(UnitBase _parent) : base(_parent) {}
    public override eUnitState GetCode()
    {
        return eUnitState.Move;
    }

    public override void OnStateEnter(StateChangeEventArg _arg)
    {
        mAnimator.SetTrigger("move");
    }

    public override void OnStateExit()
    {
    }

    public override void Update()
    {
        //. 다음 목적지에 도착 했나??
        PathInfo pathInfo = mParent.mPathInfo;
        Vector3 charPosition = mParent.transform.position; 
        if( PathManager.Instance.IsArriveNextPosition(pathInfo, charPosition) )
        {
            //. 도착 했으면 다음 목적지 셋팅
            PathManager.Instance.SetNextPathStep(pathInfo, charPosition);
        }

        //. 적이 있는지 검사
        UnitBase attackTarget = GameManager.Instance.FindAttackTarget(mParent);
        if( attackTarget != null && attackTarget.IsDead() == false)
        {
            //. 공격 상태로 변경
            UnitFindInfo findInfo = new UnitFindInfo();
            findInfo.mCamp = attackTarget.mCamp;
            findInfo.mOid = attackTarget.mOId;
            findInfo.mPathType = attackTarget.mPathInfo.pathType;
            mParent.ChangeState(eUnitState.Attack, new StateChangeEventArg() { mUnitFindInfo = findInfo });
            return;
        }

        //. 이동
        mParent.StartMove();
    }
}
