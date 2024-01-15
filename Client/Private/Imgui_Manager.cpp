#pragma once
#include "stdafx.h"

#include "Tool_Define.h"
#include "Imgui_Manager.h"
#include "GameInstance.h"
#include "Dynamic_Terrain.h"
#include "Field.h"
#include "Camera_MapTool.h"
#include "Bone.h"
#include "Model.h"
#include "MonsterPart_EN00_Weapon.h"
#include "Monster_EN00.h"
#include "BoundingBox_AABB.h"
#include "Layer.h"

#include "Navigation.h"
#include "Cell.h"


#include <regex>
#include <codecvt>
#include <filesystem>
#include <iostream>
#include <fstream>
//#include "NonAnimObject.h"
#include "Environment_Object.h"
#include "../../Reference/Public/Delaunator/delaunator.hpp"

ImGuiIO g_io;
static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE	   mCurrentGizmoMode(ImGuizmo::WORLD);
static bool useSnap(false);

IMPLEMENT_SINGLETON(CImgui_Manager);

ImGuiFileDialog* m_pFileDialog;

CImgui_Manager::CImgui_Manager()
	: m_bReady(true)
	, m_bMapToolPickMode(false), m_eBrushMode(BRUSHMODE::BRUSH_END), m_iTileMode(0)
	, m_bObjectToolPickMode(false)
{
}

HRESULT CImgui_Manager::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	m_eLevelID = LEVEL_TOOL;

	m_pDevice = pDevice;
	m_pContext = pContext;
	m_pGameInstance = CGameInstance::GetInstance();

	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
	Safe_AddRef(m_pGameInstance);

	if (FAILED(ImGui_Initialize()))	//! ImGui 초기화
		return E_FAIL;

	ImGuizmo_Initialize();			//! ImGui 즈모 초기화
	ImGuiFileDialog_Intialize();	//! Imgui 파일다이얼로그 초기화

	Ready_ProtoTagList();			//! 오브젝트 프로토태그 리스트 준

	//m_pNavigation = dynamic_cast<CNavigation*>(m_pGameInstance->Clone_Component(LEVEL_TOOL, TEXT("Prototype_Component_Navigation"), nullptr));

	

	return S_OK;
}

void CImgui_Manager::Tick(_float fTimeDelta)
{
	//! 레이 월드좌표 갱신
	UpdateRay();

	ImGui_MainTick();

	//#Imgui_MainTool_Start
	ImGui::Begin(u8"메인 툴");

	if (ImGui::BeginTabBar(u8""))
	{
		ImGui_MapToolTick();

		ImGui_ObjectToolTick();

		ImGui_NaviToolTick();


		ShowDialog(m_eToolID);

		ImGui::EndTabBar();
	}

	//#Imgui_MainTool_End
	ImGui::End();

}

void CImgui_Manager::Render()
{
	if (m_bReady)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		m_bReady = false;
	}

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

}

HRESULT CImgui_Manager::ImGui_Initialize()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	g_io = ImGui::GetIO(); (void)g_io;
	g_io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);

	if (!ImGui_ImplWin32_Init(g_hWnd))
		return E_FAIL;

	if (!ImGui_ImplDX11_Init(m_pDevice, m_pContext))
		return E_FAIL;

	ImGui::StyleColorsDark();
	g_io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\malgun.ttf", 18.0f, NULL, g_io.Fonts->GetGlyphRangesKorean());

	return S_OK;
}

void CImgui_Manager::ImGuizmo_Initialize()
{
	//! 기즈모 뷰,투영 멤버변수 메모리 할당
	m_arrView = new _float[16];
	m_arrProj = new _float[16];
	//! 기즈모 뷰,투영 멤버변수 메모리 할당 끝
}

void CImgui_Manager::ImGuiFileDialog_Intialize()
{
	m_pFileDialog = ImGuiFileDialog::Instance();

	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByFullName, "((Custom.+[.]h))", ImVec4(0.1f, 0.9f, 0.1f, 0.9f));  // use a regex
	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".cpp", ImVec4(1.0f, 1.0f, 0.0f, 0.9f));
	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".hpp", ImVec4(0.0f, 0.0f, 1.0f, 0.9f));
	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".md", ImVec4(1.0f, 0.0f, 1.0f, 0.9f));
	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".png", ImVec4(0.0f, 1.0f, 1.0f, 0.9f), ICON_IGFD_FILE_PIC);  // add an icon for the filter type
	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByExtention, ".gif", ImVec4(0.0f, 1.0f, 0.5f, 0.9f), "[GIF]");             // add an text for a filter type
	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, nullptr, ImVec4(0.5f, 1.0f, 0.9f, 0.9f), ICON_IGFD_FOLDER);     // for all dirs
	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile, "CMakeLists.txt", ImVec4(0.1f, 0.5f, 0.5f, 0.9f), ICON_IGFD_ADD);
	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByFullName, "doc", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_FILE_PIC);
	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile, nullptr, ImVec4(0.2f, 0.9f, 0.2f, 0.9f), ICON_IGFD_FILE);                              // for all link files
	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir | IGFD_FileStyleByTypeLink, nullptr, ImVec4(0.8f, 0.8f, 0.8f, 0.8f), ICON_IGFD_FOLDER);  // for all link dirs
	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile | IGFD_FileStyleByTypeLink, nullptr, ImVec4(0.8f, 0.8f, 0.8f, 0.8f), ICON_IGFD_FILE);   // for all link files
	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir | IGFD_FileStyleByContainedInFullName, ".git", ImVec4(0.9f, 0.2f, 0.0f, 0.9f), ICON_IGFD_BOOKMARK);
	ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile | IGFD_FileStyleByContainedInFullName, ".git", ImVec4(0.5f, 0.8f, 0.5f, 0.9f), ICON_IGFD_SAVE);

	ImGuiFileDialog::Instance()->AddBookmark("Bin", "../Bin/");
	ImGuiFileDialog::Instance()->AddBookmark("FBX", "C:/Users/PC/Desktop/ExportFBX/");

}

void CImgui_Manager::ImGui_MainTick()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuizmo::BeginFrame();
}

void CImgui_Manager::ImGui_MapToolTick()
{

	if (ImGui::BeginTabItem(u8"맵"))
	{
		m_eToolID = CImgui_Manager::TOOL_MAP;

		if (ImGui::Button(u8"저장하기 ")) { m_eDialogMode = CImgui_Manager::DIALOG_SAVE; OpenDialog(m_eToolID); } ImGui::SameLine(); if (ImGui::Button(u8"불러오기 ")) { m_eDialogMode = CImgui_Manager::DIALOG_LOAD; OpenDialog(m_eToolID); }

		ImGui::InputFloat(u8"X 사이즈", &m_fTileX);
		ImGui::InputFloat(u8"Z 사이즈", &m_fTileZ);

		m_tMapInfo.vPosition.x = m_fTileX;
		m_tMapInfo.vPosition.y = 1.f;
		m_tMapInfo.vPosition.z = m_fTileZ;

		if (ImGui::Button(u8"생성"))
		{
			if (nullptr != m_pField)
			{
				m_pField->Delete_Component(TEXT("Com_VIBuffer"));
			}


			if (FAILED(m_pGameInstance->Add_CloneObject(LEVEL_TOOL, TEXT("Layer_BackGround"), TEXT("Prototype_GameObject_Field"), &m_tMapInfo, reinterpret_cast<CGameObject**>(&m_pField))))
				return;

			//!if (nullptr != m_pDynamic_Terrain)
			//!{
			//!	m_pDynamic_Terrain->Delete_Component(TEXT("Com_VIBuffer"));
			//!}

			//!if (FAILED(m_pGameInstance->Add_CloneObject(LEVEL_TOOL, TEXT("Layer_BackGround"), TEXT("Prototype_GameObject_Dynamic_Terrain"), &m_tMapInfo, reinterpret_cast<CGameObject**>(&m_pDynamic_Terrain))))
			//!	return;
			
		}

		if (nullptr != m_pField)
		{
			ImGui::Checkbox(u8"픽킹모드", &m_bMapToolPickMode);

			if (m_bMapToolPickMode)
			{
				ImGui::Text(u8"마우스 X : %f", m_pField->GetMousePos().x);
				ImGui::Text(u8"마우스 Y : %f", m_pField->GetMousePos().y);
				ImGui::Text(u8"마우스 Z : %f", m_pField->GetMousePos().z);

				if (ImGui::InputInt(u8"브러시 범위", &m_iBrushRange)) { m_pField->SetRadious(m_iBrushRange); }
				if (ImGui::InputInt(u8"브러시 힘", &m_iBrushPower)) { m_pField->SetPower(m_iBrushPower); }

				ImGui::NewLine(); //! 브러시 모드 라디오

				static int BrushIndex = 0;
				const char* BrushModeName[3] = { u8"다운", u8"업", u8"프레싱" };

				for (_uint i = 0; i < IM_ARRAYSIZE(BrushModeName); ++i)
				{
					if (i > 0) { ImGui::SameLine(); }
					ImGui::RadioButton(BrushModeName[i], &BrushIndex, i);
				}

				m_eBrushMode = BRUSHMODE(BrushIndex);

				ImGui::NewLine(); //! 지형 픽킹 모드 라디오

				const char* TileModeName[4] = { u8"뾰족", u8"둥글게", u8"사각", u8"필터" };

				for (_uint i = 0; i < IM_ARRAYSIZE(TileModeName); ++i)
				{
					if (i > 0) { ImGui::SameLine(); }
					ImGui::RadioButton(TileModeName[i], &m_iTileMode, i);
				}

				if (ImGui_MouseInCheck())
					PickingTerrain(m_eBrushMode);

			}
		}

// 		if (nullptr != m_pDynamic_Terrain)
// 		{
// 			ImGui::Checkbox(u8"픽킹모드", &m_bMapToolPickMode);
// 
// 			if (m_bMapToolPickMode)
// 			{
// 				ImGui::Text(u8"마우스 X : %f", m_pDynamic_Terrain->GetMousePos().x);
// 				ImGui::Text(u8"마우스 Y : %f", m_pDynamic_Terrain->GetMousePos().y);
// 				ImGui::Text(u8"마우스 Z : %f", m_pDynamic_Terrain->GetMousePos().z);
// 
// 				if (ImGui::InputInt(u8"브러시 범위", &m_iBrushRange)) { m_pDynamic_Terrain->SetRadious(m_iBrushRange); }
// 				if (ImGui::InputInt(u8"브러시 힘", &m_iBrushPower)) { m_pDynamic_Terrain->SetPower(m_iBrushPower); }
// 
// 				ImGui::NewLine(); //! 브러시 모드 라디오
// 
// 				static int BrushIndex = 0;
// 				const char* BrushModeName[3] = { u8"다운", u8"업", u8"프레싱" };
// 
// 				for (_uint i = 0; i < IM_ARRAYSIZE(BrushModeName); ++i)
// 				{
// 					if (i > 0) { ImGui::SameLine(); }
// 					ImGui::RadioButton(BrushModeName[i], &BrushIndex, i);
// 				}
// 
// 				m_eBrushMode = BRUSHMODE(BrushIndex);
// 
// 				ImGui::NewLine(); //! 지형 픽킹 모드 라디오
// 
// 				const char* TileModeName[4] = { u8"뾰족", u8"둥글게", u8"사각", u8"필터" };
// 
// 				for (_uint i = 0; i < IM_ARRAYSIZE(TileModeName); ++i)
// 				{
// 					if (i > 0) { ImGui::SameLine(); }
// 					ImGui::RadioButton(TileModeName[i], &m_iTileMode, i);
// 				}
// 
// 				if (ImGui_MouseInCheck())
// 					PickingTerrain(m_eBrushMode);
// 
// 			}
// 		}

		ImGui::EndTabItem();
	}
}

void CImgui_Manager::ImGui_ObjectToolTick()
{

	if (ImGui::BeginTabItem(u8"오브젝트"))
	{
		m_eToolID = CImgui_Manager::TOOL_OBJECT;

		static int iObjectToolMode = 0;

		ImGui::RadioButton(u8"오브젝트", &iObjectToolMode, 0); ImGui::SameLine(); ImGui::RadioButton(u8"바이너리", &iObjectToolMode, 1);

		if (iObjectToolMode == 0)
		{

			ObjectModeTick();
		}

		else if (iObjectToolMode == 1)
		{
			BinaryModeTick();
		}

		ImGui::EndTabItem();
	}
}


