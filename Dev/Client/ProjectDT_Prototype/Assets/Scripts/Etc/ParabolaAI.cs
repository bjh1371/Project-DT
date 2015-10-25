using UnityEngine;
using System.Collections;

public class ParabolaAI : MonoBehaviour 
{
	public GameObject ExplosionObj;
	public GameObject SmokeLineTrailObj;
	
	public Vector3 VectorStartPoint = new Vector3(350, 50, 0);      //������ġ
	public float GoalPoint = 0.0f;                                  //������
	public float RandRangeMinValue = 0.0f;                          //�����ּҰ�
	public float RandRangeMaxValue = 0.0f;                          //�����ִ밪
	public float AccStep = 0.001f;	                                //���ӵ�
	public float WaveHeight = 30.0f;                                //������ 1����Ŭ �ִ� ��
	public int WaveCount = 0;                                       //������ ���� ���� ���� ���̺�Ƚ��. 0�̸� ����
    public int WaveDir = -1;                                        //������ ���� ����. 1 : ����, -1 : �Ʒ��� ����
	public int ElapsedTime = 900;                                   //����Ǵ� �ð�
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
        //������ y��ǥ�� �������� �ش�. ź������ �Ѹ� �� ��Ȯ�� ��ġ�� �ƴ� �л� ���·� �߻��ϱ� ����. �ʿ���ٸ� 0����
        float yPos = Random.Range(RandRangeMinValue, RandRangeMaxValue);                                    
		
		_VectorStartPoint = VectorStartPoint;
		_VectorEndPoint = new Vector3(GoalPoint, yPos, 0);
		_vecCur = _VectorStartPoint;

        //������ ���� ������ ���Ѵ�. ����� ������ ����
        _rPathRad = Mathf.Atan2(_VectorEndPoint.y - _VectorStartPoint.y, _VectorEndPoint.x - _VectorStartPoint.x);  
		_nPassedDistance = 0;

        //������ ���� �Ÿ��� ���Ѵ�
		_nDistance = Vector3.Distance(VectorStartPoint, _VectorEndPoint);
        

        /*�����ϴ� ���� ������ �ڵ�. ������ �����Ƿ� ����
        if (ShowSmokeTrail == true)
		{
			GameObject lineTrail = (GameObject)Instantiate(SmokeLineTrailObj, transform.position, Quaternion.Euler(0, 0, 0));
			_smokeTrailObj = (LineTrail)lineTrail.transform.gameObject.GetComponent("LineTrail");
		}*/
	}
	
	void Update ()
	{
        //������ ��ǥ�� ����. �ǽð����� �̹����� ȸ����Ű�� ����
		_vecPast.x = _vecCur.x;
		_vecPast.y = _vecCur.y;

        
		Vector3 pVector = new Vector3((_vecCur.x - _VectorStartPoint.x),(_vecCur.y - _VectorStartPoint.y));

        //���ӵ��� ����ϰ�..
        float acc = (Mathf.Sin((_nPassedDistance / _nDistance) * (Mathf.PI * WaveCount)) * WaveHeight) * WaveDir;

        _vecCur.x = _VectorStartPoint.x + _nPassedDistance;
		_vecCur.y = _VectorStartPoint.y + acc;
		_nPassedDistance += (_nDistance / ElapsedTime) + _nAccelerator;
		
        //���ӵ��� ���� ������ �ٽ� ��ġ�� ����Ѵ�
		pVector = new Vector3((_vecCur.x - _VectorStartPoint.x),(_vecCur.y - _VectorStartPoint.y));
		_vecCur = VectorRotation(_VectorStartPoint, pVector, _rPathRad);		
		_nAccelerator += _nAcc;
		
        //���⼭ ��������Ʈ ������ ������Ʈ
		transform.position = new Vector3(_vecCur.x, 0, VectorStartPoint.y + _vecCur.y);
		
        //������ ������Ʈ( ���� �������� �����ش�)
		transform.rotation = Quaternion.Euler(0, GetRotation(), 0);
		
		//if(ShowSmokeTrail == true) {_smokeTrailObj.PositionUpdate(transform.position);}
			
		if(transform.position.x < GoalPoint)
		{
            //����� �Ǵ� ������Ʈ�� �±׸� �ִ´�. ��) unit
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
            //UnitBase ���ӿ�����Ʈ�� �����ͼ� �浹 ó��
            //UnitBase collisionUnit = col.gameObject.GetComponent<UnitBase>();

            //�̺�Ʈ �ڵ� �����ε� �츮 ������Ʈ������ �ʿ������
            //HitToRocket();
            
			Destroy(gameObject);

            /*Ÿ�ݽ� ���� ����Ʈ ����. ������ ����*/

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