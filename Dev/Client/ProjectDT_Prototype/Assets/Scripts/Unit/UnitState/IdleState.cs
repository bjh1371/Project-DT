using UnityEngine;
using System.Collections;

public class IdleState : UnitState 
{
    private float mDelayTime;       /// 몇초 동안 대기 상태에 있을것인가?
    public IdleState(UnitBase _parent) : base(_parent) { }

    public override eUnitState GetCode()
    {
        return eUnitState.Idle;
    }

    public override void OnStateEnter(StateChangeEventArg _arg)
    {
        mDelayTime = _arg.mTime;
        mParent.StopMove();
        mAnimator.SetTrigger("idle");
    }

    public override void Update()
    {
        mDelayTime -= Time.deltaTime;
        if (mDelayTime <= 0)
            mParent.ChangeState(eUnitState.Move);
    }
}
