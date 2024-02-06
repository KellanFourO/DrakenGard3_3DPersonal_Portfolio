#include "stdafx.h"
#include "Player.h"
#include "PlayerState_BloodyMode_Run.h"
#include "PlayerPart_Body.h"

#include "GameInstance.h"
#include "StateMachine.h"
#include "Model.h"
#include "Transform.h"
#include "Navigation.h"
#include "RigidBody.h"
#include "Camera_Target.h"
#include "Effect_Hanabira.h"
#include "Effect_RunSmoke.h"

CPlayerState_BloodyMode_Run::CPlayerState_BloodyMode_Run()
{
}

HRESULT CPlayerState_BloodyMode_Run::Initialize(CPlayer* pPlayer)
{
	if (FAILED(__super::Initialize(pPlayer)))
		return E_FAIL;

	m_fEndTime = 0.25f; //! 일정시간 키 입력 없을시 Idle 상태로 돌아가기.
	
	return S_OK;
}

HRESULT CPlayerState_BloodyMode_Run::StartState()
{
    m_pOwnerModelCom->Set_Loop(true);
	m_pOwnerModelCom->Set_Animation(449);
    m_pOwnerModelCom->Set_AnimationSpeed(3.f);
	m_pOwnerTransform->Set_SpeedPerSec(30.f);
	m_pOwnerTransform->Set_RotationPerSec(XMConvertToRadians(90.0f));
   

	return S_OK;
}

HRESULT CPlayerState_BloodyMode_Run::EndState()
{
	m_fLastInputTime = 0.f;
	m_fAccTime = 0.f;
    m_fHanabiraAccTime = 0.f;
    m_pOwnerModelCom->Reset_AnimationSpeed();


	return S_OK;
}

void CPlayerState_BloodyMode_Run::Tick(const _float& fTimeDelta)
{
	KeyInput(fTimeDelta);

    m_fSoundAccTime += fTimeDelta;

    if (m_fSoundAccTime > m_fSoundPlayTime && m_bFootStep1 == false)
    {
        m_pGameInstance->Play_Sound(L"PLAYER_EFFECT", L"Run1.wav", SOUND_EFFECT2, 3.f);
        m_bFootStep1 = true;
        m_fSoundAccTime = 0.f;
    }
    
    if (m_fSoundAccTime > m_fSoundPlayTime && m_bFootStep1 == true)
    {
        m_bFootStep1 = false;
        m_pGameInstance->Play_Sound(L"PLAYER_EFFECT", L"Run2.wav", SOUND_EFFECT2, 3.f);
        m_fSoundAccTime = 0.f;
    }

    //CreateHanabira(fTimeDelta);
}

void CPlayerState_BloodyMode_Run::Late_Tick(const _float& fTimeDelta)
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

