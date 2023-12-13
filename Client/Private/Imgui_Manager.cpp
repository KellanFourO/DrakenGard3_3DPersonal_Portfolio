#pragma once
#include "stdafx.h"

#include "Tool_Define.h"
#include "Imgui_Manager.h"
#include "GameInstance.h"
#include "Dynamic_Terrain.h"
#include "Camera_MapTool.h"

#include <regex>
#include <codecvt>

ImGuiIO g_io;
static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE	   mCurrentGizmoMode(ImGuizmo::WORLD);
static bool useSnap(false);

IMPLEMENT_SINGLETON(CImgui_Manager);

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

	Ready_ProtoTagList();			//! 오브젝트 프로토태그 리스트 준비

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
	ImGuiFileDialog::Instance()->AddBookmark("MapData", "../Bin/DataFiles/Map/");
	ImGuiFileDialog::Instance()->AddBookmark("ObjectData", "../Bin/DataFiles/Object/");
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

			if (ImGui::Button(u8"저장하기 "))	{ m_eDialogMode = CImgui_Manager::DIALOG_SAVE; OpenDialog(m_eToolID); } ImGui::SameLine(); if (ImGui::Button(u8"불러오기 ")) { m_eDialogMode = CImgui_Manager::DIALOG_LOAD; OpenDialog(m_eToolID); }

			ImGui::InputFloat(u8"X 사이즈", &m_fTileX);
			ImGui::InputFloat(u8"Z 사이즈", &m_fTileZ);

			m_tMapInfo.vPosition.x = m_fTileX;
			m_tMapInfo.vPosition.y = 1.f;
			m_tMapInfo.vPosition.z = m_fTileZ;

			if (ImGui::Button(u8"생성"))
			{
				if (nullptr != m_pDynamic_Terrain)
				{
					m_pDynamic_Terrain->Delete_Component(TEXT("Com_VIBuffer"));
				}

				if (FAILED(m_pGameInstance->Add_CloneObject(LEVEL_TOOL, TEXT("Layer_BackGround"), TEXT("Prototype_GameObject_Dynamic_Terrain"), &m_tMapInfo, reinterpret_cast<CGameObject**>(&m_pDynamic_Terrain))))
					return;
			}

			if (nullptr != m_pDynamic_Terrain)
			{
				ImGui::Checkbox(u8"픽킹모드", &m_bMapToolPickMode);

				if (m_bMapToolPickMode)
				{
					ImGui::Text(u8"마우스 X : %f", m_pDynamic_Terrain->GetMousePos().x);
					ImGui::Text(u8"마우스 Y : %f", m_pDynamic_Terrain->GetMousePos().y);
					ImGui::Text(u8"마우스 Z : %f", m_pDynamic_Terrain->GetMousePos().z);

					if (ImGui::InputInt(u8"브러시 범위", &m_iBrushRange)) { m_pDynamic_Terrain->SetRadious(m_iBrushRange); }
					if (ImGui::InputInt(u8"브러시 힘", &m_iBrushPower)) { m_pDynamic_Terrain->SetPower(m_iBrushPower); }
					
					ImGui::NewLine(); //! 브러시 모드 라디오

					static int BrushIndex = 0;
					const char* BrushModeName[3] = { u8"다운", u8"업", u8"프레싱" };

					for (_uint i = 0; i < IM_ARRAYSIZE(BrushModeName); ++i)
					{
						if(i > 0) { ImGui::SameLine(); }
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

					if(ImGui_MouseInCheck())
					PickingTerrain(m_eBrushMode);

				}
			}

			ImGui::EndTabItem();
		}
}

