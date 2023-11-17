#pragma once

#include "Component.h"
BEGIN(Engine)

class ENGINE_DLL CShader final : public CComponent
{

protected:
	CShader(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CShader(const CShader& rhs);
	virtual ~CShader() = default;

public:
	virtual HRESULT Initialize_Prototype(const wstring & strShaderFilePath, const D3D11_INPUT_ELEMENT_DESC * pElements, _uint iNumElements); //! ������Ÿ�� ������ ����Ѵٴ� ���� �� �� �ִ�
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Begin(_uint iPassIndex);
	HRESULT	Bind_Matrix(const _char* pConstantName, const _float4x4* pMatrix);

private:
	//!���� �����Ͽ� �������� ���̴� ���� �ϳ��� ��ǥ�ϴ� �İ�ü
	D3DX11_TECHNIQUE_DESC		m_TechniqueDesc;
	ID3DX11Effect*				m_pEffect = { nullptr };
	vector<ID3D11InputLayout*>	m_InputLayouts;

public:
	static CShader* Create(ID3D11Device * pDevice, ID3D11DeviceContext * pContext, const wstring & strShaderFilePath, const D3D11_INPUT_ELEMENT_DESC * pElements, _uint iNumElements);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

END
