#pragma once
#include "VIBuffer.h"

BEGIN(Engine)

class ENGINE_DLL CVIBuffer_Dynamic_Terrain final : public CVIBuffer
{
private:
	CVIBuffer_Dynamic_Terrain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CVIBuffer_Dynamic_Terrain(const CVIBuffer_Dynamic_Terrain& rhs);
	virtual ~CVIBuffer_Dynamic_Terrain() = default;

public:
	_bool			Get_Vertex(_uint _iIndex, VTXDYNAMIC * _pOut);
	_bool			Get_Indices(_uint _iIndex, _uint3 * _pOut);

public:
	_bool			Compute_MousePos(RAY _Ray, _matrix _WorldMatrix, _float3 * pOut);

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg) override;

public:
	void			Update(_vector _vMousePos, _float _fRadious, _float _fPower, _uint _iMode);

private:
	_uint					m_iNumVerticesX = 0, m_iNumVerticesZ = 0;

	_float					m_fInterval = { 0.f };


	vector<_uint3>			m_Indices;
	vector<VTXDYNAMIC>		m_VertexInfo;

public:
	static	CVIBuffer_Dynamic_Terrain* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END