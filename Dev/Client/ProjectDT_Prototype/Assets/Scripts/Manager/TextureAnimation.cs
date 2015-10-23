using UnityEngine;
using System.Collections;

public class TextureAnimation : MonoBehaviour 
{
	public int FrameUpdateSpeed = 1;
	public int FrameCount = 1;
	public float OffSetCount;
	
	private float _updatePastTime;
	private float _accumTimeAterUpdate;
	private int _curFrame;	
    private Vector2 _texOffset;
	
	private Material _material;
	
	void Start () 
	{
		_material = gameObject.GetComponent<Renderer>().material;
        _texOffset = _material.GetTextureOffset("_MainTex");
		_updatePastTime = 1.0f / FrameUpdateSpeed;
	}
	
	void Update () 
	{
		_accumTimeAterUpdate += Time.deltaTime;
		
		if(_accumTimeAterUpdate >= _updatePastTime)
		{
			_curFrame = (_curFrame + 1) % FrameCount;
			_texOffset.x = _curFrame * OffSetCount;
	        _material.SetTextureOffset("_MainTex", _texOffset);
			_accumTimeAterUpdate = 0.0f;
		}
	}
}
