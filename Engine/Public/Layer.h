#pragma once
#include "Base.h"

BEGIN(Engine)


class CLayer final : public CBase
{
private:
	CLayer();
	virtual ~CLayer() = default;

public:
	class CComponent*	Get_Component(const wstring& strComponentTag, _uint iIndex);
	list<class CGameObject*>& Get_ObjectList() { return m_GameObjects; }
	class CComponent*	Get_PartComponent(const wstring& strComponentTag, _uint iIndex, const wstring& strPartTag);

	class CGameObject*	Get_Player();

public:
	HRESULT Add_GameObject(class CGameObject* pGameObject);
	HRESULT Erase_GameObject(class CGameObject* pGameObject);
	void	Priority_Tick(_float fTimeDelta);
	void	Tick(_float fTimeDelta);
	void	Late_Tick(_float fTimeDelta);

private:
	list<class CGameObject*>	m_GameObjects;
	
	_float						m_fTimeAcc = 0.f;

public:
	static CLayer* Create();
	virtual void Free() override;

};

END

