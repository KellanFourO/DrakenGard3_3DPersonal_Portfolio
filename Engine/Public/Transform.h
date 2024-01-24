#pragma once

#include "Component.h"

//TODO 트랜스폼 컴객체의 역할
//! 객체들의 월드 상태를 표현하기 위한 데이터를 가진다. ( 월드행렬 )
//! 해당 월드 상에서의 변환을 위한 기능을 가진다.

BEGIN(Engine)

class ENGINE_DLL CTransform final : public CComponent
{
public:
	enum STATE { STATE_RIGHT, STATE_UP, STATE_LOOK, STATE_POSITION, STATE_END };

	CTransform(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTransform(const CTransform& rhs);
	virtual ~CTransform() = default;

public:
	_float	Get_Speed() { return m_fSpeedPerSec; }
	_float  Get_RotationSpeed() { return m_fRotationPerSec; }


	//! 행렬 행 정보 교체
	void Set_State(STATE eState, const _float4& vState)
	{
		memcpy(&m_WorldMatrix.m[eState],&vState, sizeof(_float3));
	}

	void Set_State(STATE eState, _fvector vState)
	{
		m_WorldMatrix.m[eState][0] = XMVectorGetX(vState);
		m_WorldMatrix.m[eState][1] = XMVectorGetY(vState);
		m_WorldMatrix.m[eState][2] = XMVectorGetZ(vState);
		m_WorldMatrix.m[eState][3] = XMVectorGetW(vState);
	}

	

	_vector Get_State(STATE eState)
	{
		return XMVectorSet
			(
				m_WorldMatrix.m[eState][0],
				m_WorldMatrix.m[eState][1],
				m_WorldMatrix.m[eState][2],
				m_WorldMatrix.m[eState][3]
			);
	}

	_float3 Get_Scaled()
	{
		return _float3
		(
			XMVectorGetX(XMVector3Length(XMLoadFloat4x4(&m_WorldMatrix).r[STATE_RIGHT])),
			XMVectorGetX(XMVector3Length(XMLoadFloat4x4(&m_WorldMatrix).r[STATE_UP])),
			XMVectorGetX(XMVector3Length(XMLoadFloat4x4(&m_WorldMatrix).r[STATE_LOOK]))
		);
	}

	_float3 Get_Pos()
	{
		return _float3
		(
			m_WorldMatrix.m[STATE_POSITION][0],
			m_WorldMatrix.m[STATE_POSITION][1],
			m_WorldMatrix.m[STATE_POSITION][2]
		);
	}

	_float3	Get_TranslatePos() { return m_vTranslatePos; }

	_matrix Get_WorldMatrix()
	{
		return XMLoadFloat4x4(&m_WorldMatrix);
	}

	void	Set_WorldFloat4x4(_float4x4 mat4x4)
	{
		XMStoreFloat4x4(&m_WorldMatrix, XMLoadFloat4x4(&mat4x4));
	}

	_float4x4 Get_WorldFloat4x4()
	{
		return m_WorldMatrix;
	}

	_matrix Get_WorldMatrixInverse()
	{
		return XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix));
	}

	_float4x4 Get_WorldFloat4x4Inverse()
	{
		_float4x4 InverseMatrix;
		XMStoreFloat4x4(&InverseMatrix, XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix)));

		return InverseMatrix;
	}

	void Set_Scaling(_float fScaleX, _float fScaleY, _float fScaleZ);

	void Set_SpeedPerSec(_float fSpeedPerSec) { m_fSpeedPerSec = fSpeedPerSec; }
	void Set_RotationPerSec(_float fRotationPerSec) { m_fRotationPerSec = fRotationPerSec; }

	void    Add_LookPos(_float3& _vAddPos);
	_float3 Get_MoveAxisPos(_float fDistance, STATE eState, _bool bAddType, _float3 vCamLook);

public:
	void	Go_Player_Straight(_float fTimeDelta, _float3 vCamLook, class CNavigation* pNavigation = nullptr);
	void	Go_Player_Left(_float fTimeDelta, _float3 vCamLook, class CNavigation* pNavigation = nullptr);
	void	Go_Player_Right(_float fTimeDelta, _float3 vCamLook, class CNavigation* pNavigation = nullptr);
	void	Go_Player_Backward(_float fTimeDelta, _float3 vCamLook, class CNavigation* pNavigation = nullptr);
	void	Go_Player_Up(_float fTimeDelta, _float3 vCamLook, class CNavigation* pNavigation = nullptr);
	void	Go_Player_Down(_float fTimeDelta, _float3 vCamLook, class CNavigation* pNavigation = nullptr);

	void	Go_Straight(_float fTimeDelta, class CNavigation* pNavigation = nullptr);
	void	Go_Left(_float fTimeDelta, class CNavigation* pNavigation = nullptr);
	void	Go_Right(_float fTimeDelta, class CNavigation* pNavigation = nullptr);
	void	KeepEye(_float fTimeDelta, _bool bRight = false, class CNavigation* pNavigation = nullptr);
	void	Go_Backward(_float fTimeDelta, class CNavigation* pNavigation = nullptr);
	void	Go_Up(_float fTimeDelta, class CNavigation* pNavigation = nullptr);
	void	Go_Down(_float fTimeDelta, class CNavigation* pNavigation = nullptr);
	
	

	void	Turn(_fvector vAxis, _float fTimeDelta);
	_bool	AreVectorsAligned(const _fvector& v1, const _fvector& v2, _float fTolerance = 0.001f);
	void	Rotation(_fvector vAxis, _float fRadian);
	void	RotationOfCameraDir(_fvector vCamLook, _float fRadian);
	_bool	TurnToTarget(const _fvector& vTargetPosition, _float fTimeDelta);
	
	

	void	Go_Target(_fvector vTargetPos, _float fTimeDelta, _float fSpare = 0.1f);
	void	Go_Target_Navi(_fvector vTargetPos, _float fTimeDelta, class CNavigation* pNavigation, _float fSpare = 0.1f);

	void	Look_At(_fvector vTargetPos);
	void	Look_At_CamLook(_float3 vCamLook);
	void	Look_At_OnLand(_fvector vTargetPos);
	_bool	HasArrived(const DirectX::XMFLOAT3& _vCurrentPos, const DirectX::XMFLOAT3& _vTargetPos, _float fArrivalThreshold);

	//TODO Translate
	void	Translate(const _float3& vTranslation, class CNavigation* pNavigation, _bool bNotAgent = FALSE);

	void	Translate(const _float4& vTranslation, class CNavigation* pNavigation, _bool bNotAgent = FALSE)
	{ 	
		Translate(_float3(vTranslation.x, vTranslation.y, vTranslation.z), pNavigation, bNotAgent);
	}
	

public:
	virtual void Write_Json(json& Out_Json) override;
	virtual void Load_FromJson(const json& In_Json) override;

public:
	virtual HRESULT Initialize_Prototype(_float fSpeedPerSec, _float fRotationPerSec);
	
public:
	HRESULT	Bind_ShaderResource(class CShader* pShader, const _char* pConstantName);

private:
	_float				m_fSpeedPerSec = { 0.0f };
	_float				m_fRotationPerSec = { 0.0f };
	
	_float3				m_vTranslatePos = {};

	_float4x4			m_WorldMatrix;

public:
	static	CTransform* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _float fSpeedPerSec, _float fRotationPerSec);
	virtual CComponent* Clone(void* pArg) override; //! 복사를 강제한다
	virtual void Free() override;
};

END

