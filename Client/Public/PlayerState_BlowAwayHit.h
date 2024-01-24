#pragma once
#include "PlayerState_Base.h"

BEGIN(Client)		
class CPlayer;

class CPlayerState_BlowAwayHit final : public CPlayerState_Base 
{
private:
		 CPlayerState_BlowAwayHit();
virtual ~CPlayerState_BlowAwayHit() = default;

public:
	virtual HRESULT Initialize(CPlayer* pPlayer) override;
	virtual HRESULT StartState() override;
	virtual HRESULT EndState() override;

	virtual void	Tick(const _float& fTimeDelta) override;

public:
	void			Set_ImpulsePos(_float3 vImpulsePos) { m_vImpulsePos = vImpulsePos; }

private:
	_bool			m_bCurrentHitAnimEnd[3] = { false, false, false};
	_bool			m_bKeyInput = false;
	_float3			m_vImpulsePos = {};

public:
	static CPlayerState_BlowAwayHit* Create(CPlayer* pPlayer);
	virtual void Free() override;
};

END

