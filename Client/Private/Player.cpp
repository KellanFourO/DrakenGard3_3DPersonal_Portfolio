#include "stdafx.h"
#include "..\Public\Player.h"
#include "GameInstance.h"
#include "PartObject.h"
#include <iostream>
//TODO 컴포넌트

//TODO 파츠
#include "PlayerPart_Body.h"
#include "PlayerPart_Weapon.h"

//TODO 상태
#include "PlayerGroundStates.h"

#include "Camera_Target.h"
#include "Collider.h"
#include "RigidBody.h"

CPlayer::CPlayer(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CAnimObject(pDevice, pContext)
{
}

CPlayer::CPlayer(const CPlayer & rhs)
	: CAnimObject(rhs)
{
}

HRESULT CPlayer::Initialize_Prototype()
{	
	m_isPlayer = true;
	

	return S_OK;
}
 
HRESULT CPlayer::Initialize(void* pArg)
{	

	CGameObject::GAMEOBJECT_DESC PlayerDesc = {};
	
	if (nullptr != pArg)
		PlayerDesc = *(GAMEOBJECT_DESC*)pArg;

	PlayerDesc.fSpeedPerSec = 10.0f;
	PlayerDesc.fRotationPerSec = XMConvertToRadians(180.0f);
	m_eCurrentLevelID = (LEVEL)PlayerDesc.iLevelIndex;

	

	if (FAILED(__super::Initialize(&PlayerDesc)))
		return E_FAIL;	

	if (FAILED(Ready_Components()))
		return E_FAIL;

	if (FAILED(Ready_PartObjects()))
		return E_FAIL;

	Find_Collider();
	
	if (FAILED(Ready_Camera()))
			return E_FAIL;

	if (FAILED(Ready_States()))
		return E_FAIL;

	
	

	return S_OK;
}

void CPlayer::Priority_Tick(_float fTimeDelta)
{
	m_pStateCom->Priority_Tick(fTimeDelta);

	for (auto& Pair : m_PartObjects)
	{
		if (nullptr != Pair.second)
			Pair.second->Priority_Tick(fTimeDelta);
	}
}

void CPlayer::Tick(_float fTimeDelta)
{
	m_pStateCom->Tick(fTimeDelta);
	
	for (auto& Pair : m_PartObjects)
	{
		if (nullptr != Pair.second)
			Pair.second->Tick(fTimeDelta);
	}

	m_pColliderCom->Update(m_pTransformCom->Get_WorldMatrix());
	m_pRigidBodyCom->Tick(fTimeDelta);
	
	

	Key_Input(fTimeDelta);

}

void CPlayer::Late_Tick(_float fTimeDelta)
{
	m_pStateCom->Late_Tick(fTimeDelta);
	
	for (auto& Pair : m_PartObjects)
	{
		if (nullptr != Pair.second)
			Pair.second->Late_Tick(fTimeDelta);
	}


	m_fAccTime += fTimeDelta;

	//if (m_fAccTime > 0.2f)
	//{
	//	//printf(m_pStateCom->Get_aNI);
	//	printf(ConvertWstrToStrTest(m_pStateCom->Get_StateTag()).c_str());
	//	printf("\n\n");
	//	m_fAccTime = 0.f;
	//}

	if (FAILED(m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this)))
		return ;
}

HRESULT CPlayer::Render()
{
	#ifdef _DEBUG
		m_pNavigationCom->Render();
		m_pColliderCom->Render();
	#endif

	return S_OK;
}

void CPlayer::On_Collision(CGameObject* pCollisionObject, wstring& LeftTag, wstring& RightTag, _float3& vCollisionPos, _bool bType)
{
	//! 내 바디와 상대 바디 끼리 충돌했을 경우
	

	

	//m_pRigidBodyCom->Add_Force(vCollisionPos, CRigidBody::FORCE_MODE::FORCE);

	//wcout << LeftTag.c_str() << TEXT(" On_Collision is ") << RightTag.c_str() << endl;
}

void CPlayer::On_CollisionEnter(CGameObject* pCollisionObject, wstring& LeftTag, wstring& RightTag, _bool bType)
{
	if (bType == false)
	{
		CPlayerPart_Body* pBody = dynamic_cast<CPlayerPart_Body*>(Find_PartObject(TEXT("Part_Body")));
		pBody->Set_Move(false);
	}
		


	wcout << LeftTag.c_str() << TEXT(" On_CollisionEnter is ") << RightTag.c_str() << endl;

}

void CPlayer::On_CollisionExit(CGameObject* pCollisionObject, wstring& LeftTag, wstring& RightTag, _bool bType)
{
	if (bType == false)
	{
		CPlayerPart_Body* pBody = dynamic_cast<CPlayerPart_Body*>(Find_PartObject(TEXT("Part_Body")));
		pBody->Set_Move(true);
	}
	
	wcout << LeftTag.c_str() << TEXT(" On_CollisionExit is ") << RightTag.c_str() << endl;


	
}


