#pragma once
#include "PlayerState_Base.h"

BEGIN(Client)
class CPlayer;

class CPlayerState_Idle final : public CPlayerState_Base 
{
private:
		 CPlayerState_Idle();
virtual ~CPlayerState_Idle() = default;

public:
	virtual HRESULT Initialize(CPlayer* pPlayer) override;
	virtual HRESULT StartState() override;
	virtual HRESULT EndState() override;

	virtual void	Tick(const _float& fTimeDelta) override;

private:
	virtual void	KeyInput(const _float& fTimeDelta) override;

public:
	static CPlayerState_Idle* Create(CPlayer* pPlayer);
	virtual void Free() override;
};

END

