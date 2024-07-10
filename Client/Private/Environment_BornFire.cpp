#include "stdafx.h"
#include "Environment_BornFire.h"
#include "Effect_BornFire.h"
#include "GameInstance.h"


CEnvironment_BornFire::CEnvironment_BornFire(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CNonAnimObject(pDevice, pContext)
{
}

CEnvironment_BornFire::CEnvironment_BornFire(const CEnvironment_BornFire& rhs)
	: CNonAnimObject(rhs)
	, m_pNavigationCom(rhs.m_pNavigationCom)
{
	Safe_AddRef(m_pNavigationCom);
}

HRESULT CEnvironment_BornFire::Initialize_Prototype(LEVEL eLevel)
{
	m_eCurrentLevelID = eLevel;

	
	//! For.Com_Navigation
	if (FAILED(__super::Add_Component(m_eCurrentLevelID, TEXT("Prototype_Component_Navigation"),
		TEXT("Com_Navigation"), reinterpret_cast<CComponent**>(&m_pNavigationCom))))
		return E_FAIL;

	return S_OK;
}

HRESULT CEnvironment_BornFire::Initialize(void* pArg)
{
	CEnvironment_BornFire::ENVIRONMENT_DESC Desc = {};

	if (nullptr != pArg)
		Desc = *(ENVIRONMENT_DESC*)pArg;

	m_strModelTag = Desc.strModelTag;

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;
	m_pTransformCom->Set_WorldFloat4x4(Desc.WorldMatrix);
	//m_pTransformCom->Set_WorldFloat4x4(m_WorldMatrix);

	if (FAILED(Ready_Components()))
		return E_FAIL;


	

	LIGHT_DESC			LightDesc{};
	
	LightDesc.eType = LIGHT_DESC::TYPE_POINT;
	LightDesc.vPosition = Desc.vPos;
	LightDesc.fRange = 8.f;
	LightDesc.vDiffuse = _float4(1.f, 0.f, 0.f, 1.f);
	LightDesc.vAmbient = _float4(0.4f, 0.1f, 0.1f, 1.f);
	LightDesc.vSpecular = LightDesc.vDiffuse;
	 
	if (FAILED(m_pGameInstance->Add_Light(LightDesc)))
	return E_FAIL;
	
	m_pTransformCom->Set_WorldFloat4x4(Desc.WorldMatrix);
	
	CEffect_BornFire::BORNFIRE_DESC BornFireDesc;
	
	_vector vCreatePos = XMLoadFloat4(&Desc.vPos);
	vCreatePos.m128_f32[3] = 1.f;
	BornFireDesc.bBossBreath = false;
	BornFireDesc.vScale = { 3.f, 3.f, 5.f};
	
	vCreatePos.m128_f32[1] += 1.5f;
	XMStoreFloat4(&BornFireDesc.vPos, vCreatePos);
	
	XMStoreFloat4(&BornFireDesc.vLook, m_pTransformCom->Get_State(CTransform::STATE_LOOK));
	
	m_pGameInstance->Add_CloneObject(m_eCurrentLevelID, TEXT("Layer_Effect"), TEXT("Prototype_GameObject_Effect_BornFire"), &BornFireDesc);

		
	//!m_pTransformCom->Set_Scaling(Desc.vScale.x, Desc.vScale.y, Desc.vScale.z);
	//!m_pTransformCom->Set_State(CTransform::STATE_POSITION, Desc.vPos);
	//!m_pTarget = Desc.pTarget;
	//!m_vCreatePos = Desc.vPos;
	//!m_vCreateLook = Desc.vLook;
	//!m_fLifeTime = Desc.fLifeTime;
	//!m_bBreath = Desc.bBossBreath;

	return S_OK;
}

void CEnvironment_BornFire::Priority_Tick(_float fTimeDelta)
{
	
	m_pNavigationCom->Update(XMMatrixIdentity());
}

void CEnvironment_BornFire::Tick(_float fTimeDelta)
{
}

void CEnvironment_BornFire::Late_Tick(_float fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this)))
		return;
}

HRESULT CEnvironment_BornFire::Render()
{
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	_uint		iNumMeshes = m_pModelCom->Get_NumMeshes();

	for (size_t i = 0; i < iNumMeshes; i++)
	{
		m_pModelCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture", i, aiTextureType_DIFFUSE);

		m_pShaderCom->Begin(0);


		m_pModelCom->Render(i);
	}

#ifdef _DEBUG
	m_pNavigationCom->Render();
#endif 


	return S_OK;
}

HRESULT CEnvironment_BornFire::Ready_Components()
{
	/* For.Com_Shader */
	if (FAILED(__super::Add_Component(m_eCurrentLevelID, TEXT("Prototype_Component_Shader_Model"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	/* For.Com_Model */
	if (FAILED(__super::Add_Component(m_eCurrentLevelID, m_strModelTag,
		TEXT("Com_Model"), reinterpret_cast<CComponent**>(&m_pModelCom))))
		return E_FAIL;

	

	return S_OK;
}

HRESULT CEnvironment_BornFire::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;
	

	return S_OK;
}


CEnvironment_BornFire* CEnvironment_BornFire::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eLevel)
{
	CEnvironment_BornFire* pInstance = new CEnvironment_BornFire(pDevice, pContext);

	/* ������ü�� �ʱ�ȭ�Ѵ�.  */
	if (FAILED(pInstance->Initialize_Prototype(eLevel)))
	{
		MSG_BOX("Failed to Created : CEnvironment_BornFire");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CEnvironment_BornFire::Clone(void* pArg)
{
	CEnvironment_BornFire* pInstance = new CEnvironment_BornFire(*this);

	/* ������ü�� �ʱ�ȭ�Ѵ�.  */
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CEnvironment_BornFire");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CEnvironment_BornFire::Free()
{
	__super::Free();

	Safe_Release(m_pNavigationCom);
	
}