#include "stdafx.h"
#include "Player.h"
#include "PlayerState_Attack1.h"
#include "PlayerPart_Body.h"

#include "GameInstance.h"
#include "StateMachine.h"
#include "Model.h"
#include "Transform.h"
#include "Navigation.h"
#include "RigidBody.h"

CPlayerState_Attack1::CPlayerState_Attack1()
{
}

HRESULT CPlayerState_Attack1::Initialize(CPlayer* pPlayer)
{
	if(FAILED(__super::Initialize(pPlayer)))
		return E_FAIL;


	return S_OK;
}

HRESULT CPlayerState_Attack1::StartState()
{
	m_pOwnerModelCom->Set_Animation(90);
	m_fDuration = m_pOwnerModelCom->Get_CurrentDuration();

	return S_OK;
}

HRESULT CPlayerState_Attack1::EndState()
{
	
	m_fAccTime = 0.f;
	m_fLastInputTime = 0.f;
	m_bChange = false;
	m_fDuration = 0.f;
	return S_OK;
}

void CPlayerState_Attack1::Tick(const _float& fTimeDelta)
{
	
}

void CPlayerState_Attack1::Late_Tick(const _float& fTimeDelta)
{
	m_fAccTime +=  30 * fTimeDelta;
	
	if (m_fDuration < m_fAccTime && !m_bChange)
	{
		m_pOwnerModelCom->Set_Animation(91);
		m_bChange = true;
		m_fAccTime = 0;
	}

	if (m_bChange)
	{
		m_fDuration = m_pOwnerModelCom->Get_CurrentDuration();
	
		if (m_fDuration < m_fAccTime)
		{
			KeyInput(fTimeDelta);

			//m_pOwnerStateCom->Transition(CStateMachine::STATETYPE::STATE_GROUND, TEXT("PlayerState_Idle"));
		}
	}
}

void CPlayerState_Attack1::KeyInput(const _float& fTimeDelta)
{
	if (m_pGameInstance->Mouse_Down(DIM_LB))
	{
		m_pOwnerStateCom->Transition(CStateMachine::STATETYPE::STATE_GROUND, TEXT("PlayerState_Attack2"));
	}
}

CPlayerState_Attack1* CPlayerState_Attack1::Create(CPlayer* pPlayer)
{
	CPlayerState_Attack1* pInstance = new CPlayerState_Attack1();

	/* ������ü�� �ʱ�ȭ�Ѵ�.  */
	if (FAILED(pInstance->Initialize(pPlayer)))
	{
		MSG_BOX("Failed to Created : CPlayerState_Attack1");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CPlayerState_Attack1::Free()
{
	__super::Free();
}