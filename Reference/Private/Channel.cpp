#include "Channel.h"
#include "Bone.h"

CChannel::CChannel()
{
}

HRESULT CChannel::Initialize(const string strName, vector<KEYFRAME>& Keyframes, _int iBoneIndex)
{
	strcpy_s(m_szName, strName.c_str());

	m_KeyFrames.reserve(Keyframes.size());

	for (auto& iter : Keyframes)
		m_KeyFrames.push_back(iter);

	m_iNumKeyFrames = m_KeyFrames.size();

	m_iBoneIndex = iBoneIndex;
	return S_OK;

}

_bool CChannel::Blend_TransformationMatrix(_float fCurrentTrackPosition, const CModel::BONES& Bones, _uint* pCurrentKeyFrame, KEYFRAME& pPrevKeyFrame)
{
		
		_vector		vScale;
		_vector		vRotation;
		_vector		vPosition;

		_float3		vSourScale, vDestScale;
		_float4		vSourRotation, vDestRotation;
		_float3		vSourPosition, vDestPosition;

		vSourScale	  = pPrevKeyFrame.vScale;
		vSourRotation = pPrevKeyFrame.vRotation;
		vSourPosition = pPrevKeyFrame.vPosition;

		vDestScale	  = m_KeyFrames[*pCurrentKeyFrame].vScale;
		vDestRotation = m_KeyFrames[*pCurrentKeyFrame].vRotation;
		vDestPosition = m_KeyFrames[*pCurrentKeyFrame].vPosition;

		_float	fRatio = 0.f;

		fRatio = fCurrentTrackPosition / 0.2f;

		if (fRatio >= 1.f)
		{
			return true;
		}

		vScale	  = XMVectorLerp(XMLoadFloat3(&vSourScale), XMLoadFloat3(&vDestScale), fRatio);
		vRotation = XMQuaternionSlerp(XMLoadFloat4(&vSourRotation), XMLoadFloat4(&vDestRotation), fRatio);
		vPosition = XMVectorLerp(XMLoadFloat3(&vSourPosition), XMLoadFloat3(&vDestPosition), fRatio);
	

	//! 위에서 저 상태들로 뭐하려고한거야? 시간에맞는 TransformationMatrix 만들어주려고 했던거잖아?
	//! 어떻게 만들거야? 로테이션은 쿼터니언인데? 이것도 함수있어 개꿀 ㅋㅋ
	//! 응? 인자값으로 로테이션 원점을 달라는데? 원점이뭐지? 0,0,0,1 이잖아 주면되지 ㅋㅋ
	_matrix	TransformationMatrix = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f, 0.f, 0.f, 1.f), vRotation, vPosition);

	//! 이제 이 채널과 같은 이름을 가진 뼈를 찾아서 그 뼈의 TransformationMatrix 갱신해줄거야.
	Bones[m_iBoneIndex]->Set_TransformationMatrix(TransformationMatrix);

	
	return false;
}