void CPlayerState_BloodyMode_Run::KeyInput(const _float& fTimeDelta)
{


    if (m_pGameInstance->Key_Pressing(DIK_W))
    {
        if (m_pGameInstance->Key_Pressing(DIK_SPACE))
        {
            m_pOwnerStateCom->Transition(CStateMachine::STATETYPE::STATE_GROUND, TEXT("PlayerState_DashFront"));
        }




        m_ePrevDir = m_eCurrentDir;
        m_eCurrentDir = CPlayerState_Base::FRONT;

        if (m_ePrevDir != m_eCurrentDir)
        {
            Vertical_Camera_Rotate();
        }

        _float3 vCamLook;
        XMStoreFloat3(&vCamLook, m_pOwnerCam->Get_Transform()->Get_State(CTransform::STATE_LOOK));

        m_pOwnerTransform->Go_Player_Straight(fTimeDelta, vCamLook, m_pOwnerNavagation);
        m_fLastInputTime = fTimeDelta;
    }

    if (m_pGameInstance->Key_Pressing(DIK_A))
    {

        if (m_pGameInstance->Key_Pressing(DIK_SPACE))
        {
            m_pOwnerStateCom->Transition(CStateMachine::STATETYPE::STATE_GROUND, TEXT("PlayerState_DashLeft"));
        }


        m_ePrevDir = m_eCurrentDir;
        m_eCurrentDir = CPlayerState_Base::LEFT;

        if (m_ePrevDir != m_eCurrentDir)
        {
            Horizon_Camera_Rotate();
        }

        _vector vCamLook = m_pOwnerCam->Get_Transform()->Get_State(CTransform::STATE_LOOK);
        _vector vCamUp = m_pOwnerCam->Get_Transform()->Get_State(CTransform::STATE_UP);

        _float3 vCamLeft;
        XMStoreFloat3(&vCamLeft, XMVector3Cross(vCamLook, vCamUp));



        m_pOwnerTransform->Go_Player_Straight(fTimeDelta, vCamLeft, m_pOwnerNavagation);
        m_fLastInputTime = fTimeDelta;
    }

    if (m_pGameInstance->Key_Pressing(DIK_S))
    {

        if (m_pGameInstance->Key_Pressing(DIK_SPACE))
        {
            m_pOwnerStateCom->Transition(CStateMachine::STATETYPE::STATE_GROUND, TEXT("PlayerState_DashBack"));
        }

        m_ePrevDir = m_eCurrentDir;
        m_eCurrentDir = CPlayerState_Base::BACK;

        if (m_ePrevDir != m_eCurrentDir)
        {
            Vertical_Camera_Rotate();
        }

        m_pOwnerTransform->Go_Straight(fTimeDelta, m_pOwnerNavagation);
        m_fLastInputTime = fTimeDelta;
    }

    if (m_pGameInstance->Key_Pressing(DIK_D))
    {

        if (m_pGameInstance->Key_Pressing(DIK_SPACE))
        {
            m_pOwnerStateCom->Transition(CStateMachine::STATETYPE::STATE_GROUND, TEXT("PlayerState_DashRight"));
        }

        m_ePrevDir = m_eCurrentDir;
        m_eCurrentDir = CPlayerState_Base::RIGHT;

        if (m_ePrevDir != m_eCurrentDir)
        {
            Horizon_Camera_Rotate();
        }

        _vector vCamLook = m_pOwnerCam->Get_Transform()->Get_State(CTransform::STATE_LOOK);
        _vector vCamUp = m_pOwnerCam->Get_Transform()->Get_State(CTransform::STATE_UP);

        _float3 vCamRight;
        XMStoreFloat3(&vCamRight, XMVector3Cross(vCamUp, vCamLook));


        m_pOwnerTransform->Go_Player_Straight(fTimeDelta, vCamRight, m_pOwnerNavagation);
        m_fLastInputTime = fTimeDelta;
    }

    //if (m_pGameInstance->Key_Down(DIK_SPACE))
    //{
    //    m_pOwnerStateCom->Transition(CStateMachine::STATETYPE::STATE_AIR, TEXT("PlayerState_Jump"));
    //}

    if (m_pGameInstance->Mouse_Down(DIM_LB))
    {
        m_pOwnerStateCom->Transition(CStateMachine::STATETYPE::STATE_GROUND, TEXT("PlayerState_Attack1"));
    }
}

void CPlayerState_BloodyMode_Run::CreateHanabira(_float fTimeDelta)
{
    m_fHanabiraAccTime += fTimeDelta;

    if (m_fHanabiraAccTime >= m_fHanabiraCreateTime)
    {
       

        for (_int i = 0; i < 20; ++i)
        {
            _vector vMyPos = m_pOwnerTransform->Get_State(CTransform::STATE_POSITION);

            _float4 vRandomPos = m_pOwnerTransform->Get_RandomPositionAroundCenter(vMyPos, XMConvertToRadians(360.f));

            CEffect_Hanabira::HANABIRA_DESC Desc;

            Desc.fSpeedPerSec = 5.f;
            Desc.fRotationPerSec = XMConvertToRadians(90.f);
            Desc.pTarget = m_pGameInstance->Get_Player(LEVEL_GAMEPLAY);
            Desc.vPos = vRandomPos;
            Desc.fLifeTime = 1.5f;
            Desc.eType = CEffect_Hanabira::HANABIRA_SPREAD;
            XMStoreFloat4(&Desc.vLook, m_pOwnerTransform->Get_State(CTransform::STATE_LOOK));

            m_pGameInstance->Add_CloneObject(LEVEL_GAMEPLAY, TEXT("Layer_Effect"), TEXT("Prototype_GameObject_Effect_Hanabira"), &Desc);
        }

        
        

        CEffect_RunSmoke::EFFECT_DESC Desc;

        Desc.fLifeTime = 2.f;
        Desc.fPlaySpeed = 0.05f;
        XMStoreFloat4(&Desc.vCreatePos, m_pOwnerTransform->Get_State(CTransform::STATE_POSITION));
        Desc.vCreatePos.y += 0.2f;

        XMStoreFloat3(&Desc.vDir, -m_pOwnerTransform->Get_State(CTransform::STATE_LOOK));
        Desc.vScale = _float3{ 1.4f, 1.4f, 1.f };
        
        m_pGameInstance->Add_CloneObject(LEVEL_GAMEPLAY, TEXT("Layer_Effect"), TEXT("Prototype_GameObject_Effect_RunSmoke"), &Desc);


        m_fHanabiraAccTime = 0.f;
    }


    
}

CPlayerState_BloodyMode_Run* CPlayerState_BloodyMode_Run::Create(CPlayer* pPlayer)
{
	CPlayerState_BloodyMode_Run* pInstance = new CPlayerState_BloodyMode_Run();

	/* 원형객체를 초기화한다.  */
	if (FAILED(pInstance->Initialize(pPlayer)))
	{
		MSG_BOX("Failed to Created : CPlayerState_BloodyMode_Run");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CPlayerState_BloodyMode_Run::Free()
{
	__super::Free();
}
