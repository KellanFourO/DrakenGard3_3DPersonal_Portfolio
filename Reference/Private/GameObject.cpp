#include "GameObject.h"
#include "GameInstance.h"
#include "Transform.h"

CGameObject::CGameObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice(pDevice)
	, m_pContext(pContext)
	, m_pGameInstance(CGameInstance::GetInstance())
	, m_isCloned(false)
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

CGameObject::CGameObject(const CGameObject& rhs)
	: m_pDevice(rhs.m_pDevice)
	, m_pContext(rhs.m_pContext)
	, m_pGameInstance(rhs.m_pGameInstance)
	, m_isCloned(true)
{
	Safe_AddRef(m_pGameInstance);
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CGameObject::Initialize_Prototype()
{
	return S_OK;
}

HRESULT CGameObject::Initialize(void* pArg)
{
	GAMEOBJECT_DESC Desc = {};

	if(nullptr != pArg)
		Desc = *(GAMEOBJECT_DESC*)pArg;

	m_pTransformCom = CTransform::Create(m_pDevice, m_pContext, Desc.fSpeedPerSec, Desc.fRotationPerSec);
	if(nullptr == m_pTransformCom)
		return E_FAIL;

	if(nullptr != Find_Component(g_pTransformTag))
		return E_FAIL;

	m_Components.emplace(g_pTransformTag, m_pTransformCom);

	Safe_AddRef(m_pTransformCom);

	if (Desc.isPicking)
	{
	//_float4 vPos = { Desc.vPos.x, Desc.vPos.y, Desc.vPos.z, 1.f };
	//
	//m_pTransformCom->Set_State(CTransform::STATE_POSITION, vPos);

	//m_pTransformCom->Set_State(CTransform::STATE_POSITION, XMVectorSet(Desc.vPos.x, Desc.vPos.y, Desc.vPos.z, 1.f));

	m_pTransformCom->Set_State(CTransform::STATE_POSITION, Desc.vPos);
	}

	return S_OK;
}

void CGameObject::Priority_Tick(_float fTimeDelta)
{
// 	for (size_t i = 0; m_UpdateComponents.size(); i++)
// 	{
// 		m_UpdateComponents[i]->Priority_Tick(fTimeDelta);
// 	}
}

void CGameObject::Tick(_float fTimeDelta)
{
// 	for (size_t i = 0; m_UpdateComponents.size(); i++)
// 	{
// 		m_UpdateComponents[i]->Tick(fTimeDelta);
// 	}
}

void CGameObject::Late_Tick(_float fTimeDelta)
{
// 	for (size_t i = 0; m_UpdateComponents.size(); i++)
// 	{
// 		m_UpdateComponents[i]->Late_Tick(fTimeDelta);
// 	}
}

HRESULT CGameObject::Render()
{
	return S_OK;
}

_bool CGameObject::Picking(_float3 vPickPos)
{
	_float3 vScale = m_pTransformCom->Get_Scaled();
	_float3	vPos = m_pTransformCom->Get_Pos();

	if (vPickPos.x >= vPos.x - vScale.x && vPickPos.x <= vPos.x + vScale.x &&
		vPickPos.y >= vPos.y - vScale.y && vPickPos.y <= vPos.y + vScale.y &&
		vPickPos.z >= vPos.z - vScale.z && vPickPos.z <= vPos.z + vScale.z)
	{
		return true; //ImGui ���� ��
	}
	
	return false;
}

void CGameObject::Write_Json(json& Out_Json)
{
	for (auto& pComponent : m_Components)
	{
		pComponent.second->Write_Json(Out_Json["Component"]);
	}
}

void CGameObject::Load_FromJson(const json& In_Json)
{
	for (auto& pComponent : m_Components)
	{
		pComponent.second->Load_FromJson(In_Json["Component"]);
	}
}



HRESULT CGameObject::Add_Component(_uint iLevelIndex, const wstring& strPrototypeTag, const wstring& strComTag, _Inout_ CComponent** ppOut, void* pArg)
{
	//! �̹� ���� �纻 ������Ʈ�� ������� ����ó��
	if(nullptr != Find_Component(strComTag))
		return E_FAIL;

	//! ������ü�� ������ �纻��ü�� ������.
	CComponent*	pComponent = m_pGameInstance->Clone_Component(iLevelIndex, strPrototypeTag, pArg);
	if(nullptr == pComponent)
		return E_FAIL;

	//! ���ڰ����� �Ѿ�� �İ�ü�� ��� �����ؼ� ���� �纻��ü�� �ּҷ� �־��ش�.
	//! ���������ʹ� �������� �������� �ʴ´�. �׷��� �ۿ��� ���ڰ����� �־��ٶ� CComponent�� ĳ�����ؼ� ��������Ѵ�.
	*ppOut = pComponent;
	
	//! �ۿ��� ���ڰ����� �־��� ������Ʈ �±׸� Ű������ �����̳ʿ� �־��ش�
	m_Components.emplace(strComTag, pComponent);

	//TODO ������ �纻��ü�� ������ ���� ������ �ƴ϶� ���ο� ���� ����� ���̱⿡ ���۷���ī��Ʈ�� �ø����ʾҴ�.
	//! ������ ���� �� �����̳ʿ� �־��ִ� ���� �� ��ü�� �ּҸ� �����ϴ� �����̱⿡ ���۷��� ī��Ʈ�� �÷���� �Ѵ�.
	Safe_AddRef(pComponent);
	
	return S_OK;
}

void CGameObject::Delete_Component(const wstring& strComTag)
{
	auto	iter = m_Components.find(strComTag);

	if (iter == m_Components.end())
		return;

	Safe_Release(iter->second); // ��Ÿ������
	m_Components.erase(iter);
}

CComponent* CGameObject::Find_Component(const wstring& strComTag)
{
	auto	iter = m_Components.find(strComTag);

	if(iter == m_Components.end())
		return nullptr;
	
	return iter->second;
}

void CGameObject::Free()
{
	__super::Free();

	Safe_Release(m_pTransformCom);

	for(auto& Pair : m_Components)
		Safe_Release(Pair.second);

	m_Components.clear();

	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}