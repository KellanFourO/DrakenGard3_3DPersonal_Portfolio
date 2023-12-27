#include "stdafx.h"
#include "Player.h"
#include "PlayerState_Attack6.h"
#include "PlayerPart_Body.h"

#include "GameInstance.h"
#include "StateMachine.h"
#include "Model.h"
#include "Transform.h"
#include "Navigation.h"
#include "RigidBody.h"
#include "Animation.h"

CPlayerState_Attack6::CPlayerState_Attack6()
{
}

HRESULT CPlayerState_Attack6::Initialize(CPlayer* pPlayer)
{
	if(FAILED(__super::Initialize(pPlayer)))
		return E_FAIL;


	return S_OK;
}

HRESULT CPlayerState_Attack6::StartState()
{
	m_pOwnerModelCom->Set_Animation(100);
	m_pOwnerModelCom->Set_Loop(false);
	return S_OK;
}

HRESULT CPlayerState_Attack6::EndState()
{

	m_fAccTime = 0.f;
	m_fLastInputTime = 0.f;
	m_isEnd = false;
	m_bInput = false;

	return S_OK;
}

void CPlayerState_Attack6::Tick(const _float& fTimeDelta)
{
}

void CPlayerState_Attack6::Late_Tick(const _float& fTimeDelta)
{
	NextComboOrIdle(m_pOwnerModelCom, m_pOwnerStateCom, TEXT("PlayerState_Attack7"), 101);
}

CPlayerState_Attack6* CPlayerState_Attack6::Create(CPlayer* pPlayer)
{
	CPlayerState_Attack6* pInstance = new CPlayerState_Attack6();

	/* ������ü�� �ʱ�ȭ�Ѵ�.  */
	if (FAILED(pInstance->Initialize(pPlayer)))
	{
		MSG_BOX("Failed to Created : CPlayerState_Attack6");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CPlayerState_Attack6::Free()
{
	__super::Free();
}