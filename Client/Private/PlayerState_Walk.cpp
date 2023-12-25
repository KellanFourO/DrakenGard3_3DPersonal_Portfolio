#include "stdafx.h"
#include "Player.h"
#include "PlayerState_Walk.h"
#include "PlayerPart_Body.h"

#include "GameInstance.h"
#include "StateMachine.h"
#include "Model.h"
#include "Transform.h"
#include "Navigation.h"
#include "RigidBody.h"

CPlayerState_Walk::CPlayerState_Walk()
{
}

HRESULT CPlayerState_Walk::Initialize(CPlayer* pPlayer)
{
	if (FAILED(__super::Initialize(pPlayer)))
		return E_FAIL;

	m_fEndTime = 0.25f; //! 일정시간 키 입력 없을시 Idle 상태로 돌아가기.


	return S_OK;
}

HRESULT CPlayerState_Walk::StartState()
{
	m_pOwnerModelCom->Set_Animation(67);
	m_pOwnerTransform->Set_SpeedPerSec(1.f);
	m_pOwnerTransform->Set_RotationPerSec(XMConvertToRadians(60.0f));

	return S_OK;
}

HRESULT CPlayerState_Walk::EndState()
{
	m_fLastInputTime = 0.f;
	m_fAccTime = 0.f;

	return S_OK;
}

void CPlayerState_Walk::Tick(const _float& fTimeDelta)
{
	KeyInput(fTimeDelta);
}

void CPlayerState_Walk::Late_Tick(const _float& fTimeDelta)
{
	if (m_pGameInstance->Key_Pressing(DIK_W) || m_pGameInstance->Key_Pressing(DIK_A)
		|| m_pGameInstance->Key_Pressing(DIK_S) || m_pGameInstance->Key_Pressing(DIK_D))
		m_bKeyPressing = true;
	else
		m_bKeyPressing = false;

	m_fAccTime += m_fLastInputTime + fTimeDelta;

	if (m_fAccTime > m_fEndTime && !m_bKeyPressing)
		m_pOwnerStateCom->Transition(CStateMachine::STATETYPE::STATE_GROUND, TEXT("PlayerState_Idle"));
}

void CPlayerState_Walk::KeyInput(const _float& fTimeDelta)
{
	if (m_pGameInstance->Key_Down(DIK_LSHIFT))
	{
		m_pOwnerStateCom->Transition(CStateMachine::STATETYPE::STATE_GROUND, TEXT("PlayerState_Run"));
	}

	if (m_pGameInstance->Key_Pressing(DIK_W))
	{
		m_pOwnerTransform->Go_Straight(fTimeDelta, m_pOwnerNavagation);
	}

	if (m_pGameInstance->Key_Pressing(DIK_A))
	{
		m_pOwnerTransform->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta * -1.f);
	}

	if (m_pGameInstance->Key_Pressing(DIK_S))
	{
		m_pOwnerTransform->Go_Backward(fTimeDelta);
	}

	if (m_pGameInstance->Key_Pressing(DIK_D))
	{
		m_pOwnerTransform->Turn(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta * 1.f);
	}

	if (m_pGameInstance->Key_Down(DIK_SPACE))
	{
		m_pOwnerStateCom->Transition(CStateMachine::STATETYPE::STATE_AIR, TEXT("PlayerState_Jump"));
	}
}

CPlayerState_Walk* CPlayerState_Walk::Create(CPlayer* pPlayer)
{
	CPlayerState_Walk* pInstance = new CPlayerState_Walk();

	/* 원형객체를 초기화한다.  */
	if (FAILED(pInstance->Initialize(pPlayer)))
	{
		MSG_BOX("Failed to Created : CPlayerState_Walk");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CPlayerState_Walk::Free()
{
	__super::Free();
}
