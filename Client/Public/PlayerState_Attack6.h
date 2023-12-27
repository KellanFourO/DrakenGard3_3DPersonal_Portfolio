#pragma once
#include "PlayerState_NormalAttackBase.h"

BEGIN(Client)
class CPlayer;

class CPlayerState_Attack6 final : public CPlayerState_NormalAttackBase
{
private:
		 CPlayerState_Attack6();
virtual ~CPlayerState_Attack6() = default;

public:
	virtual HRESULT Initialize(CPlayer* pPlayer) override;
	virtual HRESULT StartState() override;
	virtual HRESULT EndState() override;

	virtual void	Tick(const _float& fTimeDelta) override;
	virtual void	Late_Tick(const _float& fTimeDelta) override;

public:
	static CPlayerState_Attack6* Create(CPlayer* pPlayer);
	virtual void Free() override;
};

END
