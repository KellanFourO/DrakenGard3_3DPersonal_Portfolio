#pragma once
#include "VIBuffer.h"

BEGIN(Engine)

class CMesh final : public CVIBuffer
{
private:
	CMesh(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CMesh(const CMesh& rhs);
	virtual ~CMesh() = default;

public:
	_uint	Get_MaterialIndex() const { return m_iMaterialIndex; }

public:
	virtual HRESULT Initialize_Prototype(const aiMesh* pAIMesh, _fmatrix PivotMatrix);
	virtual HRESULT Initialize(void* pArg) override;

private:
	char		m_szName[MAX_PATH]; //! �޽� �̸� ���� �޾�����
	_uint		m_iMaterialIndex = { 0 };

public:
	//TODO Create�ÿ� ���ڰ����� const aiMesh* �� �޴� ���� �� �� �ִ�.
	//! aiMesh�� ����� ���̺귯���� �о���� aiScene ����ü ���ο� ����ִ�.
	//! �׷��� �翬�� �о���̸鼭 Mesh�� �������شٴ� �������� ���� �ɰ�.
	static CMesh* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const aiMesh* pAIMesh, _fmatrix PivotMatrix);
	virtual CComponent* Clone(void* pArg);
	virtual void Free() override;

};

END
