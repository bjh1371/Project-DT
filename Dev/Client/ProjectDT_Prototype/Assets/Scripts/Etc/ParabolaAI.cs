using UnityEngine;
using System.Collections;

public class ParabolaAI : MonoBehaviour 
{
	public GameObject ExplosionObj;
	public GameObject SmokeLineTrailObj;
	
	public Vector3 VectorStartPoint = new Vector3(350, 50, 0);      //시작위치
	public float GoalPoint = 0.0f;                                  //목적지
	public float RandRangeMinValue = 0.0f;                          //랜덤최소값
	public float RandRangeMaxValue = 0.0f;                          //랜덤최대값
	public float AccStep = 0.001f;	                                //가속도
	public float WaveHeight = 30.0f;                                //포물선 1사이클 최대 폭
	public int WaveCount = 0;                                       //목적지 까지 가는 동안 웨이브횟수. 0이면 직진
    public int WaveDir = -1;                                        //포물선 시작 방향. 1 : 위쪽, -1 : 아래쪽 부터
	public int ElapsedTime = 900;                                   //진행되는 시간
	public bool ShowSmokeTrail = true;
	
	private float _nAcc;
	private float _nDistance = 0.0f;
	private float _nPassedDistance = 0.0f;
	private float _rPathRad = 0.0f;
	private float _nAccelerator = 0.0f;
	//private float _nMoveStepToEffect = 0.0f;
	
	private Vector3 _VectorStartPoint;
	private Vector3 _VectorEndPoint;
	private Vector3 _vecCur;
	private Vector3 _vecPast = new Vector3();
	
	//private LineTrail _smokeTrailObj;
	
	public delegate void EvOnHitRocket();
	public static event EvOnHitRocket HitToRocket;
	
	void Start () 
	{
		_nAcc = AccStep;
        //목적지 y좌표에 랜덤값을 준다. 탄막으로 뿌릴 때 정확한 위치가 아닌 분사 형태로 발사하기 위함. 필요없다면 0으로
        float yPos = Random.Range(RandRangeMinValue, RandRangeMaxValue);                                    
		
		_VectorStartPoint = VectorStartPoint;
		_VectorEndPoint = new Vector3(GoalPoint, yPos, 0);
		_vecCur = _VectorStartPoint;

        //목적지 까지 각도를 구한다. 행렬을 돌리기 위함
        _rPathRad = Mathf.Atan2(_VectorEndPoint.y - _VectorStartPoint.y, _VectorEndPoint.x - _VectorStartPoint.x);  
		_nPassedDistance = 0;

        //목적지 까지 거리를 구한다
		_nDistance = Vector3.Distance(VectorStartPoint, _VectorEndPoint);
        

        /*추적하는 라인 생성용 코드. 지금은 없으므로 삭제
        if (ShowSmokeTrail == true)
		{
			GameObject lineTrail = (GameObject)Instantiate(SmokeLineTrailObj, transform.position, Quaternion.Euler(0, 0, 0));
			_smokeTrailObj = (LineTrail)lineTrail.transform.gameObject.GetComponent("LineTrail");
		}*/
	}
	
	void Update ()
	{
        //지나온 좌표를 저장. 실시간으로 이미지를 회전시키기 위함
		_vecPast.x = _vecCur.x;
		_vecPast.y = _vecCur.y;

        
		Vector3 pVector = new Vector3((_vecCur.x - _VectorStartPoint.x),(_vecCur.y - _VectorStartPoint.y));

        //가속도를 계산하고..
        float acc = (Mathf.Sin((_nPassedDistance / _nDistance) * (Mathf.PI * WaveCount)) * WaveHeight) * WaveDir;

        _vecCur.x = _VectorStartPoint.x + _nPassedDistance;
		_vecCur.y = _VectorStartPoint.y + acc;
		_nPassedDistance += (_nDistance / ElapsedTime) + _nAccelerator;
		
        //가속도를 구한 값에서 다시 위치를 계산한다
		pVector = new Vector3((_vecCur.x - _VectorStartPoint.x),(_vecCur.y - _VectorStartPoint.y));
		_vecCur = VectorRotation(_VectorStartPoint, pVector, _rPathRad);		
		_nAccelerator += _nAcc;
		
        //여기서 스프라이트 포지션 업데이트
		transform.position = new Vector3(_vecCur.x, 0, VectorStartPoint.y + _vecCur.y);
		
        //각도도 업데이트( 가는 방향으로 꺽어준다)
		transform.rotation = Quaternion.Euler(0, GetRotation(), 0);
		
		//if(ShowSmokeTrail == true) {_smokeTrailObj.PositionUpdate(transform.position);}
			
		if(transform.position.x < GoalPoint)
		{
            //대상이 되는 오브젝트의 태그를 넣는다. 예) unit
            if (gameObject.tag == "RocketBullet")
			{
				Destroy(this.gameObject);
				//if(ShowSmokeTrail == true){_smokeTrailObj.Kill();}
			}
		}
	}
		
	void OnTriggerEnter2D(Collider2D col)
	{
		if(col.gameObject.tag == "Player")
		{
            //UnitBase 게임오브젝트를 가져와서 충돌 처리
            //UnitBase collisionUnit = col.gameObject.GetComponent<UnitBase>();

            //이벤트 코드 실행인데 우리 프로젝트에서는 필요없을듯
            //HitToRocket();
            
			Destroy(gameObject);

            /*타격시 폭발 이펙트 생성. 지금은 없다*/

			//ExplosionObj.transform.position = transform.position;
			//Instantiate(ExplosionObj);


			//if(ShowSmokeTrail == true)
			//{
			//	_smokeTrailObj.Kill();
			//}
		}
	}
	
	public float GetRotation() { return Mathf.Atan2(_vecCur.y - _vecPast.y, _vecCur.x - _vecPast.x) * -180 / Mathf.PI; }
    public float GetRotationRad() { return Mathf.Atan2(_vecCur.y - _vecPast.y, _vecCur.x - _vecPast.x); }    
	public Vector3 GetPosition() { return _vecCur; }

	private Vector3 VectorRotation(Vector3 VectorStartPoint, Vector3 pProcess, float radian)
	{
		float cosTheta = Mathf.Cos(radian);
		float sinTheta = Mathf.Sin(radian);
		return new Vector3 (VectorStartPoint.x + (cosTheta * pProcess.x - sinTheta * pProcess.y), VectorStartPoint.y + (sinTheta * pProcess.x  + cosTheta * pProcess.y));
	}
}