_bool CImgui_Manager::ImGui_MouseInCheck()
{
	POINT tMouse;

	GetCursorPos(&tMouse);
	ScreenToClient(m_pGameInstance->Get_GraphicDesc().hWnd, &tMouse);


	ImVec2 windowPos = ImGui::GetWindowPos(); //왼쪽상단모서리점
	ImVec2 windowSize = ImGui::GetWindowSize();

	if (tMouse.x >= windowPos.x && tMouse.x <= windowPos.x + windowSize.x &&
		tMouse.y >= windowPos.y && tMouse.y <= windowPos.y + windowSize.y)
	{
		return false; //ImGui 영역 내
	}

	return true;
}

void CImgui_Manager::UpdateRay()
{
	_uint WinCX = g_iWinSizeX;
	_uint WinCY = g_iWinSizeY;

	m_tWorldRay = m_pGameInstance->Get_Ray(WinCX, WinCY);
}

void CImgui_Manager::Set_Guizmo()
{
	/*==== Set ImGuizmo ====*/
	ImGuizmo::SetOrthographic(false);
	ImGuiIO& io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);


	if (ImGui::IsKeyPressed(ImGuiKey_T))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(ImGuiKey_R))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(ImGuiKey_E))
		mCurrentGizmoOperation = ImGuizmo::SCALE;

	if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
		mCurrentGizmoOperation = ImGuizmo::SCALE;

	_float* arrView = m_arrView;
	_float* arrProj = m_arrProj;

	_float4x4 matWorld = m_PickingObject->Get_Transform()->Get_WorldFloat4x4();
	_float arrWorld[] = { matWorld._11,matWorld._12,matWorld._13,matWorld._14,
						  matWorld._21,matWorld._22,matWorld._23,matWorld._24,
						  matWorld._31,matWorld._32,matWorld._33,matWorld._34,
						  matWorld._41,matWorld._42,matWorld._43,matWorld._44 };

	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
	ImGuizmo::DecomposeMatrixToComponents(arrWorld, matrixTranslation, matrixRotation, matrixScale);
	ImGui::DragFloat3("Tr", matrixTranslation);
	ImGui::DragFloat3("Rt", matrixRotation);
	ImGui::DragFloat3("Sc", matrixScale);
	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, arrWorld);


	ImGui::Checkbox("UseSnap", &useSnap);
	ImGui::SameLine();

	switch (mCurrentGizmoOperation)
	{
	case ImGuizmo::TRANSLATE:
		ImGui::DragFloat3("Snap", &snap[0]);
		break;
	case ImGuizmo::ROTATE:
		ImGui::DragFloat3("Angle Snap", &snap[0]);
		break;
	case ImGuizmo::SCALE:
		ImGui::DragFloat3("Scale Snap", &snap[0]);
		break;
	}

	ImGuizmo::Manipulate(arrView, arrProj, mCurrentGizmoOperation, mCurrentGizmoMode, arrWorld, NULL, useSnap ? &snap[0] : NULL);

	_float4x4 matW = { arrWorld[0],arrWorld[1],arrWorld[2],arrWorld[3],
					arrWorld[4],arrWorld[5],arrWorld[6],arrWorld[7],
					arrWorld[8],arrWorld[9],arrWorld[10],arrWorld[11],
					arrWorld[12],arrWorld[13],arrWorld[14],arrWorld[15] };

	m_PickingObject->Get_Transform()->Set_WorldFloat4x4(matW);


	if (ImGuizmo::IsOver())
	{
		int a = 0;
	}
}

void CImgui_Manager::Set_GuizmoCamView()
{
	_float4x4 matCamView = m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_VIEW);
	_float	  arrView[] = { matCamView._11,matCamView._12,matCamView._13,matCamView._14,
						  matCamView._21,matCamView._22,matCamView._23,matCamView._24,
						  matCamView._31,matCamView._32,matCamView._33,matCamView._34,
						  matCamView._41,matCamView._42,matCamView._43,matCamView._44 };

	memcpy(m_arrView, &arrView, sizeof(arrView));
}

void CImgui_Manager::Set_GuizmoCamProj()
{
	_float4x4 matCamProj = m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_PROJ);
	_float	  arrProj[] = { matCamProj._11,matCamProj._12,matCamProj._13,matCamProj._14,
						  matCamProj._21,matCamProj._22,matCamProj._23,matCamProj._24,
						  matCamProj._31,matCamProj._32,matCamProj._33,matCamProj._34,
						  matCamProj._41,matCamProj._42,matCamProj._43,matCamProj._44 };
	memcpy(m_arrProj, &arrProj, sizeof(arrProj));
}

void CImgui_Manager::OpenDialog(TOOLID eToolID)
{
	string strKey, strTitle, strPath;
	const _char* szFilters = "Binary (*.dat, *.vfx,){.dat,.vfx},Instance (*.dat){.dat},Json (*.json){.json},All files{.*}";

	string strAdd;

	if (m_eDialogMode == CImgui_Manager::DIALOG_SAVE)
		strAdd = u8" 저장";
	else if (m_eDialogMode == CImgui_Manager::DIALOG_LOAD)
		strAdd = u8" 불러오기";

	switch (eToolID)
	{
	case Client::CImgui_Manager::TOOL_MAP:
		strKey = "MapToolDialog";
		strTitle = u8"맵 " + strAdd;
		strPath = "../Bin/DataFiles/Map/";

		break;
	case Client::CImgui_Manager::TOOL_OBJECT:
		strKey = "ObjectToolDialog";
		strTitle = u8"오브젝트 " + strAdd;
		strPath = "../Bin/DataFiles/";
		break;

	case Client::CImgui_Manager::TOOL_NAVI:
		strKey = "NaviToolDialog";
		strTitle = u8"내비게이션 " + strAdd;
		strPath = "../Bin/DataFiles/";
		break;

	case Client::CImgui_Manager::TOOL_CAMERA:
		break;
	case Client::CImgui_Manager::TOOL_EFFECT:
		break;
	}


	m_pFileDialog->OpenDialog(strKey, strTitle, szFilters, strPath, 1, nullptr, ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ConfirmOverwrite);

}

void CImgui_Manager::ShowDialog(TOOLID eToolID)
{
	string DialogKey;

	if (eToolID == CImgui_Manager::TOOL_MAP)
	{
		DialogKey = "MapToolDialog";
	}
	else if (eToolID == CImgui_Manager::TOOL_OBJECT)
	{
		DialogKey = "ObjectToolDialog";
	}

	else if (eToolID == CImgui_Manager::TOOL_NAVI)
	{
		DialogKey = "NaviToolDialog";
	}

	if (m_pFileDialog->Display(DialogKey))
	{
		if (m_pFileDialog->IsOk())
		{
			string filePathName = m_pFileDialog->GetFilePathName();
			string filePath = m_pFileDialog->GetCurrentPath();
			string fileName = m_pFileDialog->GetCurrentFileName();

			string userDatas;
			if (m_pFileDialog->GetUserDatas())
				userDatas = std::string((const char*)m_pFileDialog->GetUserDatas());
			auto selection = m_pFileDialog->GetSelection(); // multiselection

			switch (m_eDialogMode)
			{
			case Client::CImgui_Manager::DIALOG_SAVE:
			{
				if (m_eToolID == CImgui_Manager::TOOL_MAP)
					SaveMap(filePathName, fileName);

				else if (m_eToolID == CImgui_Manager::TOOL_OBJECT)
					SaveObject(filePath, fileName);

				else if(m_eToolID == CImgui_Manager::TOOL_NAVI)
					SaveNavi(filePathName);

				break;
			}


			case Client::CImgui_Manager::DIALOG_LOAD:
			{
				if (m_eToolID == CImgui_Manager::TOOL_MAP)
					LoadMap(filePathName, fileName);

				else if (m_eToolID == CImgui_Manager::TOOL_OBJECT)
				{
					if (fileName.find("NonAnim") != string::npos)
					{
						LoadNonAnimObject(filePath, fileName);
					}
					else if (fileName.find("Anim") != string::npos)
					{
						LoadAnimObject(filePath, fileName);
					}
				}

				else if (m_eToolID == CImgui_Manager::TOOL_NAVI)
						LoadNavi(filePathName);

				break;
			}
			}
		}
		m_pFileDialog->Close();
	}
}

void CImgui_Manager::OpenDialogBinaryModel()
{
	if (m_eModelType == MODEL_TYPE::TYPE_END)
		return;

	string strKey, strTitle, strPath;
	const _char* szFilters = "Target (*.fbx){.fbx},All files{.*}";

	string strAdd;

	if (m_eModelType == MODEL_TYPE::TYPE_ANIM)
		strAdd = u8" (애니메이션)";
	else if (m_eModelType == MODEL_TYPE::TYPE_NONANIM)
		strAdd = u8" (논애니메이션)";

	strKey = "BinarySave";
	strTitle = u8"모델 바이너리 저장";
	strTitle += strAdd;

	strPath = "../Bin/Resources/Models/";

	m_pFileDialog->OpenDialog(strKey, strTitle, szFilters, strPath, 1, nullptr, ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_ConfirmOverwrite);
}

void CImgui_Manager::ShowDialogBianryModel(MODEL_TYPE eModelType)
{
	string DialogKey = "BinarySave";

	if (m_pFileDialog->Display(DialogKey))
	{
		if (m_pFileDialog->IsOk())
		{
			string filePathName = m_pFileDialog->GetFilePathName();
			string filePath = m_pFileDialog->GetCurrentPath();
			string fileName = m_pFileDialog->GetCurrentFileName();

			string userDatas;
			if (m_pFileDialog->GetUserDatas())
				userDatas = std::string((const char*)m_pFileDialog->GetUserDatas());
			auto selection = m_pFileDialog->GetSelection(); // multiselection

			BinaryConvert(fileName, filePathName, eModelType);
		}
		m_pFileDialog->Close();
	}
}

void CImgui_Manager::PickingTerrain(BRUSHMODE eBrushMode)
{
	if (eBrushMode == CImgui_Manager::BRUSH_END)
		return;

	switch (eBrushMode)
	{
	case Client::CImgui_Manager::BRUSH_DOWN:
	{
		if (m_pGameInstance->Mouse_Down(DIM_LB))
		{
			m_pField->Picking_Terrain((CField::EDIT_MODE)m_iTileMode);
		}

		break;
	}

	case Client::CImgui_Manager::BRUSH_UP:
	{
		if (m_pGameInstance->Mouse_Up(DIM_LB))
		{
			m_pField->Picking_Terrain((CField::EDIT_MODE)m_iTileMode);
		}

		break;
	}

	case Client::CImgui_Manager::BRUSH_PRESSING:
	{
		if (m_pGameInstance->Mouse_Pressing(DIM_LB))
		{
			m_pField->Picking_Terrain((CField::EDIT_MODE)m_iTileMode);
		}

		break;
	}
	}
}

void CImgui_Manager::SaveMap(string strFilePath, string strFileName)
{



	json OutJson;
	//size_t pos = strObjectTag.rfind(L"_");
	//if (pos != wstring::npos)
	//{
	//	return strObjectTag.substr(pos + 1);
	//}
	size_t index = 0, current = 0;

	current = strFileName.find(".");
	string sub;
	if (current != string::npos)
	{
		sub = strFileName.substr(index, current - index);
	}

	m_pField->Write_Json(OutJson);

	if (FAILED(CJson_Utility::Save_Json(strFilePath.c_str(), OutJson)))
	{
		MSG_BOX("맵 세이브 실패");
	}
	else
	{
		MSG_BOX("저장 성공");
	}
}