void CPlayer::Set_Cam(CCamera_Target* pCam)
{
	m_pCamera = pCam;
	Safe_AddRef(m_pCamera);
}

CPartObject* CPlayer::Find_PartObject(const wstring& strPartTag)
{
	auto iter = m_PartObjects.find(strPartTag);

	if(iter == m_PartObjects.end())
		return nullptr;
	
	return iter->second;
}

HRESULT CPlayer::Ready_Components()
{
	/* For.Com_Navigation */
	CNavigation::NAVI_DESC		NaviDesc = {};
	NaviDesc.iCurrentIndex = 0;

	if (FAILED(__super::Add_Component(m_eCurrentLevelID, TEXT("Prototype_Component_Navigation"),
		TEXT("Com_Navigation"), reinterpret_cast<CComponent**>(&m_pNavigationCom), &NaviDesc)))
		return E_FAIL;

	/* For.Com_Collider */
	CBoundingBox_OBB::BOUNDING_OBB_DESC		BoundingDesc = {};

	BoundingDesc.vExtents = _float3(0.5f, 0.7f, 0.5f);
	BoundingDesc.vCenter = _float3(0.f, BoundingDesc.vExtents.y, 0.f);
	BoundingDesc.vRotation = _float3(0.f, XMConvertToRadians(45.0f), 0.f);

	if (FAILED(__super::Add_Component(m_eCurrentLevelID, TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_Collider"), reinterpret_cast<CComponent**>(&m_pColliderCom), &BoundingDesc)))
		return E_FAIL;

	m_pColliderCom->Set_PartType(CCollider::PART_BODY);

	m_pColliderCom->OnCollider();
	
	CRigidBody::RIGIDBODY_TYPE eType = CRigidBody::RIGIDBODY_TYPE::DYNAMIC;

	////TODO 리지드바디
	if (FAILED(__super::Add_Component(m_eCurrentLevelID, TEXT("Prototype_Component_RigidBody"),
		TEXT("Com_RigidBody"), reinterpret_cast<CComponent**>(&m_pRigidBodyCom), &eType)))
		return E_FAIL;
	
	m_pRigidBodyCom->Set_Owner(this);
	m_pRigidBodyCom->Set_OwnerNavigation(m_pNavigationCom);
	m_pRigidBodyCom->Clear_NetPower();
	m_pRigidBodyCom->Set_UseGravity(true);
	
	//TODO 상태머신
	if (FAILED(__super::Add_Component(m_eCurrentLevelID, TEXT("Prototype_Component_StateMachine"),
		TEXT("Com_StateMachine"), reinterpret_cast<CComponent**>(&m_pStateCom), this)))
		return E_FAIL;

	return S_OK;
}

HRESULT CPlayer::Ready_PartObjects()
{
	//! For.Part_Body 
	CPartObject::PART_DESC BodyDesc = {};

	BodyDesc.m_pParentTransform = m_pTransformCom;
	BodyDesc.m_pParentNavigation = m_pNavigationCom;
	

	if(FAILED(Add_PartObject(TEXT("Prototype_PartObject_PlayerBody"), TEXT("Part_Body"), &BodyDesc)))
		return E_FAIL;

	

	CPlayerPart_Body* pBody = dynamic_cast<CPlayerPart_Body*>(Find_PartObject(TEXT("Part_Body")));

	//TODO  내 모델 뼈 이름 찾아서 수정하자

	CBone* pSwordBone = pBody->Get_BonePtr("M_WP1");
	
	if(nullptr == pSwordBone)
		return E_FAIL;
	
	//! For.Part_Weapon
	CPartObject::PART_DESC WeaponDesc = {};
	
	WeaponDesc.m_pSocketBone = pSwordBone;
	WeaponDesc.m_pParentTransform = m_pTransformCom;
	
	
	if (FAILED(Add_PartObject(TEXT("Prototype_PartObject_PlayerWeapon"), TEXT("Part_Weapon"), &WeaponDesc)))
		return E_FAIL;


	return S_OK;
}

HRESULT CPlayer::Ready_States()
{

	if(FAILED(m_pStateCom->Add_State(TEXT("PlayerState_Idle"), CPlayerState_Idle::Create(this))))
		return E_FAIL;
	
	if (FAILED(m_pStateCom->Add_State(TEXT("PlayerState_Walk"), CPlayerState_Walk::Create(this))))
		return E_FAIL;

	if (FAILED(m_pStateCom->Add_State(TEXT("PlayerState_Run"), CPlayerState_Run::Create(this))))
		return E_FAIL;

	if (FAILED(m_pStateCom->Add_State(TEXT("PlayerState_Jump"), CPlayerState_Jump::Create(this))))
		return E_FAIL;

	if (FAILED(m_pStateCom->Add_State(TEXT("PlayerState_DashFront"), CPlayerState_DashFront::Create(this))))
		return E_FAIL;

	if (FAILED(m_pStateCom->Add_State(TEXT("PlayerState_DashBack"), CPlayerState_DashBack::Create(this))))
		return E_FAIL;

	if (FAILED(m_pStateCom->Add_State(TEXT("PlayerState_DashLeft"), CPlayerState_DashLeft::Create(this))))
		return E_FAIL;

	if (FAILED(m_pStateCom->Add_State(TEXT("PlayerState_DashRight"), CPlayerState_DashRight::Create(this))))
		return E_FAIL;

	if (FAILED(m_pStateCom->Add_State(TEXT("PlayerState_Attack1"), CPlayerState_Attack1::Create(this))))
		return E_FAIL;

	if (FAILED(m_pStateCom->Add_State(TEXT("PlayerState_Attack2"), CPlayerState_Attack2::Create(this))))
		return E_FAIL;

	if (FAILED(m_pStateCom->Add_State(TEXT("PlayerState_Attack3"), CPlayerState_Attack3::Create(this))))
		return E_FAIL;

	if (FAILED(m_pStateCom->Add_State(TEXT("PlayerState_Attack4"), CPlayerState_Attack4::Create(this))))
		return E_FAIL;

	if (FAILED(m_pStateCom->Add_State(TEXT("PlayerState_Attack5"), CPlayerState_Attack5::Create(this))))
		return E_FAIL;

	if (FAILED(m_pStateCom->Add_State(TEXT("PlayerState_Attack6"), CPlayerState_Attack6::Create(this))))
		return E_FAIL;

	if (FAILED(m_pStateCom->Add_State(TEXT("PlayerState_Attack7"), CPlayerState_Attack7::Create(this))))
		return E_FAIL;

	if (FAILED(m_pStateCom->Add_State(TEXT("PlayerState_Attack8"), CPlayerState_Attack8::Create(this))))
		return E_FAIL;

	
	m_pStateCom->Set_InitState(TEXT("PlayerState_Idle"));
	

	return S_OK;
}

HRESULT CPlayer::Ready_Camera()
{
	CCamera_Target::TARGET_CAMERA_DESC Desc = {};

	Desc.fMouseSensitivity = 0.05f;
	Desc.vEye = _float4(0.f, 20.f, -15.f, 1.f);
	Desc.vAt = _float4(0.f, 0.f, 0.f, 1.f);
	Desc.fFovY = XMConvertToRadians(60.0f);
	Desc.fAspect = (_float)g_iWinSizeX / g_iWinSizeY;
	Desc.fNear = 0.1f;
	Desc.fFar = 1000.f;
	Desc.fSpeedPerSec = 10.f;
	Desc.fRotationPerSec = XMConvertToRadians(180.0f);
	Desc.pTarget = this;

	CGameObject* pCam = nullptr;

	if (FAILED(m_pGameInstance->Add_CloneObject(m_eCurrentLevelID, TEXT("Layer_Camera"), TEXT("Prototype_GameObject_Camera_Target"), &Desc, &pCam)))
		return E_FAIL;

	m_pCamera = dynamic_cast<CCamera_Target*>(pCam);
	Safe_AddRef(m_pCamera);

	return S_OK;
}


HRESULT CPlayer::Add_PartObject(const wstring& strPrototypeTag, const wstring& strPartTag, void* pArg)
{
	if(nullptr != Find_PartObject(strPrototypeTag))
		return E_FAIL;

	CGameObject* pPartObject = m_pGameInstance->Get_CloneObject(strPrototypeTag, pArg);
	
	if(nullptr == pPartObject)
		return E_FAIL;
	
	m_PartObjects.emplace(strPartTag, dynamic_cast<CPartObject*>(pPartObject));
	
	return S_OK;
}

void CPlayer::Key_Input(const _float fTimeDelta)
{
	if (m_pGameInstance->Key_Down(DIK_TAB))
		m_bAdmin = !m_bAdmin;

}

string CPlayer::ConvertWstrToStrTest(const wstring& wstr)
{
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.length()), nullptr, 0, nullptr, nullptr);
	string str(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.length()), &str[0], size_needed, nullptr, nullptr);
	return str;
}


CPlayer * CPlayer::Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext)
{
	CPlayer*		pInstance = new CPlayer(pDevice, pContext);

	/* 원형객체를 초기화한다.  */
	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_BOX("Failed to Created : CPlayer");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject * CPlayer::Clone(void* pArg)
{
	CPlayer*		pInstance = new CPlayer(*this);

	/* 원형객체를 초기화한다.  */
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CPlayer");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CPlayer::Free()
{
	__super::Free();

	for (auto& Pair : m_PartObjects)
		Safe_Release(Pair.second);
	
		m_PartObjects.clear();

	if (nullptr != m_pStateCom)
	{
		m_pStateCom->Clear();
	}

	Safe_Release(m_pCamera);
	Safe_Release(m_pStateCom);
	Safe_Release(m_pColliderCom);
	Safe_Release(m_pRigidBodyCom);
	Safe_Release(m_pNavigationCom);

}

