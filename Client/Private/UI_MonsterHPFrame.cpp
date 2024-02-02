#include "stdafx.h"
#include "UI_MonsterHPFrame.h"
#include "GameInstance.h"

CUI_MonsterHPFrame::CUI_MonsterHPFrame(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eCurrentLevel)
	:CMyUI(pDevice,pContext)
	, m_eCurrentLevel(eCurrentLevel)
{
	
}

CUI_MonsterHPFrame::CUI_MonsterHPFrame(const CUI_MonsterHPFrame& rhs)
	: CMyUI(rhs)
	, m_eCurrentLevel(rhs.m_eCurrentLevel)
{
}

HRESULT CUI_MonsterHPFrame::Initialize_Prototype()
{
	//TODO 원형객체의 초기화과정을 수행한다.
	/* 1.서버로부터 값을 받아와서 초기화한다 .*/
	/* 2.파일입출력을 통해 값을 받아온다.*/

	return S_OK;
}

HRESULT CUI_MonsterHPFrame::Initialize(void* pArg)
{
	m_tInfo = *(MONSTER_HP*)pArg;
	m_tInfo.bFrame = true;
	

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if(FAILED(__super::Initialize(pArg))) //!  트랜스폼 셋팅, m_tInfo의 bWorldUI 가 false 인 경우에만 직교위치 셋팅
		return E_FAIL;

	m_fCurrentHp = m_tInfo.pOwnerStatus->fCurrentHp;
	m_fPrevHp = m_fCurrentHp;
	m_tInfo.fCrntHPUV = m_tInfo.pOwnerStatus->fCurrentHp / m_tInfo.pOwnerStatus->fMaxHp;
	SetUp_UV(0);
	

	return S_OK;
}

void CUI_MonsterHPFrame::Priority_Tick(_float fTimeDelta)
{
}

void CUI_MonsterHPFrame::Tick(_float fTimeDelta)
{


	m_tInfo.fCrntHPUV = m_tInfo.pOwnerStatus->fCurrentHp / m_tInfo.pOwnerStatus->fMaxHp; 
	
	if (m_tInfo.fCrntHPUV <= 0.0)
	{
		m_tInfo.fCrntHPUV = 0.0;

		Set_Dead();
	}
	
	
}

void CUI_MonsterHPFrame::Late_Tick(_float fTimeDelta)
{
	Compute_OwnerCamDistance();

	if (FAILED(m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_UI, this)))
		return;
}

HRESULT CUI_MonsterHPFrame::Render()
{
	//TODO 셰이더에게 행렬을 던져주는 행위는 반드시 셰이더의 비긴함수를 호출하기 이전에 해야한다.
	//! 그 이유는, 셰이더의 비긴함수 내에서 pPass->Apply(0, m_prContext); 코드를 수행한다.
	//! Apply 호출 후에 행렬을 던져줘도 에러는 나지 않지만, 안정성이 떨어진다.
	//! Apply 호출 후에 행렬을 던져주면, 어떤 때에는 정상적으로 수행되고, 어떤 때에는 값이 제대로 안 넘어가는 경우가 있다.
	//! 
	if (m_tInfo.eMonsterType == CUI_MonsterHPFrame::BOSS)
	{

	}
	else if (m_tInfo.eMonsterType == CUI_MonsterHPFrame::COMMON)
	{

		if (m_fOwnerCamDistance > 40.f)
		{
			m_pGameInstance->Get_CamDir();
			return E_FAIL;
		}
		__super::SetUp_WorldToScreen(m_tInfo.pOwnerTransform->Get_State(CTransform::STATE_POSITION));

		__super::SetUp_Transform(m_fWorldToScreenX, m_fWorldToScreenY, m_fDefaultScale * m_fScaleOffsetX, m_fDefaultScale * m_fScaleOffsetY);


		//__super::SetUp_BillBoarding();
	}

	if(FAILED(Bind_ShaderResources()))
		return E_FAIL;

	//! 이 셰이더에 0번째 패스로 그릴거야.
	m_pShaderCom->Begin(8); //! Shader_PosTex 7번 패스 = VS_MAIN,  PS_UI_HP

	//! 내가 그리려고 하는 정점, 인덱스 버퍼를 장치에 바인딩해
	m_pVIBufferCom->Bind_VIBuffers();

	//! 바인딩된 정점, 인덱스를 그려
	m_pVIBufferCom->Render();
	
	return S_OK;
}

HRESULT CUI_MonsterHPFrame::Ready_Components()
{
	//! For.Com_Shader
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxPosTex"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;
	
	//! For.Com_VIBuffer
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_VIBuffer_Rect"),
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom))))
		return E_FAIL;

	if (m_tInfo.eMonsterType == CUI_MonsterHPFrame::BOSS)
	{
		//! For.Com_Texture
		if (FAILED(__super::Add_Component(m_eCurrentLevel, TEXT("Prototype_Component_Texture_EnemyBossHPFrame"),
			TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
			return E_FAIL;
	}
	else
	{
		//! For.Com_Texture
		if (FAILED(__super::Add_Component(m_eCurrentLevel, TEXT("Prototype_Component_Texture_EnemyHPFrame"),
			TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
			return E_FAIL;
	}

	

	return S_OK;
}

HRESULT CUI_MonsterHPFrame::Bind_ShaderResources()
{
	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_ViewMatrix)))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_ProjMatrix)))
		return E_FAIL;

	if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;
	
	return S_OK;
}

void CUI_MonsterHPFrame::Compute_OwnerCamDistance()
{
	_vector		vPosition = m_tInfo.pOwnerTransform->Get_State(CTransform::STATE_POSITION);
	_vector		vCamPosition = XMLoadFloat4(&m_pGameInstance->Get_CamPosition());

	m_fOwnerCamDistance = XMVectorGetX(XMVector3Length(vPosition - vCamPosition));
}

CUI_MonsterHPFrame* CUI_MonsterHPFrame::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eCurrentLevel)
{
	CUI_MonsterHPFrame* pInstance = new CUI_MonsterHPFrame(pDevice, pContext, eCurrentLevel);

	/* 원형객체를 초기화한다.  */
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CUI_MonsterHPFrame");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CUI_MonsterHPFrame::Clone(void* pArg)
{
	CUI_MonsterHPFrame* pInstance = new CUI_MonsterHPFrame(*this);

	/* 원형객체를 초기화한다.  */
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CUI_MonsterHPFrame");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CUI_MonsterHPFrame::Free()
{
	__super::Free();

	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pShaderCom);
	Safe_Release(m_pTextureCom);
	
}
