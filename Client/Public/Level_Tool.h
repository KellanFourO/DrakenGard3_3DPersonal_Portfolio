#pragma once

#include "Client_Defines.h"
#include "Level.h"

BEGIN(Client)

class CLevel_Tool final : public CLevel
{
private:
	CLevel_Tool(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CLevel_Tool() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;

private:
	HRESULT Ready_Imgui();
	HRESULT Ready_Layer_Camera(const wstring& strLayerTag);
	HRESULT Ready_Layer_BackGround(const wstring& strLayerTag);
	HRESULT Ready_Layer_Effect(const wstring& strLayerTag);

public:
	static CLevel_Tool* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual void Free() override;

private:
	class CImgui_Manager*	m_pImguiManager = { nullptr };
	class CDynamic_Terrain*	m_pDynamic_Terrain = { nullptr };
	_bool					m_bModeChange = false;
};

END