void CImgui_Manager::LoadMap(string strFilePath, string strFileName)
{
	json	InJson;

	size_t index = 0, current = 0;

	current = strFileName.find(".");
	string sub;
	if (current != string::npos)
	{
		sub = strFileName.substr(index, current - index);
	}

	CJson_Utility::Load_Json(strFilePath.c_str(), InJson);

	if (m_pDynamic_Terrain)
		m_pDynamic_Terrain->Load_FromJson(InJson, ConvertStrToWstr(sub));

	else
	{
		//Out_Json.emplace("SizeX", m_tDynamicInfo.fX);
		//Out_Json.emplace("SizeY", m_tDynamicInfo.fY);
		//Out_Json.emplace("SizeZ", m_tDynamicInfo.fZ);

		CDynamic_Terrain::DINFO tInfo;
		tInfo.fX = InJson["SizeX"];
		tInfo.fY = InJson["SizeY"];
		tInfo.fZ = InJson["SizeZ"];

		m_pGameInstance->Add_CloneObject(LEVEL_TOOL, TEXT("Layer_BackGround"), TEXT("Prototype_GameObject_Dynamic_Terrain"), &tInfo);
	}
}

HRESULT CImgui_Manager::Add_PrototypeTag(const wstring& strPrototypeTag, _bool bModelType)
{
	string pTag = ConvertWstrToStr(strPrototypeTag);

	if (true == bModelType)
		m_vecAnimObjectTags.push_back(pTag);
	else
		m_vecNonAnimObjectTags.push_back(pTag);


	return S_OK;
}

HRESULT CImgui_Manager::Ready_ProtoTagList()
{
	vector<wstring> LayerTags = m_pGameInstance->Get_LayerTags();

	for (auto& strLayerTag : LayerTags)
	{
		m_vecLayerTags.push_back(ConvertWstrToStr(strLayerTag));
	}

	
	map<const wstring, _bool> ObjectTags = m_pGameInstance->Get_ObjectTags();

	for (auto& wstrTag : ObjectTags)
	{
		Add_PrototypeTag(wstrTag.first, wstrTag.second);
	}

	vector<wstring> ModelTags = m_pGameInstance->Get_ModelTags();

	for (auto& strModelTag : ModelTags)
	{
		m_vecModelTags.push_back(ConvertWstrToStr(strModelTag));
	}

	return S_OK;
}

void CImgui_Manager::BinaryModeTick()
{
	ImGui::NewLine();

	static bool bModelType = true;

	ImGui::Text(u8"체크안할시 논애님");
	ImGui::Checkbox(u8"모델타입", &bModelType);

	m_eModelType = (MODEL_TYPE)bModelType;

	ImGui::NewLine();

	if (ImGui::Button(u8"바이너리화"))
	{
		OpenDialogBinaryModel();
	}

	ShowDialogBianryModel(m_eModelType);
}

void CImgui_Manager::ObjectModeTick()
{

	if (ImGui::Button(u8"저장하기")) { m_eDialogMode = CImgui_Manager::DIALOG_SAVE; OpenDialog(m_eToolID); } ImGui::SameLine(); if (ImGui::Button(u8"불러오기")) { m_eDialogMode = CImgui_Manager::DIALOG_LOAD; OpenDialog(m_eToolID); }

	if (nullptr != m_pField)
	{
		ImGui::Text(u8"마우스 X : %f", m_pField->GetMousePos().x);
		ImGui::Text(u8"마우스 Y : %f", m_pField->GetMousePos().y);
		ImGui::Text(u8"마우스 Z : %f", m_pField->GetMousePos().z);


		ImGui::Checkbox(u8"픽킹모드", &m_bObjectToolPickMode);
		
		

		if (true == m_bObjectToolPickMode)
		{
			ImGui::RadioButton(u8"Create", &m_iObjectMode, 0); ImGui::SameLine(); ImGui::RadioButton(u8"Select", &m_iObjectMode, 1); ImGui::SameLine(); ImGui::RadioButton(u8"Delete", &m_iObjectMode, 2);

			ImGui::RadioButton(u8"AnimModel", &m_iModelType, 0); ImGui::SameLine(); ImGui::RadioButton(u8"NonAnimModel", &m_iModelType, 1);

			if (0 == m_iObjectMode)
			{
				CreateObjectFunction();
			}

			else if (1 == m_iObjectMode)
			{
				SelectObjectFunction();
			}

			else if (2 == m_iObjectMode)
			{
				DeleteObjectFunction();
			}
		}
	}
}

