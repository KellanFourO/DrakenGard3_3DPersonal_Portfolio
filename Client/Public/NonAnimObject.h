#pragma once
#include "GameObject.h"
#include "Client_Defines.h"

BEGIN(Client)

class CNonAnimObject abstract : public CGameObject
{
protected:
	CNonAnimObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CNonAnimObject(const CNonAnimObject& rhs);
	virtual ~CNonAnimObject() = default;


protected:
	LEVEL				m_eCurrentLevelID = { LEVEL_END };

};

END

