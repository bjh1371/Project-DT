using UnityEngine;
using System.Collections;

public class UnitState{
    protected UnitBase mParent = null;
    protected Animator mAnimator = null;
    public UnitState(UnitBase _parent)
    {
        mParent = _parent;
        mAnimator = _parent.GetComponent<Animator>();
        if( mAnimator == null )
        {
            Log.PrintError(eLogFilter.Normal, "Animator is null");
            Debug.Break();
        }
    }

    public virtual eUnitState GetCode() 
    { 
        return eUnitState.None; 
    }

    public virtual void OnStateEnter(StateChangeEventArg _arg) { }
    public virtual void OnStateExit() { }
    //public virtual void PreUpdate() { }
    public virtual void Update() { }
    public virtual void PostUpdate() { }

    public virtual void OnAnimationEvent(eAnimationEvent _eAnimEvent) { }
}
