#pragma once

#include "Client_Defines.h"
#include "NonAnimObject.h"

BEGIN(Engine)
class CShader;
class CTexture;
class CVIBuffer_Particle_Point;
END

BEGIN(Client)

class CParticle_BossBreath final : public CNonAnimObject
{
public:
	typedef struct tagParticleObjectDesc : public CGameObject::GAMEOBJECT_DESC
	{
		wstring strTextureTag;
		_int	iShaderPathIndex;
		_uint	iNumInstance;
		_float3 vCenter;
		_float	fRange;
		_float2 vSpeed;
		_float2 vScale;
		_float3 vRotation;
		_float4 vColor;
		_float2 vLifeTime;
		_float4 vDir = { 1.f, 0.f, 0.f, 0.f };
		_bool	bRandom = true;
		_float2	vRandomRotation = {};
		_float3	vInterval = { 1.f, 1.f, 1.f };
		_float fAge;
	}PARTICLE_DESC;

private:
	CParticle_BossBreath(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CParticle_BossBreath(const CParticle_BossBreath& rhs);
	virtual ~CParticle_BossBreath() = default;

public:
	void	Start_Particle() { m_bParticleStart = true; }
	void	End_Particle() { m_bParticleStart = false; }

public:
	virtual HRESULT Initialize_Prototype(LEVEL eLevel);
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Priority_Tick(_float fTimeDelta) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	CShader*						m_pShaderCom = { nullptr };
	CTexture*						m_pTextureCom = { nullptr };
	CVIBuffer_Particle_Point*		m_pVIBufferCom = { nullptr };

private:
	PARTICLE_DESC					m_tParticleDesc = {};
	_bool							m_bParticleStart = false;

private:
	HRESULT Ready_Components(void* pArg);
	HRESULT Bind_ShaderResources();


public:
	/* ������ü�� �����Ѵ�. */
	static CParticle_BossBreath* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eLevel);

	/* �纻��ü�� �����Ѵ�. */
	virtual CGameObject* Clone(void* pArg) override;

	virtual void Free() override;
};

END