void CImgui_Manager::ImGui_ObjectToolTick()
{

		if (ImGui::BeginTabItem(u8"오브젝트"))
		{
			m_eToolID = CImgui_Manager::TOOL_OBJECT;

			if (ImGui::Button(u8"저장하기")) { m_eDialogMode = CImgui_Manager::DIALOG_SAVE; OpenDialog(m_eToolID); } ImGui::SameLine(); if (ImGui::Button(u8"불러오기")) { m_eDialogMode = CImgui_Manager::DIALOG_LOAD; OpenDialog(m_eToolID); }

			if (nullptr != m_pDynamic_Terrain)
			{
				ImGui::Text(u8"마우스 X : %f", m_pDynamic_Terrain->GetMousePos().x);
				ImGui::Text(u8"마우스 Y : %f", m_pDynamic_Terrain->GetMousePos().y);
				ImGui::Text(u8"마우스 Z : %f", m_pDynamic_Terrain->GetMousePos().z);

				ImGui::Checkbox(u8"픽킹모드", &m_bObjectToolPickMode);

				if(true == m_bObjectToolPickMode)
				{
					ImGui::RadioButton(u8"Create", &m_iObjectMode, 0); ImGui::SameLine(); ImGui::RadioButton(u8"Select", &m_iObjectMode, 1);

					if(0 == m_iObjectMode)
					{
						CreateObjectFunction();
					}

					else if(1 == m_iObjectMode)
					{
						SelectObjectFunction();
					}
				}
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

	if(m_eDialogMode == CImgui_Manager::DIALOG_SAVE)
		strAdd = u8" 저장";
	else if(m_eDialogMode == CImgui_Manager::DIALOG_LOAD)
		strAdd = u8" 불러오기";


	switch (eToolID)
	{
	case Client::CImgui_Manager::TOOL_MAP:
		strKey = "MapToolDialog";
		strTitle = u8"맵 다이얼로그" + strAdd;
		strPath = "../Bin/DafaFiles/Map/";
		
		break;
	case Client::CImgui_Manager::TOOL_OBJECT:
		strKey = "ObjectToolDialog";
		strTitle = u8"오브젝트 다이얼로그" + strAdd;
		strPath = "../Bin/DafaFiles/Object/";
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
							SaveObject(filePathName);
						break;
					}
						

					case Client::CImgui_Manager::DIALOG_LOAD:
					{
						if (m_eToolID == CImgui_Manager::TOOL_MAP)
							LoadMap(filePathName, fileName);

						else if (m_eToolID == CImgui_Manager::TOOL_OBJECT)
							LoadObject(filePathName);
						break;
					}
				}
			}
			m_pFileDialog->Close();
		}	
}

