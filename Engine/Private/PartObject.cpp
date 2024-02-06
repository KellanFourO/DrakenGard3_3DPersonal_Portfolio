#include "PartObject.h"
#include "GameInstance.h"
#include "Transform.h"
#include "Shader.h"
#include "Model.h"
#include "Bone.h"
#include "Navigation.h"
#include "Texture.h"

CPartObject::CPartObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{
}

CPartObject::CPartObject(const CPartObject& rhs)
	: CGameObject(rhs)
	, m_eCurrentLevelIndex(rhs.m_eCurrentLevelIndex)
	, m_strName(rhs.m_strName)
{
	
}

void CPartObject::SetUp_Animation(_uint iAnimIndex)
{
	m_pModelCom->Set_Animation(iAnimIndex);
}

CBone* CPartObject::Get_BonePtr(const _char* pBoneName)
{
	return m_pModelCom->Get_BonePtr(pBoneName);
}

void CPartObject::Init_Status(_float fMaxHp, _float fDmg)
{
	m_tStatus.fMaxHp = fMaxHp;
	m_tStatus.fDmg = fDmg;
	m_tStatus.eAttackType = tagStatusDesc::ATTACKTYPE_END;
	m_tOriginStatus = m_tStatus;
}

HRESULT CPartObject::Ready_Components(_uint iLevelIndex, const wstring& strShaderTag, const wstring& strModelTag)
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(iLevelIndex, strShaderTag, TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(iLevelIndex, strModelTag,  TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;


	return S_OK;
}

HRESULT CPartObject::Bind_ShaderResources()
{
	if (FAILED(m_pShaderCom->Bind_Matrix("g_WorldMatrix", &m_WorldMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	return S_OK;
}

void CPartObject::Dead_Action(_float fTimeDelta, _float fLifeTime)
{
	if (true == m_bDissove)
	{
		m_fDissoveWeight += fTimeDelta * 0.5f;

		if (m_fDissoveWeight > fLifeTime)
		{
			Set_Dead();
		}
		m_iPassIndex = 8;

		m_pShaderCom->Bind_RawValue("g_fDissolveWeight", &m_fDissoveWeight, sizeof(_float));
		m_pDissoveTexture->Bind_ShaderResource(m_pShaderCom, "g_DissolveTexture");
	}
}

void CPartObject::Free()
{
	if(nullptr != m_pParentNavigationCom)
		Safe_Release(m_pParentNavigationCom);

	Safe_Release(m_pParentTransformCom);
	Safe_Release(m_pSocketBone);
	Safe_Release(m_pShaderCom);

	if(nullptr != m_pModelCom)
		Safe_Release(m_pModelCom);

	__super::Free();
}