void CImgui_Manager::CreateObjectFunction()
{

	_int iObjectTagSize = 0, iLayerTagSize = 0;

	
	if (m_bOpenLayerTags == true)
	{
		iLayerTagSize = m_vecLayerTags.size();

		if (ImGui::BeginListBox(u8"레이어 태그 리스트"))
		{
			for (_uint i = 0; i < iLayerTagSize; ++i)
			{
				const _bool isSelected = (m_iSelectLayerTagIndex == i);

				if (ImGui::Selectable(m_vecLayerTags[i].c_str(), isSelected))
				{
					m_iSelectLayerTagIndex = i;

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndListBox();
		}
	}

	ImGui::Checkbox(u8"모델픽킹", &m_bModelPicking);

	if(m_iSelectLayerTagIndex != -1)
		m_bOpenLayerTags = false;

	if (m_bOpenLayerTags == false) //! 레이어 태그를 선택 했다면.
	{

		if (0 == m_iModelType)
		{
			iObjectTagSize = m_vecAnimObjectTags.size();


			if (ImGui::BeginListBox(u8"애니메이션 모델 태그 리스트"))
			{
				for (_uint i = 0; i < iObjectTagSize; ++i)
				{
					const _bool isSelected = (m_iSelectTagIndex == i);

					if (ImGui::Selectable(m_vecAnimObjectTags[i].c_str(), isSelected))
					{
						m_iSelectTagIndex = i;

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndListBox();
			}
		}
		else
		{
			//iObjectTagSize = m_vecNonAnimObjectTags.size();
			iObjectTagSize = m_vecModelTags.size();


			if (ImGui::BeginListBox(u8"환경 모델 태그 리스트"))
			{
				for (_uint i = 0; i < iObjectTagSize; ++i)
				{
					const _bool isSelected = (m_iSelectTagIndex == i);
					
					if (ImGui::Selectable(m_vecModelTags[i].c_str(), isSelected))
					{
						m_iSelectTagIndex = i;
					
						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndListBox();
			}
		}

		if (ImGui::Button(u8"취소"))
		{
			m_bOpenLayerTags = true;
			m_iSelectLayerTagIndex = -1;
		}
	}


	if (m_pGameInstance->Mouse_Down(DIM_LB))
	{
		if (nullptr != m_pField)
			m_fPickingPos = m_pField->GetMousePos(); //! 마우스 클릭한 지점의 월드 좌표 받기
		else
		{
			MSG_BOX("필드부터 생성해");
			return;
		}


		if (true == m_bModelPicking)
		{
			_int iNonAnimObjectSize = m_vecNonAnimObjects.size();

			CGameObject* pHighestObject = nullptr;

			_int	iIndex = 0;
			_float fHighestYValue = -FLT_MAX;
			_float3 vHighestPickesPos = {};
			_bool	bIsPicking = false;

			for (_int i = 0; i < iNonAnimObjectSize; ++i)
			{
				_float3 vPickedPos = {};

				if (m_vecNonAnimObjects[i]->Picking(m_fPickingPos, dynamic_cast<CModel*>(m_vecNonAnimObjects[i]->Find_Component(TEXT("Com_Model"))), &vPickedPos) && true == ImGui_MouseInCheck())
				{
					
					_float3 vDestPos;
					XMStoreFloat3(&vDestPos, m_vecNonAnimObjects[i]->Get_Transform()->Get_State(CTransform::STATE_POSITION));

					if (vDestPos.y > fHighestYValue)
					{
						fHighestYValue = vDestPos.y;
						iIndex = i;
						vHighestPickesPos = vPickedPos;
						bIsPicking = true;
					}
				}
			}

			if (true == bIsPicking)
			{
				CGameObject* pGameObject = nullptr;

				wstring wstr;
				string SliceTag = "";
				string IndexTag;

				if (0 == m_iModelType)
				{
					CGameObject::GAMEOBJECT_DESC pDesc;

					pDesc.iLevelIndex = LEVEL_TOOL;

					wstr = ConvertStrToWstr(m_vecAnimObjectTags[m_iSelectTagIndex]);

					if (FAILED(m_pGameInstance->Add_CloneObject(LEVEL_TOOL, ConvertStrToWstr(m_vecLayerTags[m_iSelectLayerTagIndex]), wstr, &pDesc, reinterpret_cast<CGameObject**>(&pGameObject))))
						return;

					SliceTag = ConvertWstrToStr(wstr);
					IndexTag = "@" + to_string(m_vecAnimObjects.size() + 1);
					SliceTag = SliceTag + IndexTag;

					m_vecCreateAnimObjectTags.push_back(SliceTag);
					m_vecCreateAnimObjectLayerTag.push_back(m_vecLayerTags[m_iSelectLayerTagIndex]);
					m_vecAnimObjects.push_back(pGameObject);
				}

				else
				{
					CEnvironment_Object::ENVIRONMENT_DESC Desc;
					Desc.iLevelIndex = LEVEL_TOOL;
					Desc.strModelTag = ConvertStrToWstr(m_vecModelTags[m_iSelectTagIndex]);

					wstr = TEXT("Prototype_GameObject_Environment");

					if (FAILED(m_pGameInstance->Add_CloneObject(LEVEL_TOOL, ConvertStrToWstr(m_vecLayerTags[m_iSelectLayerTagIndex]), wstr, &Desc, reinterpret_cast<CGameObject**>(&pGameObject))))
						return;

					SliceTag = m_vecModelTags[m_iSelectTagIndex];
					IndexTag = "@" + to_string(m_vecNonAnimObjects.size() + 1);
					SliceTag = SliceTag + IndexTag;

					m_vecCreateNonAnimObjectTags.push_back(SliceTag);
					m_vecCreateNonAnimObjectLayerTag.push_back(m_vecLayerTags[m_iSelectLayerTagIndex]);
					m_vecNonAnimObjects.push_back(pGameObject);
				}

				pGameObject->Get_Transform()->Set_State(CTransform::STATE_POSITION, XMVectorSet(vHighestPickesPos.x, vHighestPickesPos.y, vHighestPickesPos.z, 1.f));
			}
			
		}
		else if (m_pField->MouseOnTerrain() && ImGui_MouseInCheck())
		{
			CGameObject* pGameObject = nullptr;

			wstring wstr;
			string SliceTag = "";
			string IndexTag = "";

			if (0 == m_iModelType)
			{
				CGameObject::GAMEOBJECT_DESC pDesc;
				pDesc.iLevelIndex = LEVEL_TOOL;

				wstr = ConvertStrToWstr(m_vecAnimObjectTags[m_iSelectTagIndex]);

				if (FAILED(m_pGameInstance->Add_CloneObject(LEVEL_TOOL, ConvertStrToWstr(m_vecLayerTags[m_iSelectLayerTagIndex]), wstr, &pDesc, reinterpret_cast<CGameObject**>(&pGameObject))))
					return;

				SliceTag = ConvertWstrToStr(wstr);
				IndexTag = "@" + to_string(m_vecAnimObjects.size() + 1);
				SliceTag = SliceTag + IndexTag;

				m_vecCreateAnimObjectTags.push_back(SliceTag);
				m_vecCreateAnimObjectLayerTag.push_back(m_vecLayerTags[m_iSelectLayerTagIndex]);
				m_vecAnimObjects.push_back(pGameObject);
			}
			else
			{
				CEnvironment_Object::ENVIRONMENT_DESC Desc;
				Desc.iLevelIndex = LEVEL_TOOL;
				Desc.strModelTag = ConvertStrToWstr(m_vecModelTags[m_iSelectTagIndex]);

				wstr = TEXT("Prototype_GameObject_Environment");

				if (FAILED(m_pGameInstance->Add_CloneObject(LEVEL_TOOL, ConvertStrToWstr(m_vecLayerTags[m_iSelectLayerTagIndex]), wstr, &Desc, reinterpret_cast<CGameObject**>(&pGameObject))))
					return;

				SliceTag = m_vecModelTags[m_iSelectTagIndex];
				IndexTag = "@" + to_string(m_vecNonAnimObjects.size() + 1);
				SliceTag = SliceTag + IndexTag;

				m_vecCreateNonAnimObjectTags.push_back(SliceTag);
				m_vecCreateNonAnimObjectLayerTag.push_back(m_vecLayerTags[m_iSelectLayerTagIndex]);
				m_vecNonAnimObjects.push_back(pGameObject);
			}

			pGameObject->Get_Transform()->Set_State(CTransform::STATE_POSITION, XMVectorSet(m_fPickingPos.x, m_fPickingPos.y, m_fPickingPos.z, 1.f));
		}

	}

}

void CImgui_Manager::SelectObjectFunction()
{
	Set_GuizmoCamView(); //! 기즈모 뷰 투영 셋팅해주기.
	Set_GuizmoCamProj();	//! 기즈모 뷰 투영 셋팅해주기.
	

	static bool bPartDebug = false;

	ImGui::Text(u8"현재 EN00파츠");
	ImGui::Checkbox(u8"테스트", &bPartDebug);
	

	if (nullptr != m_PickingObject)
	{
		Set_Guizmo();

		if (true == bPartDebug)
		{
			static _float TestInput[3];
			static _float TestCenter[3];

			m_PartObject = dynamic_cast<CMonster_EN00*>(m_PickingObject)->Find_PartObject(TEXT("Part_Weapon"));


			ImGui::InputFloat3(u8"테스트Extents", TestInput);
			ImGui::InputFloat3(u8"테스트Center", TestCenter);

			CBoundParent* pBound = dynamic_cast<CMonsterPart_EN00_Weapon*>(m_PartObject)->Get_Collider()->Get_Bounding();

			CBoundingBox_AABB* pAABBBox = dynamic_cast<CBoundingBox_AABB*>(pBound);

			

			_float3 vTest = { TestInput[0],TestInput[1], TestInput[2] };
			_float3 vTestCenter = { TestCenter[0], TestCenter[1], TestCenter[2] };

			BoundingBox* pBox = pAABBBox->Get_Bounding();

			pBox->Extents = vTest;
			pBox->Center = vTestCenter;
			

		}
	}

	_int iObjectListSize;

	if (0 == m_iModelType)
		iObjectListSize = m_vecCreateAnimObjectTags.size();
	else 
		iObjectListSize = m_vecCreateNonAnimObjectTags.size();


	if (0 == m_iModelType)
	{
		if (ImGui::BeginListBox(u8""))
		{
			for (_int i = 0; i < iObjectListSize; ++i)
			{
				const _bool isSelected = (m_iPickingObjectIndex == i);

				if (ImGui::Selectable(m_vecCreateAnimObjectTags[i].c_str(), isSelected))
				{
					m_PickingObject = m_vecAnimObjects[i];
					m_iPickingObjectIndex = i;

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndListBox();
		}
	}
	else
	{
		if (ImGui::BeginListBox(u8""))
		{
			for (_int i = 0; i < iObjectListSize; ++i)
			{
				const _bool isSelected = (m_iPickingObjectIndex == i);

				if (ImGui::Selectable(m_vecCreateNonAnimObjectTags[i].c_str(), isSelected))
				{
					m_PickingObject = m_vecNonAnimObjects[i];
					m_iPickingObjectIndex = i;

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndListBox();
		}
	}
	

	ImGui::NewLine();

	ImGui::Checkbox(u8"픽킹모드", &m_bObjectToolPickMode);

	if (nullptr != m_pField && false == m_bObjectToolPickMode)
		return;

	m_fPickingPos = m_pField->GetMousePos();

	if (nullptr != m_PickingObject)
	{
		if(m_pGameInstance->Mouse_Down(DIM_RB))
			m_PickingObject = nullptr;
	}
	

	if (m_pGameInstance->Mouse_Down(DIM_LB) && nullptr == m_PickingObject)
	{
		_int iObjectSize;

		if (0 == m_iModelType)
		{
			iObjectSize = m_vecAnimObjects.size();
		}
		else 
			iObjectSize = m_vecNonAnimObjects.size();
		
		

		for (_uint i = 0; i < iObjectSize; ++i)
		{
			_float3 vPickedPos = {};
			
			if (0 == m_iModelType)
			{
				if (m_vecAnimObjects[i]->Picking(m_fPickingPos, dynamic_cast<CModel*>(m_vecAnimObjects[i]->Find_Component(TEXT("Com_Model"))), &vPickedPos))
				{
					m_PickingObject = m_vecAnimObjects[i];
					m_iPickingObjectIndex = i;
				}
			}
			else
			{
				if (m_vecNonAnimObjects[i]->Picking(m_fPickingPos, dynamic_cast<CModel*>(m_vecNonAnimObjects[i]->Find_Component(TEXT("Com_Model"))), &vPickedPos))
				{
					m_PickingObject = m_vecNonAnimObjects[i];
					m_iPickingObjectIndex = i;
				}
			}
			
		}
	}
	

	
}

void CImgui_Manager::DeleteObjectFunction()
{
	ImGui::NewLine();
	

	ImGui::InputInt(u8"삭제할 오브젝트의 인덱스 : ", &m_iPickingObjectIndex);

	_int iObjectListSize;

	if (0 == m_iModelType)
	{
		iObjectListSize = m_vecAnimObjects.size();

		if (ImGui::BeginListBox(u8""))
		{
			for (_int i = 0; i < iObjectListSize; ++i)
			{
				const _bool isSelected = (m_iPickingObjectIndex == i);

				if (ImGui::Selectable(m_vecCreateAnimObjectTags[i].c_str(), isSelected))
				{
					m_PickingObject = m_vecAnimObjects[i];
					m_iPickingObjectIndex = i;

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndListBox();
		}
	}
	else
	{
		iObjectListSize = m_vecNonAnimObjects.size();

		if (ImGui::BeginListBox(u8""))
		{
			for (_int i = 0; i < iObjectListSize; ++i)
			{
				const _bool isSelected = (m_iPickingObjectIndex == i);

				if (ImGui::Selectable(m_vecCreateNonAnimObjectTags[i].c_str(), isSelected))
				{
					m_PickingObject = m_vecNonAnimObjects[i];
					m_iPickingObjectIndex = i;

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndListBox();
		}
	}

	
		if (ImGui::Button(u8"삭제") && m_iPickingObjectIndex >= 0)
		{
			// 범위 체크 필요
			if (0 == m_iModelType)
			{
				if (m_iPickingObjectIndex < static_cast<int>(m_vecCreateAnimObjectTags.size()) && m_iPickingObjectIndex < static_cast<int>(m_vecAnimObjects.size()))
				{
					// 해당 인덱스의 요소를 동시에 삭제
					m_vecCreateAnimObjectTags.erase(m_vecCreateAnimObjectTags.begin() + m_iPickingObjectIndex);
					m_pGameInstance->Erase_CloneObject(LEVEL_TOOL, ConvertStrToWstr(m_vecCreateAnimObjectLayerTag[m_iPickingObjectIndex]), m_vecAnimObjects[m_iPickingObjectIndex]);
					m_vecCreateAnimObjectLayerTag.erase(m_vecCreateAnimObjectLayerTag.begin() + m_iPickingObjectIndex);
					m_vecAnimObjects.erase(m_vecAnimObjects.begin() + m_iPickingObjectIndex);
				}
				else
				{
					// 인덱스가 범위를 벗어날 경우 처리
					ImGui::Text(u8"인덱스가 범위를 벗어납니다.");
				}
			}
			else
			{
				if (m_iPickingObjectIndex < static_cast<int>(m_vecCreateNonAnimObjectTags.size()) && m_iPickingObjectIndex < static_cast<int>(m_vecNonAnimObjects.size()))
				{
					// 해당 인덱스의 요소를 동시에 삭제
					m_vecCreateNonAnimObjectTags.erase(m_vecCreateNonAnimObjectTags.begin() + m_iPickingObjectIndex);
					m_pGameInstance->Erase_CloneObject(LEVEL_TOOL, ConvertStrToWstr(m_vecCreateNonAnimObjectLayerTag[m_iPickingObjectIndex]), m_vecNonAnimObjects[m_iPickingObjectIndex]);
					m_vecCreateNonAnimObjectLayerTag.erase(m_vecCreateNonAnimObjectLayerTag.begin() + m_iPickingObjectIndex);
					m_vecNonAnimObjects.erase(m_vecNonAnimObjects.begin() + m_iPickingObjectIndex);
				}
				else
				{
					// 인덱스가 범위를 벗어날 경우 처리
					ImGui::Text(u8"인덱스가 범위를 벗어납니다.");
				}
			}
			
			m_PickingObject = nullptr;
			m_iPickingObjectIndex = 0;
		}
		else
		{
			// 음수일 경우 처리
			ImGui::Text(u8"올바른 인덱스 값을 입력하세요.");
		}

	//! 오브젝트 태그는 지울 필요가 없다.
	
	//! CreateAnimObjectTags를 지워야한다.
	//! m_vecCreateAnimObjectTag와 m_vecAnimObject의 인덱스는 동일하다.
	

}

void CImgui_Manager::SaveObject(string strPath, string strFileName)
{
	
	_int iAnimObjectSize = m_vecAnimObjects.size();
	_int iNonAnimObjectSize = m_vecNonAnimObjects.size();
	json AnimationJson;
	json NonAnimationJson;
	
	string strNoExtFileName = filesystem::path(strFileName).stem().string();

	string strAnimPath = strPath + "/" + strNoExtFileName + "_Anim.json";
	string strNonAnimPATH = strPath + "/" + strNoExtFileName + "_NonAnim.json";


	for (auto& tag : m_vecCreateAnimObjectTags) {
		// 문자열에서 '@' 문자 이후의 부분을 지움
		size_t atIndex = tag.find('@');
		if (atIndex != std::string::npos) {
			tag.erase(atIndex); // '@' 이후의 문자열을 모두 제거
		}

		// 결과 출력
		std::cout << tag << std::endl;
	}

	for (_int i = 0; i < iAnimObjectSize; i++)
	{
		AnimationJson[i].emplace("Index", i);
		AnimationJson[i].emplace("LayerTag", m_vecCreateAnimObjectLayerTag[i]);
		AnimationJson[i].emplace("ObjectTag", m_vecCreateAnimObjectTags[i]);
		m_vecAnimObjects[i]->Write_Json(AnimationJson[i]);
	}
	
	

	for (auto& tag : m_vecCreateNonAnimObjectTags) {
		// 문자열에서 '@' 문자 이후의 부분을 지움
		size_t atIndex = tag.find('@');
		if (atIndex != std::string::npos) {
			tag.erase(atIndex); // '@' 이후의 문자열을 모두 제거
		}

		// 결과 출력
		std::cout << tag << std::endl;
	}

	for (_int i = 0; i < iNonAnimObjectSize; i++)
	{
		
		NonAnimationJson[i].emplace("Index" , i);
		NonAnimationJson[i].emplace("LayerTag", m_vecCreateNonAnimObjectLayerTag[i]);
		NonAnimationJson[i].emplace("ObjectTag", m_vecCreateNonAnimObjectTags[i]);
		m_vecNonAnimObjects[i]->Write_Json(NonAnimationJson[i]);
	}

	if (FAILED(CJson_Utility::Save_Json(strAnimPath.c_str(), AnimationJson)))
	{
		MSG_BOX("애니메이션 모델 세이브 실패");
	}
	else
	{
		MSG_BOX("애니메이션 모델 저장 성공");
	}

	if (FAILED(CJson_Utility::Save_Json(strNonAnimPATH.c_str(), NonAnimationJson)))
	{
		MSG_BOX("논 애니메이션 모델 세이브 실패");
	}
	else
	{
		MSG_BOX("논 애니메이션 모델 저장 성공");
	}

}

void CImgui_Manager::LoadAnimObject(string strPath, string strFileName)
{
	json LoadJson;

	string strFullPath = strPath + "/" + strFileName;

	CJson_Utility::Load_Json(strFullPath.c_str(), LoadJson);

	_int JsonSize = LoadJson.size();
	m_PickingObject = nullptr;

	ClearAnimObjects();
	
	for (_int i = 0; i < JsonSize; i++)
	{
		m_vecCreateAnimObjectLayerTag.push_back(LoadJson[i]["LayerTag"]);
		m_vecCreateAnimObjectTags.push_back(LoadJson[i]["ObjectTag"]);

		CGameObject* pGameObject = nullptr;

		CGameObject::GAMEOBJECT_DESC Desc;

		Desc.iLevelIndex = LEVEL_TOOL;

		wstring wstrLayerTag = ConvertStrToWstr(LoadJson[i]["LayerTag"]);
		wstring wstrObjectTag = ConvertStrToWstr(LoadJson[i]["ObjectTag"]);

		m_pGameInstance->Add_CloneObject(LEVEL_TOOL, wstrLayerTag, wstrObjectTag, &Desc, &pGameObject);

		const json& TransformJson = LoadJson[i]["Component"]["Transform"];

		_float4x4 WorldMatrix;

		for (_int i = 0; i < 4; ++i)
		{
			for (_int j = 0; j < 4; ++j)
			{
				WorldMatrix.m[i][j] = TransformJson[i][j];
			}
		}


		pGameObject->Get_Transform()->Set_WorldFloat4x4(WorldMatrix);

		m_vecAnimObjects.push_back(pGameObject);

	}
	
	
	

}
	
void CImgui_Manager::LoadNonAnimObject(string strPath, string strFileName)
{
	json LoadJson;

	string strFullPath = strPath + "/" + strFileName;

	CJson_Utility::Load_Json(strFullPath.c_str(), LoadJson);

	_int JsonSize = LoadJson.size();
	m_PickingObject = nullptr;

	ClearNonAnimObjects();

	for (_int i = 0; i < JsonSize; i++)
	{
		string IndexTag = "@" + to_string(i);
		string pushObjectTag = string(LoadJson[i]["ObjectTag"]) + IndexTag;

		m_vecCreateNonAnimObjectLayerTag.push_back(LoadJson[i]["LayerTag"]);
		m_vecCreateNonAnimObjectTags.push_back(pushObjectTag);

		CGameObject* pGameObject = nullptr;

		CEnvironment_Object::ENVIRONMENT_DESC Desc;

		Desc.iLevelIndex = LEVEL_TOOL;
		Desc.strModelTag = ConvertStrToWstr(LoadJson[i]["ObjectTag"]);

		wstring wstrLayerTag = ConvertStrToWstr(LoadJson[i]["LayerTag"]);
		

		m_pGameInstance->Add_CloneObject(LEVEL_TOOL, wstrLayerTag, TEXT("Prototype_GameObject_Environment"), &Desc, &pGameObject);

		const json& TransformJson = LoadJson[i]["Component"]["Transform"];

		_float4x4 WorldMatrix;

		for (_int i = 0; i < 4; ++i)
		{
			for (_int j = 0; j < 4; ++j)
			{
				WorldMatrix.m[i][j] = TransformJson[i][j];
			}
		}


		pGameObject->Get_Transform()->Set_WorldFloat4x4(WorldMatrix);

		m_vecNonAnimObjects.push_back(pGameObject);

	}
}

void CImgui_Manager::ClearAnimObjects()
{
	_int iAnimObjectsSize = m_vecAnimObjects.size();
	

	for (_int i = 0; i < iAnimObjectsSize; ++i)
	{
		m_vecCreateAnimObjectTags.erase(m_vecCreateAnimObjectTags.begin() + i);
		m_pGameInstance->Erase_CloneObject(LEVEL_TOOL, ConvertStrToWstr(m_vecCreateAnimObjectLayerTag[i]), m_vecAnimObjects[i]);
		m_vecCreateAnimObjectLayerTag.erase(m_vecCreateAnimObjectLayerTag.begin() + i);
		m_vecAnimObjects.erase(m_vecAnimObjects.begin() + i);
	}

	
}

void CImgui_Manager::ClearNonAnimObjects()
{
	_int iNonAnimObjectsSize = m_vecNonAnimObjects.size();

	for (_int i = 0; i < iNonAnimObjectsSize; ++i)
	{
		
		m_pGameInstance->Erase_CloneObject(LEVEL_TOOL, ConvertStrToWstr(m_vecCreateNonAnimObjectLayerTag[i]), m_vecNonAnimObjects[i]);
		m_vecNonAnimObjects.erase(m_vecNonAnimObjects.begin() + i);
	}

	m_vecCreateNonAnimObjectTags.clear();
	m_vecCreateNonAnimObjectLayerTag.clear();

	
}

HRESULT CImgui_Manager::StartBakeBinary()
{


	return S_OK;
}

HRESULT CImgui_Manager::BinaryConvert(string strFileName, string strFilePath, const MODEL_TYPE& eModelType)
{
	//! strFilePath가 곧 sourceUpperPath

	if (FAILED(ReadFBX(strFilePath, eModelType)))
		return E_FAIL;

	if (FAILED(Read_BoneData(m_pAiScene->mRootNode, 0, -1, 0)))
		return E_FAIL;

	if (FAILED(Read_MeshData(eModelType)))
		return E_FAIL;

	if (FAILED(Write_MeshData(strFileName)))
		return E_FAIL;

	if (FAILED(Write_BoneData(strFileName)))
		return E_FAIL;

	if (FAILED(Read_MaterialData()))
		return E_FAIL;

	if (FAILED(Write_MaterialData(strFileName)))
		return E_FAIL;

	if (MODEL_TYPE::TYPE_ANIM == eModelType)
	{
		if (FAILED(Read_AnimationData()))
			return E_FAIL;

		if (FAILED(Write_AnimationData(strFileName)))
			return E_FAIL;
	}

	if(FAILED(CreateModelInfo(eModelType, strFileName)))
		return E_FAIL;
	
	m_vecBones.clear();
	m_vecMesh.clear();
	m_vecMaterial.clear();
	m_vecAnimation.clear();

	return S_OK;
}

HRESULT CImgui_Manager::ReadFBX(string strFilePath, const MODEL_TYPE& eModelType)
{
	if (MODEL_TYPE::TYPE_END == eModelType)
		return E_FAIL;

	_int iFlag = 0;

	if (MODEL_TYPE::TYPE_NONANIM == eModelType)
		iFlag |= aiProcess_PreTransformVertices | aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast;
	else
		iFlag |= aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast;

	m_pAiScene = m_Impoter.ReadFile(strFilePath, iFlag);

	if (nullptr == m_pAiScene)
		return E_FAIL;

	return S_OK;
}

HRESULT CImgui_Manager::Read_BoneData(aiNode* pAINode, _int iIndex, _int iParentIndex, _int iDepth)
{
	if (nullptr == pAINode)
		return E_FAIL;

	asBone* pBone = new asBone;

	pBone->strName = pAINode->mName.data;
	pBone->iParent = iParentIndex;
	pBone->iIndex = iIndex;
	pBone->iDepth = iDepth;
	

	_float4x4 matTransformation;

	memcpy(&matTransformation, &pAINode->mTransformation, sizeof(_float4x4));

	XMStoreFloat4x4(&pBone->matTransformation, XMMatrixTranspose(XMLoadFloat4x4(&matTransformation)));

	m_vecBones.push_back(pBone);


	for (size_t i = 0; i < pAINode->mNumChildren; ++i)
	{
		Read_BoneData(pAINode->mChildren[i], m_vecBones.size(), iIndex, pBone->iDepth + 1);
	}

	return S_OK;
}

HRESULT CImgui_Manager::Write_BoneData(string strFileName)
{

	string strNoExtFileName = filesystem::path(strFileName).stem().string();
	string strEXT = ".Bone";
	
	string strCreatePath = CheckOrCreatePath(strNoExtFileName) + "\\";
	

	string strFullPath = strCreatePath + strNoExtFileName + strEXT;
	

	HANDLE	hFile;

	hFile = CreateFile(ConvertStrToWstr(strFullPath).c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	if (hFile == INVALID_HANDLE_VALUE)
		return E_FAIL;

	DWORD dwByte;
	size_t vecBonesSize = m_vecBones.size();
	WriteFile(hFile, &vecBonesSize, sizeof(size_t), &dwByte, nullptr);

	for (asBone* pBone : m_vecBones)
	{
		size_t strLength = pBone->strName.size();
		WriteFile(hFile, &strLength, sizeof(size_t), &dwByte, nullptr);
		WriteFile(hFile, pBone->strName.c_str(), strLength, &dwByte, nullptr);
		WriteFile(hFile, &pBone->matTransformation, sizeof(XMFLOAT4X4), &dwByte, nullptr);
		WriteFile(hFile, &pBone->matOffset, sizeof(XMFLOAT4X4), &dwByte, nullptr);
		WriteFile(hFile, &pBone->iIndex, sizeof(_int), &dwByte, nullptr);
		WriteFile(hFile, &pBone->iParent, sizeof(_int), &dwByte, nullptr);
		WriteFile(hFile, &pBone->iDepth, sizeof(_uint), &dwByte, nullptr);
	}

	

	CloseHandle(hFile);

	return S_OK;
}

HRESULT CImgui_Manager::Read_MeshData(const MODEL_TYPE& eModelType)
{
	if(nullptr == m_pAiScene)
		return E_FAIL;


	for (_uint i = 0; i < m_pAiScene->mNumMeshes; ++i)
	{
		aiMesh* pAIMesh = m_pAiScene->mMeshes[i];
		

		asMesh* pMeshData = new asMesh;
		//ZEROMEMORY(pMeshData);
		//pMeshData->strName = string();
		//
		pMeshData->strName = pAIMesh->mName.data;

		if (eModelType == MODEL_TYPE::TYPE_NONANIM)
		{
			pMeshData->iNumFace = pAIMesh->mNumFaces;
			pMeshData->isAnim = (UINT)eModelType;

			pMeshData->vecNonAnims.reserve(pAIMesh->mNumVertices);

			VTXMESH vertex{};

			for (_uint j = 0; j < pAIMesh->mNumVertices; ++j)
			{
				memcpy(&vertex.vPosition, &pAIMesh->mVertices[j], sizeof(_float3));
				memcpy(&vertex.vNormal, &pAIMesh->mNormals[j], sizeof(_float3));
				memcpy(&vertex.vTexcoord, &pAIMesh->mTextureCoords[0][j], sizeof(_float2));
				memcpy(&vertex.vTangent, &pAIMesh->mTangents[j], sizeof(_float3));

				pMeshData->vecNonAnims.push_back(vertex);
			}
		}

		else if (eModelType == MODEL_TYPE::TYPE_ANIM)
		{
			pMeshData->isAnim = (UINT)eModelType;
			pMeshData->iNumFace = pAIMesh->mNumFaces;
			pMeshData->vecAnims.reserve(pAIMesh->mNumVertices);

			for (size_t j = 0; j < pAIMesh->mNumVertices; j++)
				pMeshData->vecAnims.push_back(VTXANIMMESH{});

			for (size_t j = 0; j < pMeshData->vecAnims.size(); j++)
			{
				memcpy(&pMeshData->vecAnims[j].vPosition, &pAIMesh->mVertices[j], sizeof(_float3));
				memcpy(&pMeshData->vecAnims[j].vNormal, &pAIMesh->mNormals[j], sizeof(_float3));
				memcpy(&pMeshData->vecAnims[j].vTexcoord, &pAIMesh->mTextureCoords[0][j], sizeof(_float2));
				memcpy(&pMeshData->vecAnims[j].vTangent, &pAIMesh->mTangents[j], sizeof(_float3));
			}

			/* Static과 달리 해당 메시에 영향을 주는 뼈의 정보를 저장한다. */
			for (_uint j = 0; j < pAIMesh->mNumBones; ++j)
			{
				aiBone* pAIBone = pAIMesh->mBones[j];

				for (_uint k = 0; k < pAIBone->mNumWeights; ++k)
				{
					_uint		iVertexIndex = pAIBone->mWeights[k].mVertexId;
											   pAIBone->mWeights[j].mVertexId;
					if (0.0f == pMeshData->vecAnims[iVertexIndex].vBlendWeights.x)
					{
						pMeshData->vecAnims[iVertexIndex].vBlendIndices.x = j;
						pMeshData->vecAnims[iVertexIndex].vBlendWeights.x = pAIBone->mWeights[k].mWeight;
					}

					else if (0.0f == pMeshData->vecAnims[iVertexIndex].vBlendWeights.y)
					{
						pMeshData->vecAnims[iVertexIndex].vBlendIndices.y = j;
						pMeshData->vecAnims[iVertexIndex].vBlendWeights.y = pAIBone->mWeights[k].mWeight;
					}

					else if (0.0f == pMeshData->vecAnims[iVertexIndex].vBlendWeights.z)
					{
						pMeshData->vecAnims[iVertexIndex].vBlendIndices.z = j;
						pMeshData->vecAnims[iVertexIndex].vBlendWeights.z = pAIBone->mWeights[k].mWeight;
					}

					else if (0.0f == pMeshData->vecAnims[iVertexIndex].vBlendWeights.w)
					{
						pMeshData->vecAnims[iVertexIndex].vBlendIndices.w = j;
						pMeshData->vecAnims[iVertexIndex].vBlendWeights.w = pAIBone->mWeights[k].mWeight;
					}
				}
			}
		}

		pMeshData->vecIndices.resize(pAIMesh->mNumFaces);

		_uint iNumIndice = { 0 };

		for (_uint j = 0; j < pAIMesh->mNumFaces; ++j)
		{
			aiFace& AIFace = pAIMesh->mFaces[j];
			
			pMeshData->vecIndices[j] = { AIFace.mIndices[0], AIFace.mIndices[1], AIFace.mIndices[2] };
		}

		pMeshData->iMaterialIndex = pAIMesh->mMaterialIndex;

		/* Bones (현재 메시에 영향을 주는 뼈들을 순회하며 행렬 정보를 저장하고 뼈들을 컨테이너에 모아둔다. */
		_uint numBones = pAIMesh->mNumBones;

		for (_uint j = 0; j < numBones; j++)
		{
			aiBone* srcMeshBone = pAIMesh->mBones[j];

			_float4x4 OffsetMatrix;
			memcpy(&OffsetMatrix, &srcMeshBone->mOffsetMatrix, sizeof(_float4x4));
			XMStoreFloat4x4(&OffsetMatrix, XMMatrixTranspose(XMLoadFloat4x4(&OffsetMatrix)));

			_uint boneIndex = Get_BoneIndex(srcMeshBone->mName.C_Str());
			m_vecBones[boneIndex]->matOffset = OffsetMatrix;

			pMeshData->vecBoneIndices.push_back(boneIndex);
			pMeshData->vecOffsetMatrix.push_back(OffsetMatrix);
		}

		m_vecMesh.push_back(pMeshData);
	}

	return S_OK;
}

HRESULT CImgui_Manager::Write_MeshData(string strFileName)
{
	string strNoExtFileName = filesystem::path(strFileName).stem().string();
	string strEXT = ".mesh";

	string strCreatePath = CheckOrCreatePath(strNoExtFileName) + "\\";

	string strFullPath = strCreatePath + strNoExtFileName + strEXT;

	HANDLE	hFile;

	hFile = CreateFile(ConvertStrToWstr(strFullPath).c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	DWORD dwByte = 0;

	size_t MeshSize = m_vecMesh.size();
	WriteFile(hFile, &MeshSize, sizeof(size_t), &dwByte, nullptr);

	for (_int i = 0; i < m_vecMesh.size(); ++i)
	{
		size_t strLength = m_vecMesh[i]->strName.size();
		WriteFile(hFile, &strLength, sizeof(size_t), &dwByte, nullptr);
		WriteFile(hFile, m_vecMesh[i]->strName.c_str(), strLength, &dwByte, nullptr);
		WriteFile(hFile, &m_vecMesh[i]->isAnim, sizeof(_bool), &dwByte, nullptr);


		if (m_vecMesh[i]->isAnim == (_uint)MODEL_TYPE::TYPE_NONANIM)
		{
			size_t vecNonAnimsSize = m_vecMesh[i]->vecNonAnims.size();
			WriteFile(hFile, &vecNonAnimsSize, sizeof(size_t), &dwByte, nullptr);
			for (VTXMESH& vertex : m_vecMesh[i]->vecNonAnims)
			{
				WriteFile(hFile, &vertex.vPosition, sizeof(_float3), &dwByte, nullptr);
				WriteFile(hFile, &vertex.vNormal, sizeof(_float3), &dwByte, nullptr);
				WriteFile(hFile, &vertex.vTexcoord, sizeof(_float2), &dwByte, nullptr);
				WriteFile(hFile, &vertex.vTangent, sizeof(_float3), &dwByte, nullptr);
			}
		}
		else
		{
			size_t vecAnimsSize = m_vecMesh[i]->vecAnims.size();
			WriteFile(hFile, &vecAnimsSize, sizeof(size_t), &dwByte, nullptr);
			for (VTXANIMMESH& vertex : m_vecMesh[i]->vecAnims)
			{
				WriteFile(hFile, &vertex.vPosition, sizeof(_float3), &dwByte, nullptr);
				WriteFile(hFile, &vertex.vNormal, sizeof(_float3), &dwByte, nullptr);
				WriteFile(hFile, &vertex.vTexcoord, sizeof(_float2), &dwByte, nullptr);
				WriteFile(hFile, &vertex.vTangent, sizeof(_float3), &dwByte, nullptr);

				WriteFile(hFile, &vertex.vBlendIndices, sizeof(XMUINT4), &dwByte, nullptr);
				WriteFile(hFile, &vertex.vBlendWeights, sizeof(_float4), &dwByte, nullptr);
			}
		}

		size_t vecIndicesSize = m_vecMesh[i]->vecIndices.size();
		WriteFile(hFile, &vecIndicesSize, sizeof(size_t), &dwByte, nullptr);
		for (FACEINDICES32& index : m_vecMesh[i]->vecIndices)
		{
			WriteFile(hFile, &index, sizeof(FACEINDICES32), &dwByte, nullptr);
		}

		WriteFile(hFile, &m_vecMesh[i]->iMaterialIndex, sizeof(_uint), &dwByte, nullptr);
		WriteFile(hFile, &m_vecMesh[i]->iNumFace, sizeof(_int), &dwByte, nullptr);

		size_t vecBoneIndicesSize = m_vecMesh[i]->vecBoneIndices.size();
		WriteFile(hFile, &vecBoneIndicesSize, sizeof(size_t), &dwByte, nullptr);
		for (_int& index : m_vecMesh[i]->vecBoneIndices)
		{
			WriteFile(hFile, &index, sizeof(_int), &dwByte, nullptr);
		}

		size_t vecOffsetMatrixSize = m_vecMesh[i]->vecOffsetMatrix.size();
		WriteFile(hFile, &vecOffsetMatrixSize, sizeof(size_t), &dwByte, nullptr);
		for (_float4x4& OffsetMatrix : m_vecMesh[i]->vecOffsetMatrix)
		{
			WriteFile(hFile, &OffsetMatrix, sizeof(_float4x4), &dwByte, nullptr);
		}

	}

	CloseHandle(hFile);

	return S_OK;
}

HRESULT CImgui_Manager::Read_MaterialData()
{
	for (_uint i = 0; i < m_pAiScene->mNumMaterials; ++i)
	{
		aiMaterial* pMaterial = m_pAiScene->mMaterials[i];

		if (nullptr == pMaterial)
			return E_FAIL;

		asMaterial* pMaterialData = new asMaterial;

		aiString aifile;
		string strName;

		for(size_t j = 1; j < AI_TEXTURE_TYPE_MAX; j++)
		{
			if (FAILED(pMaterial->GetTexture(aiTextureType(j), 0, &aifile)))
					continue;

			strName = aifile.data;

			pMaterialData->strTextureFilePath[j] = filesystem::path(strName).filename().string();
			//pMaterialData->
		}

		m_vecMaterial.push_back(pMaterialData);
	}

	return S_OK;
}

HRESULT CImgui_Manager::Write_MaterialData(string strFileName)
{
	string strNoExtFileName = filesystem::path(strFileName).stem().string();
	string strEXT = ".mat";

	string strCreatePath = CheckOrCreatePath(strNoExtFileName) + "\\";

	string strFullPath = strCreatePath + strNoExtFileName + strEXT;

	HANDLE	hFile;

	hFile = CreateFile(ConvertStrToWstr(strFullPath).c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	if (nullptr == hFile)
	{
		return E_FAIL;
	}
	DWORD dwByte = 0;

	size_t vecMaterialSize = m_vecMaterial.size();
	WriteFile(hFile, &vecMaterialSize, sizeof(size_t), &dwByte, nullptr);

	for (asMaterial* pMaterialData : m_vecMaterial)
	{
		for (string strPath : pMaterialData->strTextureFilePath)
		{
			size_t strLength = strPath.size();
			WriteFile(hFile, &strLength, sizeof(size_t), &dwByte, nullptr);
			WriteFile(hFile, strPath.c_str(), strLength, &dwByte, nullptr);
		}
	}

	CloseHandle(hFile);

	return S_OK;
}

HRESULT CImgui_Manager::Read_AnimationData()
{
	for (_uint i = 0; i < m_pAiScene->mNumAnimations; ++i)
	{
		aiAnimation* pAnimation = m_pAiScene->mAnimations[i];
		asAnimation* pAnimationData = new asAnimation;

		pAnimationData->strName = pAnimation->mName.data;
		pAnimationData->fDuration = pAnimation->mDuration;
		pAnimationData->fTicksPerSecond = pAnimation->mTicksPerSecond;

		for (_uint j = 0; j < pAnimation->mNumChannels; ++j)
		{
			asChannel* pChannelData = new asChannel;
			aiNodeAnim* pChannel = pAnimation->mChannels[j];

			pChannelData->strName = pChannel->mNodeName.data;

			_uint iNumKeyFrames;

			iNumKeyFrames = max(pChannel->mNumScalingKeys, pChannel->mNumRotationKeys);
			iNumKeyFrames = max(pChannel->mNumPositionKeys, iNumKeyFrames);

			_float3		vScale{};
			_float4		vRotation{};
			_float3		vPosition{};

			for (_uint k = 0; k < iNumKeyFrames; ++k)
			{
				asKeyFrame* pKeyFrameData = new asKeyFrame;

				if (k < pChannel->mNumScalingKeys)
				{
					memcpy(&vScale, &pChannel->mScalingKeys[k].mValue, sizeof(_float3));
					pKeyFrameData->fTrackPosition = pChannel->mScalingKeys[k].mTime;
				}

				if (k < pChannel->mNumRotationKeys)
				{
					//! 어심프의 로테이션키의 밸류는 aiQuaternion이야. x,y,z,w 순이아닌 w,x,y,z 순으로 되어있어
					//! 우리가 쓰던 거랑 순서가 다르지? 그래서 memcpy를 사용하면 안돼.

					vRotation.x = pChannel->mRotationKeys[k].mValue.x;
					vRotation.y = pChannel->mRotationKeys[k].mValue.y;
					vRotation.z = pChannel->mRotationKeys[k].mValue.z;
					vRotation.w = pChannel->mRotationKeys[k].mValue.w;
					pKeyFrameData->fTrackPosition = pChannel->mRotationKeys[k].mTime;
				}

				if (k < pChannel->mNumPositionKeys)
				{
					memcpy(&vPosition, &pChannel->mPositionKeys[k].mValue, sizeof(_float3));
					pKeyFrameData->fTrackPosition = pChannel->mPositionKeys[k].mTime;
				}

				pKeyFrameData->vScale = vScale;
				pKeyFrameData->vRotation = vRotation;
				pKeyFrameData->vPosition = vPosition;

				pChannelData->vecKeyFrames.push_back(pKeyFrameData);
			}
			pAnimationData->vecChannels.push_back(pChannelData);
		}
		m_vecAnimation.push_back(pAnimationData);
	}


	return S_OK;
}

HRESULT CImgui_Manager::Write_AnimationData(string strFileName)
{
	string strNoExtFileName = filesystem::path(strFileName).stem().string();
	string strEXT = ".anim";

	string strCreatePath = CheckOrCreatePath(strNoExtFileName) + "\\";

	string strFullPath = strCreatePath + strNoExtFileName + strEXT;

	HANDLE	hFile;

	hFile = CreateFile(ConvertStrToWstr(strFullPath).c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	if (nullptr == hFile)
	{
		return E_FAIL;
	}
	DWORD dwByte = 0;
	size_t vecAnimationSize = m_vecAnimation.size();
	WriteFile(hFile, &vecAnimationSize, sizeof(size_t), &dwByte, nullptr);
	for (asAnimation* pAnimationData : m_vecAnimation)
	{
		size_t strLength = pAnimationData->strName.size();
		WriteFile(hFile, &strLength, sizeof(size_t), &dwByte, nullptr);
		WriteFile(hFile, pAnimationData->strName.c_str(), strLength, &dwByte, nullptr);

		WriteFile(hFile, &pAnimationData->fDuration, sizeof(_float), &dwByte, nullptr);
		WriteFile(hFile, &pAnimationData->fTicksPerSecond, sizeof(_float), &dwByte, nullptr);

		size_t vecChannelSize = pAnimationData->vecChannels.size();
		WriteFile(hFile, &vecChannelSize, sizeof(size_t), &dwByte, nullptr);
		for (asChannel* pChannelData : pAnimationData->vecChannels)
		{
			size_t strLength = pChannelData->strName.size();
			WriteFile(hFile, &strLength, sizeof(size_t), &dwByte, nullptr);
			WriteFile(hFile, pChannelData->strName.c_str(), strLength, &dwByte, nullptr);


			size_t vecKeyFrameSize = pChannelData->vecKeyFrames.size();
			WriteFile(hFile, &vecKeyFrameSize, sizeof(size_t), &dwByte, nullptr);
			for (asKeyFrame* pKeyFrameData : pChannelData->vecKeyFrames)
			{
				WriteFile(hFile, &pKeyFrameData->fTrackPosition, sizeof(_float), &dwByte, nullptr);
				WriteFile(hFile, &pKeyFrameData->vScale, sizeof(_float3), &dwByte, nullptr);
				WriteFile(hFile, &pKeyFrameData->vRotation, sizeof(_float4), &dwByte, nullptr);
				WriteFile(hFile, &pKeyFrameData->vPosition, sizeof(_float3), &dwByte, nullptr);
			}
		}
	}

	CloseHandle(hFile);

	return S_OK;
}

HRESULT CImgui_Manager::CreateModelInfo(const MODEL_TYPE& eModelType, string strFileName)
{
	
	string strNoExtFileName = filesystem::path(strFileName).stem().string();
	
	string strBone = "_INFO_Bone.txt";
	string strMesh = "_INFO_Mesh.txt";
	string strAnimation = "_INFO_Animation.txt";
	string strMaterial = "_INFO_Material.txt";

	string strCreatePath = CheckOrCreatePath(strNoExtFileName + "\\Info") + "\\";
	string strTest;

	string strBoneInfoPath = strCreatePath + strNoExtFileName + strBone;
	string strMeshInfoPath = strCreatePath + strNoExtFileName + strMesh;
	string strAnimationInfoPath = strCreatePath + strNoExtFileName + strAnimation;
	string strMaterialInfoPath = strCreatePath + strNoExtFileName + strMaterial;


	ofstream OutBoneFile(strBoneInfoPath.c_str());
	ofstream OutMeshFile(strMeshInfoPath.c_str());
	ofstream OutAnimationFile(strAnimationInfoPath.c_str());
	ofstream OutMaterialFile(strMaterialInfoPath.c_str());

	if (OutBoneFile.is_open())
	{
		OutBoneFile << " =========== " << strNoExtFileName << " 정보 " << " =========== " << endl;

		OutBoneFile << " ----------- " << " 뼈 " << " 정보 " << " ----------- " << endl;
		
		for (auto& pAsBone : m_vecBones)
		{
			OutBoneFile << " 이름 : " << pAsBone->strName << endl;
			OutBoneFile << " 부모 인덱스 : " << pAsBone->iParent << endl;
			OutBoneFile << " 자신 인덱스 : " << pAsBone->iIndex  << endl;

			OutBoneFile << endl << endl;
		}

		OutBoneFile.close();
	}

	if (OutMeshFile.is_open())
	{
		OutMeshFile << " ----------- " << " 메쉬 " << " 정보 " << " ----------- " << endl;

		for (auto& pAsMesh : m_vecMesh)
		{
			OutMeshFile << " 이름 : " << pAsMesh->strName << endl;

			//!m_iIndexStride == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
			string strType = pAsMesh->isAnim == 0 ? " 논 애님 " : " 애님 ";


			OutMeshFile << " 타입 " << strType << endl;

			OutMeshFile << " ============== 구분선 ============== " << endl;
		}

		OutMeshFile.close();
	}

	if (OutAnimationFile.is_open() && eModelType == TYPE_ANIM)
	{
		OutAnimationFile << " ----------- " << " 애니메이션 " << " 정보 " << " ----------- " << endl;

		_int iIndex = 0;

		for (auto& pAsAnimation : m_vecAnimation)
		{
			OutAnimationFile << " 이름 : " << pAsAnimation->strName << endl;
			OutAnimationFile << " 인덱스 : " << iIndex << endl;
			OutAnimationFile << " 틱 : " << pAsAnimation->fTicksPerSecond << endl;
			OutAnimationFile << " 총 재생 길이 : " << pAsAnimation->fDuration << endl;

			++iIndex;
		}

		OutAnimationFile.close();
	}

	if (OutMaterialFile.is_open())
	{
		OutMaterialFile << " ----------- " << " 머터리얼 " << " 정보 " << " ----------- " << endl;

		for (auto& pAsMaterial : m_vecMaterial)
		{
			OutMaterialFile << " 파일 경로 : " << pAsMaterial->strTextureFilePath << endl << endl;
		}

		OutMaterialFile.close();
	}
		
	return S_OK;
}

_uint CImgui_Manager::Get_BoneIndex(const char* szName)
{
	for (_uint i = 0; i < m_vecBones.size(); ++i)
	{
		if (!strcmp(m_vecBones[i]->strName.c_str(), szName))
			return i;
	}

	return 0;
}

//string CImgui_Manager::m_pGameInstance->ConvertWstrToStr(const wstring& str)
//{
//	wstring_convert<codecvt_utf8<_tchar>> converter;
//	string ChangeStr = converter.to_bytes(str);
//	return ChangeStr;
//}
//
//wstring CImgui_Manager::m_pGameInstance->ConvertStrToWstr(const string& str)
//{
//	wstring_convert<codecvt_utf8<wchar_t>> converter;
//	wstring wideStr = converter.from_bytes(str);
//	return wideStr;
//}

void CImgui_Manager::ImGui_NaviToolTick()
{
	if (ImGui::BeginTabItem(u8"내비게이션"))
	{
		m_eToolID = CImgui_Manager::TOOL_NAVI;

		if (ImGui::Button(u8"저장하기")) { m_eDialogMode = CImgui_Manager::DIALOG_SAVE; OpenDialog(m_eToolID); } ImGui::SameLine(); if (ImGui::Button(u8"불러오기")) { m_eDialogMode = CImgui_Manager::DIALOG_LOAD; OpenDialog(m_eToolID); }

		ImGui::RadioButton(u8"만들기", &m_iNaviToolMode, 0); ImGui::SameLine(); ImGui::RadioButton(u8"삭제", &m_iNaviToolMode, 1);

		ImGui::Text(u8"마우스 X : %f", m_fNaviPickingPos.x);
		ImGui::Text(u8"마우스 Y : %f", m_fNaviPickingPos.y);
		ImGui::Text(u8"마우스 Z : %f", m_fNaviPickingPos.z);

		if (m_iNaviToolMode == 0)
		{
			Create_Navi_Mode_Tick();
		}

		else if (m_iNaviToolMode == 1)
		{
			Delete_Navi_Mode_Tick();
		}

		if(nullptr != m_pNavigation)
		m_pNavigation->Render();

		ImGui::EndTabItem();
	}
}

void CImgui_Manager::Create_Navi_Mode_Tick()
{
	_int iObjectListSize = m_vecCreateNonAnimObjectTags.size();

	if (nullptr == m_pNaviTargetObject)
	{
		if (ImGui::BeginListBox(u8""))
		{
			for (_int i = 0; i < iObjectListSize; ++i)
			{
				const _bool isSelected = (m_iTargetIndex == i);

				if (ImGui::Selectable(m_vecCreateNonAnimObjectTags[i].c_str(), isSelected))
				{
					m_pNaviTargetObject = m_vecNonAnimObjects[i];
					m_iTargetIndex = i;
					m_pNavigation = dynamic_cast<CEnvironment_Object*>(m_pNaviTargetObject)->Get_NaviCom();

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndListBox();
		}
	}	

	else
	{
		
		_int iPickedSize = m_vecPickingListBox.size();

		static _int iPickingIndex = 0;

		if (!m_vecPickedPoints.empty())
		{
			if (ImGui::BeginListBox(u8""))
			{
				for (_int i = 0; i < iPickedSize; ++i)
				{
					const _bool isSelected = (m_iNaviListBoxIndex == i);

					if (ImGui::Selectable(m_vecPickingListBox[i].c_str(), isSelected))
					{
						m_iNaviListBoxIndex = i;

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndListBox();
			}

			if (m_iNaviListBoxIndex != -1)
			{
				ImGui::Text(u8"픽킹 X : %f", m_vecPickedPoints[m_iNaviListBoxIndex].x);
				ImGui::Text(u8"픽킹 Y : %f", m_vecPickedPoints[m_iNaviListBoxIndex].y);
				ImGui::Text(u8"픽킹 Z : %f", m_vecPickedPoints[m_iNaviListBoxIndex].z);
			}

			if (ImGui::Button(u8"픽킹인덱스 삭제"))
			{
				if (m_iNaviListBoxIndex < m_vecPickedPoints.size()) {
					m_vecPickedPoints.erase(m_vecPickedPoints.begin() + m_iNaviListBoxIndex);
					m_vecPickingListBox.erase(m_vecPickingListBox.begin() + m_iNaviListBoxIndex);
					
					if(m_vecPickingListBox.size() == 0)
						m_iNaviListBoxIndex = -1;
					else
						m_iNaviListBoxIndex = m_vecPickingListBox.size() - 1;
					
				}
			}
		}
		
		
		

		if (ImGui::Button(u8"생성") )
		{
			if (3 > m_iCurrentPickingIndex)
				return;

			

			vector<double> fPoints; 
			//fPoints.reserve(iPickedSize * 2);

			for (_int i = 0; i < iPickedSize; ++i)
			{
				fPoints.push_back(m_vecPickedPoints[i].x);
				
				fPoints.push_back(m_vecPickedPoints[i].z);
			}

			
			delaunator::Delaunator d(fPoints);


			for (size_t i = 0; i < d.triangles.size(); i += 3)
			{
				//"Triangle points: [[%f, %f], [%f, %f], [%f, %f]]\n",
				//	d.coords[2 * d.triangles[i]],        //tx0            
				//	d.coords[2 * d.triangles[i] + 1],    //ty0
				//	d.coords[2 * d.triangles[i + 1]],    //tx1
				//	d.coords[2 * d.triangles[i + 1] + 1],//ty1
				//	d.coords[2 * d.triangles[i + 2]],    //tx2
				//	d.coords[2 * d.triangles[i + 2] + 1] //ty2
				_float3 points[3] = { m_vecPickedPoints[d.triangles[i]], m_vecPickedPoints[d.triangles[i + 1]], m_vecPickedPoints[d.triangles[i + 2]] };

				Set_CCW(points);

				CCell* pCell = CCell::Create(m_pDevice, m_pContext, points, m_iNaviIndex++);

				m_pNavigation->AddCell(pCell);
			}

			Reset_NaviPicking();
		}

		ImGui::Checkbox(u8"픽킹모드", &m_bPickingNaviMode);
		

		if (m_pGameInstance->Mouse_Down(DIM_LB) && true == ImGui_MouseInCheck() && true == m_bPickingNaviMode)
		{
			_int index = 0;
			
			_float3 fPickedPos = { 0.f, 0.f, 0.f };

			if (m_pNaviTargetObject->Picking(_float3(), dynamic_cast<CModel*>(m_pNaviTargetObject->Find_Component(TEXT("Com_Model"))), &fPickedPos))
			{
				Find_NearPointPos(&fPickedPos);
				m_vecPickedPoints.push_back(fPickedPos);
				m_vecPickingListBox.push_back(to_string(m_iNaviPickingIndex));
				++m_iCurrentPickingIndex;
				++m_iNaviPickingIndex;
				m_fNaviPickingPos = fPickedPos;
			}
// 			_int	iNonAnimObjectSize = m_vecNonAnimObjects.size();
// 
// 			_int	iIndex = 0;
// 			_float fHighestYValue = -FLT_MAX;
// 			_float3 vHighestPickesPos = {};
// 			_bool	bIsPicking = false;
// 			
// 			for (_int i = 0; i < iNonAnimObjectSize; ++i)
// 			{
// 				_float3 vPickedPos = {};
// 
// 				if (m_vecNonAnimObjects[i]->Picking(m_fPickingPos, dynamic_cast<CModel*>(m_vecNonAnimObjects[i]->Find_Component(TEXT("Com_Model"))), &vPickedPos) && true == ImGui_MouseInCheck())
// 				{
// 					_float3 vDestPos;
// 					XMStoreFloat3(&vDestPos, m_vecNonAnimObjects[i]->Get_Transform()->Get_State(CTransform::STATE_POSITION));
// 
// 					if (vDestPos.y > fHighestYValue) //! 객체의 y값이  이전 객체의  y값보다 크다면
// 					{
// 						fHighestYValue = vDestPos.y; //! 이전객체의 y값은 현재 객체의 y값이 된다.
// 						iIndex = i;
// 						vHighestPickesPos = vPickedPos;
// 						bIsPicking = true;
// 					}
// 				}
// 			}
// 
// 			if (true == bIsPicking)
// 			{
// 				//! 반복문이 끝난 후 가장 y값이 높은 객체의 픽킹 위치를 기준으로
// 				
// 				Find_NearPointPos(&vHighestPickesPos);
// 				m_vecPickedPoints.push_back(vHighestPickesPos);
// 				++m_iCurrentPickingIndex;
// 
// 				m_fNaviPickingPos = vHighestPickesPos;
// 			}
		}

		ImGui::NewLine();
		
		if (ImGui::Button(u8"타겟해제"))
		{
			m_pNaviTargetObject = nullptr;
			m_pNavigation = nullptr;
		}
	}
}

void CImgui_Manager::Delete_Navi_Mode_Tick()
{
	vector<CCell*> vecCells = m_pNavigation->Get_Cells();
	_int iCellSize = vecCells.size();

	if (m_pGameInstance->Mouse_Down(DIM_LB) && true == ImGui_MouseInCheck())
	{
		_int index = 0;

		_float3 fPickedPos = { 0.f, 0.f, 0.f };

		_int	iNonAnimObjectSize = m_vecNonAnimObjects.size();

		_int	iIndex = 0;
		_float fHighestYValue = -FLT_MAX;
		_float3 vHighestPickesPos = {};
		_bool	bIsPicking = false;

		//for (_int i = 0; i < iNonAnimObjectSize; ++i)
		//{
		//	_float3 vPickedPos = {};
		//
		//	if (m_vecNonAnimObjects[i]->Picking(m_fPickingPos, dynamic_cast<CModel*>(m_vecNonAnimObjects[i]->Find_Component(TEXT("Com_Model"))), &vPickedPos) && true == ImGui_MouseInCheck())
		//	{
		//		_float3 vDestPos;
		//		XMStoreFloat3(&vDestPos, m_vecNonAnimObjects[i]->Get_Transform()->Get_State(CTransform::STATE_POSITION));
		//
		//		if (vDestPos.y > fHighestYValue)
		//		{
		//			fHighestYValue = vDestPos.y;
		//			iIndex = i;
		//			vHighestPickesPos = vPickedPos;
		//			bIsPicking = true;
		//		}
		//	}
		//}

		if (m_pNaviTargetObject->Picking(_float3(), dynamic_cast<CModel*>(m_pNaviTargetObject->Find_Component(TEXT("Com_Model"))), &fPickedPos))
		{
			Find_NearPointPos(&fPickedPos);

			m_fNaviPickingPos = fPickedPos;
			bIsPicking = true;
		}

		if (true == bIsPicking)
		{
			CCell* pTargetCell = Find_NearCell(fPickedPos);

			if(nullptr == pTargetCell)
				return;

			m_pNavigation->Delete_Cell(pTargetCell->Get_Index());
		}
	}
	
}

void CImgui_Manager::Set_CCW(_float3* vPoint)
{
	_vector vPositionFromVector[3];
	for (int i(0); i < 3; i++)
		vPositionFromVector[i] = XMLoadFloat3(&(vPoint[i]));

	_vector vAtoB(vPositionFromVector[1] - vPositionFromVector[0]);
	_vector vAtoC(vPositionFromVector[2] - vPositionFromVector[0]);

	_vector vAtoB2D, vAtoC2D, vAtoB2DCross;
	vAtoB2D = vAtoC2D = vAtoB2DCross = XMVectorSet(0.f, 0.f, 0.f, 0.f);
	vAtoB2D = XMVectorSetX(vAtoB2D, XMVectorGetX(vAtoB));
	vAtoB2D = XMVectorSetY(vAtoB2D, XMVectorGetZ(vAtoB));
	vAtoC2D = XMVectorSetX(vAtoC2D, XMVectorGetX(vAtoC));
	vAtoC2D = XMVectorSetY(vAtoC2D, XMVectorGetZ(vAtoC));
	vAtoB2DCross = XMVectorSetX(vAtoB2DCross, -1.f * XMVectorGetY(vAtoB2D));
	vAtoB2DCross = XMVectorSetY(vAtoB2DCross, XMVectorGetX(vAtoB2D));
	_float fDot(XMVectorGetX(XMVector2Dot(vAtoB2DCross, vAtoC2D)));
	if (0.f < fDot)
	{
		XMStoreFloat3(&vPoint[1], vPositionFromVector[2]);
		XMStoreFloat3(&vPoint[2], vPositionFromVector[1]);
	}
}

void CImgui_Manager::Reset_NaviPicking()
{
	m_iCurrentPickingIndex = 0;
	m_vecPickedPoints.clear();
	m_vecPickingListBox.clear();

	m_iNaviListBoxIndex = 0;
	m_iNaviPickingIndex = 0;
}

void CImgui_Manager::Find_NearPointPos(_float3* fPickedPos)
{
	vector<CCell*> vecCells = m_pNavigation->Get_Cells();
	_int iCellSize = vecCells.size();
	_float fMinDistance = FLT_MAX;
	
	_float3 vPickedPos = *fPickedPos;



	for (_int i = 0; i < iCellSize; ++i)
	{
		_float3 vPointA = *vecCells[i]->Get_Point(CCell::POINT_A);
		_float3 vPointB = *vecCells[i]->Get_Point(CCell::POINT_B);
		_float3 vPointC = *vecCells[i]->Get_Point(CCell::POINT_C);

		_float distanceA = (_float)sqrt(pow(vPickedPos.x - vPointA.x, 2) +
			pow(vPickedPos.y - vPointA.y, 2) +
			pow(vPickedPos.z - vPointA.z, 2));

		_float distanceB = (_float)sqrt(pow(vPickedPos.x - vPointB.x, 2) +
			pow(vPickedPos.y - vPointB.y, 2) +
			pow(vPickedPos.z - vPointB.z, 2));

		_float distanceC = (_float)sqrt(pow(vPickedPos.x - vPointC.x, 2) +
			pow(vPickedPos.y - vPointC.y, 2) +
			pow(vPickedPos.z - vPointC.z, 2));

		if (distanceA < fMinDistance && distanceA < m_fCombinationRange)
		{
			fMinDistance = distanceA;
			*fPickedPos = vPointA;
		}

		if (distanceB < fMinDistance && distanceB < m_fCombinationRange)
		{
			fMinDistance = distanceB;
			*fPickedPos = vPointB;
		}

		if (distanceC < fMinDistance && distanceC < m_fCombinationRange)
		{
			fMinDistance = distanceC;
			*fPickedPos = vPointC;
		}


	}

}

CCell* CImgui_Manager::Find_NearCell(_float3 fPickedPos)
{
	vector<CCell*> vecCells = m_pNavigation->Get_Cells();
	_int iCellSize = vecCells.size();
	_float fMinDistance = FLT_MAX;

	_float3 vPickedPos = fPickedPos;



	for (_int i = 0; i < iCellSize; ++i)
	{
		_float3 vPointA = *vecCells[i]->Get_Point(CCell::POINT_A);
		_float3 vPointB = *vecCells[i]->Get_Point(CCell::POINT_B);
		_float3 vPointC = *vecCells[i]->Get_Point(CCell::POINT_C);

		_float distanceA = (_float)sqrt(pow(vPickedPos.x - vPointA.x, 2) +
			pow(vPickedPos.y - vPointA.y, 2) +
			pow(vPickedPos.z - vPointA.z, 2));

		_float distanceB = (_float)sqrt(pow(vPickedPos.x - vPointB.x, 2) +
			pow(vPickedPos.y - vPointB.y, 2) +
			pow(vPickedPos.z - vPointB.z, 2));

		_float distanceC = (_float)sqrt(pow(vPickedPos.x - vPointC.x, 2) +
			pow(vPickedPos.y - vPointC.y, 2) +
			pow(vPickedPos.z - vPointC.z, 2));

		if (distanceA < fMinDistance && distanceA < m_fCombinationRange)
		{
			fMinDistance = distanceA;
			return vecCells[i];
		}

		if (distanceB < fMinDistance && distanceB < m_fCombinationRange)
		{
			fMinDistance = distanceB;
			return vecCells[i];
		}

		if (distanceC < fMinDistance && distanceC < m_fCombinationRange)
		{
			fMinDistance = distanceC;
			return vecCells[i];
		}


	}

	return nullptr;
}

void CImgui_Manager::SaveNavi(string strFullPath)
{
	m_pNavigation->SaveData(ConvertStrToWstr(strFullPath));
}

void CImgui_Manager::LoadNavi(string strFullPath)
{
}

wstring CImgui_Manager::SliceObjectTag(const wstring& strObjectTag)
{
	size_t pos = strObjectTag.rfind(L"_");
	if (pos != wstring::npos)
	{
		return strObjectTag.substr(pos + 1);
	}
}

void CImgui_Manager::Replace(string& str, string comp, string rep)
{
	string temp = str;

	size_t start_pos = 0;
	while ((start_pos = temp.find(comp, start_pos)) != wstring::npos)
	{
		temp.replace(start_pos, comp.length(), rep);
		start_pos += rep.length();
	}

	str = temp;
}

vector<string> CImgui_Manager::Get_AllFolderNames(const string& strDirPath)
{
	vector<std::string> folderNames;
	try
	{
		for (const auto& entry : filesystem::directory_iterator(strDirPath))
		{
			if (filesystem::is_directory(entry))
			{
				folderNames.push_back(entry.path().filename().string());
			}
		}
	}
	catch (const std::exception& e)
	{
		MSG_BOX("폴더명을 얻어오지못햇음");
	}

	return folderNames;
}

string CImgui_Manager::CheckOrCreatePath(const string& strfileName)
{
	string strFilePath = "../Bin/Resources/Models/";

	string strCreateDirPath = filesystem::absolute(strFilePath).string() + strfileName;
	if (!filesystem::exists(strCreateDirPath))
		filesystem::create_directory(strCreateDirPath);

		return strCreateDirPath;
	
}

string CImgui_Manager::ConvertWstrToStr(const wstring& wstr)
{
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.length()), nullptr, 0, nullptr, nullptr);
	string str(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.length()), &str[0], size_needed, nullptr, nullptr);
	return str;
}

wstring CImgui_Manager::ConvertStrToWstr(const string& str)
{
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), nullptr, 0);
	wstring wstr(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.length()), &wstr[0], size_needed);
	return wstr;
}

void CImgui_Manager::Free()
{
	
	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	for (auto& pBoneData : m_vecBones)
		Safe_Delete(pBoneData);

	m_vecBones.clear();

	m_vecLayerTags.clear();

	m_vecAnimObjectTags.clear();
	m_vecCreateAnimObjectTags.clear();

	m_vecNonAnimObjectTags.clear();
	m_vecCreateNonAnimObjectTags.clear();


	//for(auto& pAnimObject : m_vecAnimObjects)
	//	Safe_Release(pAnimObject);
	//
	//for (auto& pNonAnimObject : m_vecNonAnimObjects)
	//	Safe_Release(pNonAnimObject);

	m_pFileDialog->Close();

	ImGuiFileDialog::Instance()->RemoveBookmark("Bin");
	ImGuiFileDialog::Instance()->RemoveBookmark("MapData");
	ImGuiFileDialog::Instance()->RemoveBookmark("ObjectData");

	Safe_Delete_Array(m_arrView);
	Safe_Delete_Array(m_arrProj);

	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


