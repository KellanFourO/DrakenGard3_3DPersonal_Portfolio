#include "Channel.h"

CChannel::CChannel()
{
}

HRESULT CChannel::Initialize(const aiNodeAnim* pChannel)
{
	strcpy_s(m_szName, pChannel->mNodeName.data);

	//!#키프레임개수_주의점
	//!  스케일, 로테이션,포지션의 키프레임의 개수는 각자 다를 수 있어.
	//!  왜냐하면,  애니메이션이 구동될때 크기의 변화는 없이 회전만 할 수도 있고 마찬가지로 위치는 달라지는데 회전은 하지 않을수도 있는 거지.
	//! 즉, 이전 위치 그대로일 수도 있다는거야. 그래서 키 프레임의 개수는 다 다르니 
	//! 총 키프레임의 개수는 스케일,로테이션,포지션 각 가지고있는 키프레임의 개수들중 가장 큰값을 가진 놈으로 찾아서 보관할거야.
	m_iNumKeyFrames = max(pChannel->mNumScalingKeys, pChannel->mNumRotationKeys);
	m_iNumKeyFrames = max(pChannel->mNumPositionKeys, m_iNumKeyFrames);

	//TODO 위 주의점에서 말했듯 이전 위치를 기억해야겠지?
	//!  밑에서 루프를 돌릴건데, 예를들어 공격 애니메이션의 3번 프레임의 스케일은 존재하지않을 수 있어.
	//! 그래서 각각 조건문을 걸어서 걸러줄건데, 그 조건문을 통과하지못한다면 그냥 0으로 채워질거야. 
	//! 루프문 돌기전에 변수를 선언해놓고 값을 채워줘서 이전 값을 기억하게 하자.
	_float3 vScale;
	_float4	vRotation;
	_float3	vPosition;

	for (size_t i = 0; i < m_iNumKeyFrames; i++)
	{
		KEYFRAME		KeyFrame = {};

		if (i < pChannel->mNumScalingKeys)
		{
			memcpy(&vScale, &pChannel->mScalingKeys[i].mValue, sizeof(_float3));
			KeyFrame.fTrackPosition = pChannel->mScalingKeys[i].mTime;
		}

		if (i < pChannel->mNumRotationKeys)
		{
			//! 어심프의 로테이션키의 밸류는 aiQuaternion이야. x,y,z,w 순이아닌 w,x,y,z 순으로 되어있어
			//! 우리가 쓰던 거랑 순서가 다르지? 그래서 memcpy를 사용하면 안돼.
		
			vRotation.x = pChannel->mRotationKeys[i].mValue.x;
			vRotation.y = pChannel->mRotationKeys[i].mValue.y;
			vRotation.z = pChannel->mRotationKeys[i].mValue.z;
			vRotation.w = pChannel->mRotationKeys[i].mValue.w;
			KeyFrame.fTrackPosition = pChannel->mRotationKeys[i].mTime;
		}

		if (i < pChannel->mNumPositionKeys)
		{
			memcpy(&vPosition, &pChannel->mPositionKeys[i].mValue, sizeof(_float3));
			KeyFrame.fTrackPosition = pChannel->mPositionKeys[i].mTime;
		}

		KeyFrame.vScale = vScale;
		KeyFrame.vRotation = vRotation;
		KeyFrame.vPosition = vPosition;

		m_KeyFrames.push_back(KeyFrame);
	}

	return S_OK;
}

void CChannel::Invalidate_TransformationMatrix(_float fCurrentTrackPosition)
{
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
		if(fCurrentTrackPosition >= m_KeyFrames[m_iCurrentKeyFrameIndex + 1].fTrackPosition)
			++m_iCurrentKeyFrameIndex;

		//! 소스와 데스트야. 소스는 현재 프레임인 0번 프레임이 가진 상태를 의미하고, 데스트는 다음 프레임인 1번 프레임의 상태를 의미해.
		_float3		vSourScale, vDestScale;
		_float4		vSourRotation, vDestRotation;
		_float3		vSourPosition, vDestPosition;

		vSourScale		= m_KeyFrames[m_iCurrentKeyFrameIndex].vScale;
		vSourRotation	= m_KeyFrames[m_iCurrentKeyFrameIndex].vRotation;
		vSourPosition	= m_KeyFrames[m_iCurrentKeyFrameIndex].vPosition;
						
		vDestScale		= m_KeyFrames[m_iCurrentKeyFrameIndex + 1].vScale;
		vDestRotation	= m_KeyFrames[m_iCurrentKeyFrameIndex + 1].vRotation;
		vDestPosition	= m_KeyFrames[m_iCurrentKeyFrameIndex + 1].vPosition;


		//! XMVectorLerp라는 함수는 아주 기특하게도 비율만 넣어주면 선형보간을 예쁘게 해준단말이지. 비율 구하자
		//!  현재애니메이션위치 에서 
		_float	fRatio = 
		(fCurrentTrackPosition - m_KeyFrames[m_iCurrentKeyFrameIndex].fTrackPosition) /
		(m_KeyFrames[m_iCurrentKeyFrameIndex + 1].fTrackPosition - m_KeyFrames[m_iCurrentKeyFrameIndex].fTrackPosition);

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
}

CChannel* CChannel::Create(const aiNodeAnim* pChannel)
{
	CChannel* pInstance = new CChannel();

	if (FAILED(pInstance->Initialize(pChannel)))
	{
		MSG_BOX("Failed to Created : CChannel");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CChannel::Free()
{
}
