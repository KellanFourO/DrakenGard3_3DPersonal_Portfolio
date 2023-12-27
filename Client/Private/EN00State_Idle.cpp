#include "EN00State_Idle.h"
#include "GameObject.h"
#include "Model.h"
#include "StateMachine.h"

CEN00State_Idle::CEN00State_Idle()
{
}

HRESULT CEN00State_Idle::Initialize(CGameObject* pOwner)
{
	if(FAILED(__super::Initialize(pOwner)))
		return E_FAIL;
	
	m_fSearchRange = 10.f;

	m_pOwner = pOwner;

	if(FAILED(AddRefIfNotNull(m_pOwner)))
		return E_FAIL;
	
	return S_OK;
}

HRESULT CEN00State_Idle::StartState()
{
	m_pOwnerModelCom->Set_Animation(0);
	
	return S_OK;
}

HRESULT CEN00State_Idle::EndState()
{
	return S_OK;
}

void CEN00State_Idle::Priority_Tick(const _float& fTimeDelta)
{
}

void CEN00State_Idle::Tick(const _float& fTimeDelta)
{
	if (true == Search())
	{
		m_pOwnerStateCom->Transition(CStateMachine::STATE_GROUND, TEXT("EN00State_Chase"));
	}

}

void CEN00State_Idle::Late_Tick(const _float& fTimeDelta)
{
}

CEN00State_Idle* CEN00State_Idle::Create(CGameObject* pOwner)
{
	CEN00State_Idle* pInstance = new CEN00State_Idle();

	/* ������ü�� �ʱ�ȭ�Ѵ�.  */
	if (FAILED(pInstance->Initialize(pOwner)))
	{
		MSG_BOX("Failed to Created : CEN00State_Idle");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CEN00State_Idle::Free()
{
	__super::Free();

	Safe_Release(m_pOwner);
}