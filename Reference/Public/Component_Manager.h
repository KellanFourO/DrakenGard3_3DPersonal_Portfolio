#pragma once

#include "Model.h"
#include "Shader.h"
#include "Texture.h"
#include "Collider.h"
#include "Transform.h"
#include "RigidBody.h"
#include "Navigation.h"
#include "StateMachine.h"

#include "VIBuffer_Rect.h"
#include "VIBuffer_Cube.h"
#include "VIBuffer_Field.h"
#include "VIBuffer_Terrain.h"
#include "VIBuffer_Particle_Rect.h"
#include "VIBuffer_Particle_Point.h"
#include "VIBuffer_Dynamic_Terrain.h"



//! 원형 컴포넌트들을 레벨별로 보관할 것이다.
//! 복제하고자 하는 원형을 찾아 복제하여 리턴할 것이다.

BEGIN(Engine)


class CComponent_Manager final : public CBase
{
private:
	CComponent_Manager();
	virtual ~CComponent_Manager() = default;

public:
	HRESULT Initialize(_uint iNumLevels);
	HRESULT Add_Prototype(_uint iLevelIndex, const wstring& strPrototypeTag, class CComponent* pPrototype, _bool bModelCom = false);
	class CComponent* Clone_Component(_uint iLevelIndex, const wstring& strPrototypeTag, void* pArg);
	void	Clear(_uint iLevelIndex);

private:
	_uint		m_iNumLevels = { 0 };

	map<const wstring, class CComponent*>*	m_pPrototypes = { nullptr };
	typedef map<const wstring, class CComponent*>	PROTOTYPES;

private:
	class CComponent*	Find_Prototype(_uint iLevelIndex, const wstring& strPrototypeTag);
	class CGameInstance* m_pGameInstance = { nullptr };

public:
	static CComponent_Manager*	Create(_uint iNumLevels);
	virtual void Free() override;
};

END

