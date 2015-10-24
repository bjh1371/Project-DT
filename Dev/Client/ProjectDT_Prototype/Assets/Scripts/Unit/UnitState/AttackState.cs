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

        //. 타겟이 죽었어도 다시 무브상태로
        UnitBase targetUnit = GameManager.Instance.FindUnit(mTargetUnitInfo);
        if (targetUnit == null || targetUnit.IsDead())
            mParent.ChangeState(eUnitState.Move);
        
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

            case eAnimationEvent.Fire: OnAnimationEvent_Fire();
                break;
        }
    }

    /// <summary>
    ///  활, 총알 발사 애니메이션 이벤트
    /// </summary>
    public virtual void OnAnimationEvent_Fire()
    {
        if (mParent.mProjectileObj == null)
            return;

        //. 발사체 생성
        GameObject projectileObj = Object.Instantiate(mParent.mProjectileObj);
        if (projectileObj == null)
            return;

        projectileObj.transform.position = mParent.mFirePosition.position;
        projectileObj.transform.rotation = mParent.mFirePosition.rotation;
        ProjectileBase projectileBase = projectileObj.GetComponent<ProjectileBase>();
        if (projectileBase == null)
            return;

        //. 타겟 유닛 서치, 중간에 죽었을수도 있으니깐~
        UnitBase targetUnit = GameManager.Instance.FindUnit(mTargetUnitInfo);
        if (targetUnit == null)
            return;

        //. 타겟 정보 셋팅
        projectileBase.mTargetUnit = targetUnit;
        projectileBase.mOwnerUnit = mParent;
        projectileBase.mDamageInfo = mParent.GetDamangeInfo();
    }

    /// <summary>
    /// 공격 애니메이션 이벤트
    /// </summary>
    public virtual void OnAnimationEvent_Attack()
    {
        UnitBase targetUnit = GameManager.Instance.FindUnit(mTargetUnitInfo);
        
        //. 타겟이 죽었으면 다시 Move
        if (targetUnit == null || targetUnit.IsDead())
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