void CChannel::Invalidate_TransformationMatrix(_float fCurrentTrackPosition, const CModel::BONES& Bones, _uint* pCurrentKeyFrame)
{
	//! 이전 애니메이션의 키프레임
	

	//! 커런트 트랙포지션이 0.0으로 들어오는 경우는 루프돌릴때 말고는 없어. 그러니까 인덱스를 0으로 초기화 시켜주자
	if(0.0f == fCurrentTrackPosition)
		*pCurrentKeyFrame = 0;

	_vector		vScale;
	_vector		vRotation;
	_vector		vPosition;

	KEYFRAME	LastKeyFrame = m_KeyFrames.back(); //! 애니메이션의 마지막 프레임을 얘기하는거겠지?

	//! 마지막 키프레임은 해당애니메이션의 전체길이인 듀레이션이 종료될때까지 자기 상태를 유지시켜줘야겠지?
	if (fCurrentTrackPosition >= LastKeyFrame.fTrackPosition)
	{
		vScale = XMLoadFloat3(&LastKeyFrame.vScale);
		vRotation = XMLoadFloat4(&LastKeyFrame.vRotation);
		vPosition = XMLoadFloat3(&LastKeyFrame.vPosition);
	}
	
	//! 이제 현재 프레임과 다음 프레임과 선형보간을 통해 현재의 상태를 갱신시켜줘야겠지?
	else //! 마지막 프레임의 다음프레임은 없잖아? 그래서 현재 프레임 +1이라는 조건이 말이 안돼. 근데 위에서 if로 걸러줬잖아. else 해주자
	{
		//! 점프라는 애니메이션이 있다고 치자. 점프하려면 다리의 힘을 주는 애니메이션이 0번 프레임이라고 해보자고.
		//! 현재 애니메이션 재생위치가 0번프레임보다 커졌다면 무릎이 펴진다는 애니메이션이 1번 프레임이라면 1번프레임으로 바뀌어야겠지?
		
		//TODO While문을 사용한 이유.
		//! 프레임이 떨어지면 메시가 깨지는 현상이 발생해.
		//! 왜 깨지는걸까? 모델에 Play_Animation 함수를 호출하면서 내부에서 Invalidate_Combie 함수를 호출해서 해당 뼈의 Combine 행렬을 갱신해주고있었어
		//! 그런데 프레임이 떨어지면서 넘겨주는 인자값이었던 TimeDelta 값이 생각보다 너무 큰 값이 넘겨져 버린거야.
		//! 그래서, 프레임 인덱스가 한번에 건너뛰어버린거지. 예를 들자면 1에서 한번에 3으로.
		//! 여기서 다음 키프레임은 2로 만들어져버린거야. 4가아니라. 여기서 참사가 발생한거야 이걸 막기위해 While문을 돌려준거지.
		
		
		while(fCurrentTrackPosition >= m_KeyFrames[*pCurrentKeyFrame + 1].fTrackPosition)
			++*pCurrentKeyFrame;

		//현재 키프레임 0

		//! 소스와 데스트야. 소스는 현재 프레임인 0번 프레임이 가진 상태를 의미하고, 데스트는 다음 프레임인 1번 프레임의 상태를 의미해.
		_float3		vSourScale, vDestScale;
		_float4		vSourRotation, vDestRotation;
		_float3		vSourPosition, vDestPosition;

		vSourScale = m_KeyFrames[*pCurrentKeyFrame].vScale;
		vSourRotation = m_KeyFrames[*pCurrentKeyFrame].vRotation;
		vSourPosition = m_KeyFrames[*pCurrentKeyFrame].vPosition;
		
		vDestScale = m_KeyFrames[*pCurrentKeyFrame + 1].vScale;
		vDestRotation = m_KeyFrames[*pCurrentKeyFrame + 1].vRotation;
		vDestPosition = m_KeyFrames[*pCurrentKeyFrame + 1].vPosition;


		//! XMVectorLerp라는 함수는 아주 기특하게도 비율만 넣어주면 선형보간을 예쁘게 해준단말이지. 비율 구하자
		//!  fCurrentTrackPosition 이 0.5였다고 치자. 다음 애니메이션의 CurrentTrackPosition 0일거다. 이때 0과의 비율을 구할수없으니. 0.2~0.3 으로 임의의 보간값을 줘서 구한다.
		//! 다음 애니메이션의 커런트트랙포지션은 넘겨받아야지.
		_float	fRatio = 0.f;
			
			fRatio = (fCurrentTrackPosition - m_KeyFrames[*pCurrentKeyFrame].fTrackPosition) /
				(m_KeyFrames[*pCurrentKeyFrame + 1].fTrackPosition - m_KeyFrames[*pCurrentKeyFrame].fTrackPosition);


		//! 1번째 인자와 2번째 인자를 비율만큼 선형보간해줘서 최종 결과벡터를 벹뱉어준단말이지 아주 칱칭찬해
		vScale = XMVectorLerp(XMLoadFloat3(&vSourScale), XMLoadFloat3(&vDestScale), fRatio); 
		//! 잣댓다. 쿼터니언은 어쩌지? 이것도 따로 함수있다 개꿀?
		vRotation = XMQuaternionSlerp(XMLoadFloat4(&vSourRotation), XMLoadFloat4(&vDestRotation), fRatio);
		vPosition = XMVectorLerp(XMLoadFloat3(&vSourPosition), XMLoadFloat3(&vDestPosition), fRatio);
	}

	//! 위에서 저 상태들로 뭐하려고한거야? 시간에맞는 TransformationMatrix 만들어주려고 했던거잖아?
	//! 어떻게 만들거야? 로테이션은 쿼터니언인데? 이것도 함수있어 개꿀 ㅋㅋ
	//! 응? 인자값으로 로테이션 원점을 달라는데? 원점이뭐지? 0,0,0,1 이잖아 주면되지 ㅋㅋ
	_matrix	TransformationMatrix = XMMatrixAffineTransformation(vScale, XMVectorSet(0.f,0.f,0.f,1.f), vRotation, vPosition);

	//! 이제 이 채널과 같은 이름을 가진 뼈를 찾아서 그 뼈의 TransformationMatrix 갱신해줄거야.
	Bones[m_iBoneIndex]->Set_TransformationMatrix(TransformationMatrix);
}


CChannel* CChannel::Create(const string strName, vector<KEYFRAME>& Keyframes, _int iBoneIndex)
{
	CChannel* pInstance = new CChannel();

	if (FAILED(pInstance->Initialize(strName, Keyframes, iBoneIndex)))
	{
		MSG_BOX("Failed to Created : CChannel");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CChannel::Free()
{
}
