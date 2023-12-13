#pragma once
#include "Base.h"
#include "Client_Defines.h"


BEGIN(Client)
class CImgui_Manager final : public CBase
{
	DECLARE_SINGLETON(CImgui_Manager);

public:
	enum TOOLID { TOOL_MAP, TOOL_OBJECT, TOOL_CAMERA, TOOL_EFFECT, TOOL_END };
	enum DIALOGID { DIALOG_SAVE, DIALOG_LOAD, DIALOG_END };
	enum BRUSHMODE { BRUSH_DOWN, BRUSH_UP, BRUSH_PRESSING, BRUSH_END };

private:
			 CImgui_Manager();
	virtual ~CImgui_Manager() = default;
	
public:
	LEVEL					Get_Level_ID() { return m_eLevelID; }
	void					Set_Level_ID(LEVEL eLevel) { m_eLevelID = eLevel; }
	class CCamera_MapTool** Get_Cam() { return &m_pCamera; }

public:
	HRESULT					Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	void					Tick(_float fTimeDelta);
	void					Render();

private:
	HRESULT					ImGui_Initialize();
	void					ImGuizmo_Initialize();
	void					ImGuiFileDialog_Intialize();

private:
	void					ImGui_MainTick();
	void					ImGui_MapToolTick();
	void					ImGui_ObjectToolTick();

private:
	_bool					ImGui_MouseInCheck();
	void					UpdateRay();

//TODO Imguizmo #기즈모
//TODO ImguiDialog #다이얼로그
//TODO 맵툴 #맵툴
//TODO 오브젝트툴 #오브젝트툴
//TODO 묹문자열 #문자열함수	 

private:
	class CGameInstance* m_pGameInstance = { nullptr };
	class CDynamic_Terrain* m_pDynamic_Terrain = { nullptr };
	class CCamera_MapTool* m_pCamera = { nullptr };
	
private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };

private:
	LEVEL					m_eLevelID = { LEVEL_END };
	TOOLID					m_eToolID = { TOOL_END };


private: //TODO Imguizmo #기즈모
	void					Set_Guizmo();
	void					Set_GuizmoCamView();
	void					Set_GuizmoCamProj();

private:
	_float*						m_arrView = { nullptr };
	_float*						m_arrProj = { nullptr };
	_float						snap[3] = { 1.f, 1.f, 1.f };
		 

private: //TODO ImguiDialog #다이얼로그
		void				OpenDialog(TOOLID eToolID);
		void				ShowDialog(TOOLID eToolID);

private:
		
		ImGuiFileDialog*		m_pFileDialog;
		string					m_strCurrentDialogTag;
		DIALOGID				m_eDialogMode = { DIALOG_END };
		

private: //TODO 맵툴 #맵툴 
	void					PickingTerrain(BRUSHMODE eBrushMode);
	void					SaveMap(string strFilePath, string strFileName);
	void					LoadMap(string strFilePath, string strFileName);

private:
	_bool						m_bReady = true;
	RAY							m_tWorldRay = {};

private:
	VTXDYNAMIC					m_tMapInfo;
	_bool						m_bMapToolPickMode = { false };
	BRUSHMODE					m_eBrushMode = { BRUSHMODE::BRUSH_END };

	_int						m_iTileMode = { 0 };
	_float						m_fTileX = { 0 }, m_fTileZ = { 0 };

	_int						m_iBrushRange = 1.f, m_iBrushPower = 1.f;
		 

private: //TODO 오브젝트툴 #오브젝트툴
	HRESULT					Add_PrototypeTag(const wstring& strPrototypeTag);
	HRESULT					Ready_ProtoTagList();

	void					CreateObjectFunction();
	void					SelectObjectFunction();

	void					SaveObject(string strFilePath);
	void					LoadObject(string strFilePath);

private: 
	_bool						m_bObjectToolPickMode;
	_bool						m_bPressing;

	_int						m_iObjectMode = { 0 };
	vector<string>				m_vecObjectProtoTags;
	_int						m_iSelectTagIndex = { 0 };

	vector<string>				m_vecCreateObjectTag;
	vector<CGameObject*>		m_vecObjects;
	CGameObject*				m_PickingObject = nullptr;
	_int						m_iPickingObjectIndex = 0;
	_float3						m_fPickingPos = { 0.f, 0.f, 0.f };

	

private: //TODO 문자열 #문자열함수
	string					ConvertWstrToStr(const wstring& str);
	wstring					ConvertStrToWstr(const string& str);
	wstring					SliceObjectTag(const wstring& strObjectTag);
		 

public:
	virtual void			Free() override;

};
END
