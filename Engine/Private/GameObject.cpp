#include "GameObject.h"
#include "GameInstance.h"
#include "Transform.h"
#include "Model.h"

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

_bool CGameObject::Picking(_float3 vPickPos, class CModel* pModelCom)
{
	if (pModelCom == nullptr)
		return false;

	_uint iWinSizeX = m_pGameInstance->Get_GraphicDesc().iBackBufferSizeX;
	_uint iWinSizeY = m_pGameInstance->Get_GraphicDesc().iBackBufferSizeY;

	// 마우스의 월드 스페이스를 받는다.
	RAY MouseRayInWorldSpace = m_pGameInstance->Get_Ray(iWinSizeX, iWinSizeY);

	// 받은 마우스 월드 스페이스와 이 녀석의 월드 스페이스를 넘겨주고, 픽킹이 됐다면 픽킹된 값을 받아온다.
	if (pModelCom->Compute_MousePos(MouseRayInWorldSpace, m_pTransformCom->Get_WorldMatrix())) // 값 잘 받았음))
		return true;

	return false;

	//_float3 vScale = m_pTransformCom->Get_Scaled();
	//_float3	vPos = m_pTransformCom->Get_Pos();
	//
	//if (vPickPos.x >= vPos.x - vScale.x && vPickPos.x <= vPos.x + vScale.x &&
	//	vPickPos.y >= vPos.y - vScale.y && vPickPos.y <= vPos.y + vScale.y &&
	//	vPickPos.z >= vPos.z - vScale.z && vPickPos.z <= vPos.z + vScale.z)
	//{
	//	return true; //ImGui 영역 내
	//}
	//
	//return false;
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
	//! 이미 같은 사본 컴포넌트가 있을경우 예외처리
	if(nullptr != Find_Component(strComTag))
		return E_FAIL;

	//! 원형객체를 복사한 사본객체를 만들어낸다.
	CComponent*	pComponent = m_pGameInstance->Clone_Component(iLevelIndex, strPrototypeTag, pArg);
	if(nullptr == pComponent)
		return E_FAIL;

	//! 인자값으로 넘어온 컴객체를 방금 복사해서 만들어낸 사본객체의 주소로 넣어준다.
	//! 이중포인터는 다형성을 지원하지 않는다. 그래서 밖에서 인자값으로 넣어줄때 CComponent로 캐스팅해서 보내줘야한다.
	*ppOut = pComponent;
	
	//! 밖에서 인자값으로 넣어준 컴포넌트 태그를 키값으로 컨테이너에 넣어준다
	m_Components.emplace(strComTag, pComponent);

	//TODO 복사한 사본객체를 만들어내는 것은 참조가 아니라 새로운 것을 만드는 것이기에 레퍼런스카운트를 올리지않았다.
	//! 하지만 지금 맵 컨테이너에 넣어주는 것은 한 객체의 주소를 공유하는 행위이기에 레퍼런스 카운트를 올려줘야 한다.
	Safe_AddRef(pComponent);
	
	return S_OK;
}

void CGameObject::Delete_Component(const wstring& strComTag)
{
	auto	iter = m_Components.find(strComTag);

	if (iter == m_Components.end())
		return;

	Safe_Release(iter->second); // 잘타고있음
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
