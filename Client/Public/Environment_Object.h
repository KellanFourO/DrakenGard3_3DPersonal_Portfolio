#pragma once
#include "NonAnimObject.h"

BEGIN(Engine)
class CNavigation;
END

BEGIN(Client)

class CEnvironment_Object final : public CNonAnimObject
{
public:
	typedef struct tagEnvironmentDesc : public CGameObject::GAMEOBJECT_DESC
	{
		_float4 vPos = { 0.f, 0.f, 0.f, 0.f };
		wstring	strModelTag;
		_float4x4 WorldMatrix;
	}ENVIRONMENT_DESC;

private:
	CEnvironment_Object(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CEnvironment_Object(const CEnvironment_Object& rhs);
	virtual ~CEnvironment_Object() = default;

public:
	CNavigation*	Get_NaviCom() { return m_pNavigationCom; }

public:
	virtual HRESULT Initialize_Prototype(LEVEL eLevel);
	virtual HRESULT Initialize(void* pArg);
	virtual void Priority_Tick(_float fTimeDelta) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_Shadow() override;

private:
	HRESULT Ready_Components();
	HRESULT Bind_ShaderResources();

private:
	CNavigation*		m_pNavigationCom = { nullptr };

private:
	wstring				m_strModelTag = TEXT("");




public:
	/* 원형객체를 생성한다. */
	static CEnvironment_Object* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eLevel);

	/* 사본객체를 생성한다. */
	virtual CGameObject* Clone(void* pArg) override;

	virtual void Free() override;
};

END

