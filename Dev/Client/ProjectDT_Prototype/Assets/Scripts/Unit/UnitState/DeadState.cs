using UnityEngine;
using System.Collections;

/// <summary>
/// 죽은 상태
/// </summary>
public class DeadState : UnitState
{
    public DeadState(UnitBase _parent) : base(_parent)
    {
    }

    public override eUnitState GetCode()
    {
        return eUnitState.Dead;
    }

    public override void OnStateEnter(StateChangeEventArg _arg)
    {
        mAnimator.SetTrigger("dead");
        GameManager.Instance.OnDeadUnitNotify(mParent);
    }
    
    public override void OnAnimationEvent(eAnimationEvent _eAnimEvent)
    {
        //. 사망 처리
        if(_eAnimEvent == eAnimationEvent.AnimationEnd)
        {
            float deadProcessDelaySecond = 1.0f;
            mParent.Invoke("BeginDestroy", deadProcessDelaySecond);
        }
    }
}
