using UnityEngine;
using System.Collections;
using System.Reflection;

/// <summary>
/// 유닛의 공격 상태
/// </summary>
public class AttackState : UnitState{
    public UnitFindInfo mTargetUnitInfo { get; protected set; }

    public AttackState(UnitBase _parent) : base(_parent)
    {
    }
   
    public override eUnitState GetCode()
    {
        return eUnitState.Attack;
    }

    public override void OnStateEnter(StateChangeEventArg _arg)
    {
        //. 대상 유닛이 없으면 다시 무브상태로 
        mTargetUnitInfo = _arg.mUnitFindInfo;
        if (mTargetUnitInfo == null)
        {
            mParent.ChangeState(eUnitState.Move);
            return;
        }

        mAnimator.speed *= mParent.mStat.mAttackSpeed;
        mAnimator.SetTrigger("attack");
        mParent.StopMove();
    }

    public override void OnStateExit()
    {
        //Log.PrintLog(eLogFilter.Normal, string.Format("AttackState Exit Oid{0}", mParent.mOId));
        mAnimator.speed = 1.0f;
    }

    /// <summary>
    /// 애니메이션 이벤트 핸들러
    /// </summary>
    public override void OnAnimationEvent(eAnimationEvent _eAnimEvent)
    {
        switch(_eAnimEvent)
        {
            case eAnimationEvent.Attack: OnAnimationEvent_Attack();
                break;

            case eAnimationEvent.AnimationEnd: OnAnimationEvent_End(); 
                break;
        }
    }

    public virtual void OnAnimationEvent_Attack()
    {
        UnitBase targetUnit = GameManager.Instance.FindUnit(mTargetUnitInfo);
        
        //. 타겟이 죽었으면 다시 Move
        if (targetUnit == null || targetUnit.mState.GetCode() == eUnitState.Dead)
            mParent.ChangeState(eUnitState.Move);

        //. 대상 유닛 공격
        DamageInfo damageInfo = new DamageInfo();
        damageInfo.mDamage = mParent.mStat.mAttackPower;
        targetUnit.BeAttacked(damageInfo);
    }

    public virtual void OnAnimationEvent_End()
    {
        StateChangeEventArg arg = new StateChangeEventArg();
        arg.mTime = mParent.mStat.mAttackDelay;
        mParent.ChangeState(eUnitState.Idle, arg);
    }
}
