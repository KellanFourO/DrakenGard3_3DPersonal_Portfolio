#pragma once
#include "Base.h"
#include "Client_Defines.h"

BEGIN(Client)
class CImgui_Manager final : public CBase
{
	DECLARE_SINGLETON(CImgui_Manager);

public:
	enum TOOLID { TOOL_MAP, TOOL_OBJECT, TOOL_CAMERA, TOOL_EFFECT, TOOL_END };
	enum MAPTAPID { TAP_TILE, TAP_ENVIRONMENT, TAP_END };
	

private:
			 CImgui_Manager();
	virtual ~CImgui_Manager() = default;

public:
	HRESULT Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	void	Tick(_float fTimeDelta);
	void	Render();

	LEVEL		  Get_Level_ID() { return m_eLevelID; }
	void		  Set_Level_ID(LEVEL eLevel) { m_eLevelID = eLevel; }

	void		KeyInput();

//TODO For.MapTool Start
public:
	HRESULT	Save_EditTexture();
	void	MapToolKeyInput();

private:
	ID3D11Texture2D*	m_pTexture2D = { nullptr };
	char*		ConvertWCtoC(const wchar_t* str);
	wchar_t*	ConvertCtoWC(const char* str);

	

private:
	class CGameInstance*		m_pGameInstance = { nullptr };
	class CDynamic_Terrain*		m_pDynamic_Terrain = { nullptr };
	class CTestTree*			m_pTestTree = { nullptr };

private:
	ID3D11Device*			m_pDevice = { nullptr };
	ID3D11DeviceContext*	m_pContext = { nullptr };

	_bool					m_bReady = true;
	_bool					m_bMainTool = { true };
	_bool					m_bMapTool, m_bEffectTool, m_bObjectTool, m_bCameraTool = { false };
	LEVEL					m_eLevelID = { LEVEL_END };
	TOOLID					m_eToolID = { TOOL_END };

	RAY						m_tRay;

//TODO ���� ���� ����
private:
	MAPTAPID	m_eMapTapID = { TAP_END };

private: //! For. Tile
	VTXDYNAMIC	m_tMapInfo;
	_float		m_fTileX = { 0.0f };
	_float		m_fTileZ = { 0.0f };
	_bool		m_bTileing = { false };
	_bool		m_bCreate = { false };
	_int		m_iTileMode = { 0 };
	_int		m_iBrushRange = 1.f;
	_int		m_iBrushPower = 1.f;

private: //!For.Environment
	_bool		m_bPicking = { false };
	_float3		m_fPickingPos = { 0.f, 0.f, 0.f };
	_int		m_iEnvironmentMode = { 0 };
	vector<CTestTree*>		m_vecObject;

//TODO ���� ���� ����

	

private:
	void	HelpMarker(const char* desc);
	char* ConverWStringtoC(const wstring& wstr);

private:
	void	ShowMapTool();
	void	ShowObjectTool();
	void	ShowCameraTool();
	void	ShowEffectTool();
	
private:
	void	Picking(TOOLID eToolID, _int iMode);
	
	

public:
	virtual void Free() override;
};
END