void CImgui_Manager::PickingTerrain(BRUSHMODE eBrushMode)
{
	if(eBrushMode == CImgui_Manager::BRUSH_END)
		return;

	switch (eBrushMode)
	{
		case Client::CImgui_Manager::BRUSH_DOWN: 
		{
			if (m_pGameInstance->Mouse_Down(DIM_LB))
			{
				m_pDynamic_Terrain->Picking_Terrain((CDynamic_Terrain::EDIT_MODE)m_iTileMode);
			}

			break;
		}
		
		case Client::CImgui_Manager::BRUSH_UP:
		{
			if (m_pGameInstance->Mouse_Up(DIM_LB))
			{
				m_pDynamic_Terrain->Picking_Terrain((CDynamic_Terrain::EDIT_MODE)m_iTileMode);
			}

			break;
		}

		case Client::CImgui_Manager::BRUSH_PRESSING:
		{
			if (m_pGameInstance->Mouse_Pressing(DIM_LB))
			{
				m_pDynamic_Terrain->Picking_Terrain((CDynamic_Terrain::EDIT_MODE)m_iTileMode);
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
	if(current != string::npos)
	{
		sub = strFileName.substr(index, current - index);
	}

	m_pDynamic_Terrain->Write_Json(OutJson, ConvertStrToWstr(sub));

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

HRESULT CImgui_Manager::Add_PrototypeTag(const wstring& strPrototypeTag)
{
	string pTag = ConvertWstrToStr(strPrototypeTag);

	m_vecObjectProtoTags.push_back(pTag);

	return S_OK;
}

HRESULT CImgui_Manager::Ready_ProtoTagList()
{
	vector<wstring> vecTags = m_pGameInstance->Get_VecTags();

	for (auto& strTag : vecTags)
	{
		Add_PrototypeTag(strTag);
	}

	return S_OK;
}

void CImgui_Manager::CreateObjectFunction()
{
	_int iObjectTagSize = m_vecObjectProtoTags.size();

	if(ImGui::BeginListBox(u8"태그 리스트"))
	{
		for(_uint i = 0; i < iObjectTagSize; ++i)
		{
			const _bool isSelected = (m_iSelectTagIndex == i);

			if(ImGui::Selectable(m_vecObjectProtoTags[i].c_str(), isSelected))
			{
				m_iSelectTagIndex = i;

				if(isSelected)
					ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndListBox();
	}

	if (m_pGameInstance->Mouse_Down(DIM_LB))
	{
		if(nullptr != m_pDynamic_Terrain)
			m_fPickingPos = m_pDynamic_Terrain->GetMousePos(); //! 마우스 클릭한 지점의 월드 좌표 받기
		else
		{
			MSG_BOX("터레인부터 생성해");
			return;
		}
		
		if (m_pDynamic_Terrain->MouseOnTerrain() && ImGui_MouseInCheck())
		{
			CGameObject* pGameObject = nullptr;

			wstring wstr = ConvertStrToWstr(m_vecObjectProtoTags[m_iSelectTagIndex]);

			if (FAILED(m_pGameInstance->Add_CloneObject(LEVEL_TOOL, TEXT("Layer_BackGround"), wstr, nullptr, reinterpret_cast<CGameObject**>(&pGameObject))))
				return;

			

			string SliceTag = ConvertWstrToStr(wstr);
			string IndexTag = to_string(m_vecObjects.size() + 1);
			SliceTag = SliceTag + IndexTag;

			m_vecCreateObjectTag.push_back(SliceTag);
			
			m_vecObjects.push_back(pGameObject);
			pGameObject->Get_Transform()->Set_State(CTransform::STATE_POSITION, XMVectorSet(m_fPickingPos.x, m_fPickingPos.y, m_fPickingPos.z, 1.f));
		}
		
	}

}

void CImgui_Manager::SelectObjectFunction()
{
	Set_GuizmoCamView(); //! 기즈모 뷰 투영 셋팅해주기.
	Set_GuizmoCamProj();	//! 기즈모 뷰 투영 셋팅해주기.

	if (nullptr != m_PickingObject)
		Set_Guizmo();	

	//_int iObjectListSize = m_vecCreateObjectTag.size();
	_int iObjectListSize = m_vecCreateObjectTag.size();
	if (ImGui::BeginListBox(u8""))
	{
		for (_int i = 0; i < iObjectListSize; ++i)
		{
			const _bool isSelected = (m_iPickingObjectIndex == i);

			if (ImGui::Selectable(m_vecCreateObjectTag[i].c_str(), isSelected))
			{
				m_PickingObject = m_vecObjects[i];
				m_iPickingObjectIndex = i;

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndListBox();
	}

	ImGui::NewLine();

	ImGui::Checkbox(u8"픽킹모드", &m_bObjectToolPickMode);

	if(nullptr != m_pDynamic_Terrain && false == m_bObjectToolPickMode)
		return;

	m_fPickingPos = m_pDynamic_Terrain->GetMousePos();

	if (m_pGameInstance->Mouse_Down(DIM_LB))
	{
		_int iObjectSize = m_vecObjects.size();
		
		for (_uint i = 0; i < iObjectSize; ++i)
		{
			if (m_vecObjects[i]->Picking(m_fPickingPos, dynamic_cast<CModel*>(m_vecObjects[i]->Find_Component(TEXT("Com_Model")))))
			{
				m_PickingObject = m_vecObjects[i];
				m_iPickingObjectIndex = i;
			}
		}
	}
}

void CImgui_Manager::SaveObject(string strFilePath)
{
}

void CImgui_Manager::LoadObject(string strFilePath)
{
}

string CImgui_Manager::ConvertWstrToStr(const wstring& str)
{
	wstring_convert<codecvt_utf8<_tchar>> converter;
	string ChangeStr = converter.to_bytes(str);
	return ChangeStr;
}

wstring CImgui_Manager::ConvertStrToWstr(const string& str)
{
	wstring_convert<codecvt_utf8<wchar_t>> converter;
	wstring wideStr = converter.from_bytes(str);
	return wideStr;
}

wstring CImgui_Manager::SliceObjectTag(const wstring& strObjectTag)
{
	size_t pos = strObjectTag.rfind(L"_");
	if (pos != wstring::npos)
	{
		return strObjectTag.substr(pos + 1);
	}
}

void CImgui_Manager::Free()
{
	Safe_Release(m_pGameInstance);
	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);

	m_vecObjectProtoTags.clear();
	m_vecCreateObjectTag.clear();

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


