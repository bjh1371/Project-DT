using UnityEngine;
using System.Collections;
using System.Collections.Generic;


/// <summary>
/// 패스 종류
/// </summary>
public enum ePathType
    {
    Upper,        /// 위쪽 패스
    Lower,        /// 아래쪽 패스
}

/// <summary>
///  패스 방향
/// </summary>
public enum ePathDirection:byte
{
    Forward,     /// 정방향 ( left to right )
    Reverse,     /// 역방향 ( right to left )
}

/// <summary>
/// 패스 정보
/// </summary>
public class PathInfo
{
    public PathInfo(ePathType _pathType, ePathDirection _pathDirection)
    {
        pathType = _pathType;
        pathDirection = _pathDirection;
    }

    public ePathType pathType { get; set; }             /// 패스 타입
    public ePathDirection pathDirection { get; set; }   /// 패스 방향
    public int stepIndex { get; set; }                  /// 현재 스텝 인덱스
    public Vector3 nextPosition { get; set; }           /// 다음 이동 위치
    public Vector3 moveDirection { get; set; }          /// 이동 방향
}

/// <summary>
/// 패스 매니저
/// </summary>
public class PathManager : MonoBehaviour 
{
    public static PathManager Instance;

    public Transform mUpperPath;      /// 위쪽 패스
    public Transform mLowerPath;      /// 아래쪽 패스
    void Awake() 
    {
        if (Instance == null)
            Instance = this;
        else
            Log.PrintError(eLogFilter.PathManager, "PathManager has two instances");
    }

    /// <summary>
    /// 시작 위치 반환
    /// </summary>
    public Vector3 GetStartPosition(ePathType _pathType, ePathDirection _pathDirection)
    {
        //. 시작 위치 계산
        int startIndex = 0;
        Vector3 startPosition = Vector3.zero;
        if( GetPathPosition(_pathType, _pathDirection, startIndex, ref startPosition) == false)
            Log.PrintError(eLogFilter.PathManager, "Could not calculate start position");
        
        return startPosition;
    }

    /// <summary>
    /// 다음 패스 위치 정보 셋팅
    /// </summary>
    public bool SetNextPathStep(PathInfo _pathInfo, Vector3 _characterPosition)
    {        
        //. 다음 이동 위치 찾기
        int nextStepIndex = _pathInfo.stepIndex + 1;
        Vector3 nextPosition = Vector3.zero;
        if (GetPathPosition(_pathInfo.pathType, _pathInfo.pathDirection, nextStepIndex, ref nextPosition) == false)
            return false;

        //. 이동 위치 및 이동방향 계산
        _pathInfo.nextPosition = nextPosition;
        _pathInfo.moveDirection = (nextPosition - _characterPosition).normalized;
        _pathInfo.stepIndex++;
        return true;
    }

    /// <summary>
    /// 다음 위치에 도달 했나??
    /// </summary>
    public bool IsArriveNextPosition(PathInfo _pahtInfo, Vector3 _characterPosition)
    {
        if (_pahtInfo.pathDirection == ePathDirection.Forward )
        {
            return (_pahtInfo.nextPosition.x < _characterPosition.x);
        }
        else if( _pahtInfo.pathDirection == ePathDirection.Reverse )
        {
            return (_pahtInfo.nextPosition.x > _characterPosition.x);
        }

        Debug.Break();
        throw new System.Exception("path direction enum value is invalid");
    }

    /// <summary>
    /// 패스 위치 반환
    /// </summary>
    private bool GetPathPosition(ePathType _pathType, ePathDirection _pathDirection, int _stepIndex, ref Vector3 _pathPosition)
    {    
        //. 위쪽 or 아래쪽
        Transform pathList = null;
        if (_pathType == ePathType.Lower)
            pathList = mLowerPath;
        else if (_pathType == ePathType.Upper)
            pathList = mUpperPath;

        //. 유효성 검사
        if (pathList == null)
        {
            Log.PrintError(eLogFilter.PathManager, "Unexpected parameter (PathType)");
            return false;
        }

        //. 유효성 검사
        if (_stepIndex < 0 || _stepIndex >= pathList.childCount)
        {
            return false;
        }
            
        //. 정방향
        if( _pathDirection == ePathDirection.Forward)
        {
            _pathPosition = pathList.GetChild(_stepIndex).position;
            return true;
        }
        //. 역방향
        else if( _pathDirection == ePathDirection.Reverse)
        {
            _pathPosition = pathList.GetChild(pathList.childCount - 1 - _stepIndex).position;
            return true;
        }

        Log.PrintError(eLogFilter.PathManager, "Unexpected parameter (PathDirection)");
        return false;
    }
}
