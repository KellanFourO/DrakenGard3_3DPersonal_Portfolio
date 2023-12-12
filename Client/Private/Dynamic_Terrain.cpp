#include "stdafx.h"
#include "Dynamic_Terrain.h"
#include "GameInstance.h"


CDynamic_Terrain::CDynamic_Terrain(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	:CGameObject(pDevice, pContext)
{

}

CDynamic_Terrain::CDynamic_Terrain(const CDynamic_Terrain& rhs)
	: CGameObject(rhs)
	, m_eCurrentLevelID(rhs.m_eCurrentLevelID)
{
}

HRESULT CDynamic_Terrain::Initialize_Prototype(LEVEL eLevel)
{
	//TODO 원형객체의 초기화과정을 수행한다.
	/* 1.서버로부터 값을 받아와서 초기화한다 .*/
	/* 2.파일입출력을 통해 값을 받아온다.*/

	m_eCurrentLevelID = eLevel;

	return S_OK;
}

HRESULT CDynamic_Terrain::Initialize(void* pArg)
{

	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	/* 지형 정보 세팅 */
	/*
		1. 받은 데이터를 캐스팅을 통해 DINFO구조체로 바꿔서 담아주고, 이 구조체의 값을 또 다른 그릇에 담아서 함수에 넘겨주자.
				* 이 방식을 거치지않고 그냥 주소로 넘겨줬을 땐, 값이 제대로 안넘어갔음 *
	*/
	// 구조체 캐스팅
	DINFO* pInfo = (DINFO*)pArg;

	// 캐스팅된 구조체의 값 담기
	DINFO Info = { pInfo->fX, pInfo->fY, pInfo->fZ };

	// 주소가 아닌, 값이 담긴 구조체 넘겨주기
	ReceiveInfo(Info);

	if (FAILED(Ready_Components()))
		return E_FAIL;

	return S_OK;
}

void CDynamic_Terrain::Priority_Tick(_float fTimeDelta)
{
}

void CDynamic_Terrain::Tick(_float fTimeDelta)
{
	//m_pShaderCom->Bind_RawValue("g_vBrushPos", &m_fPickingPos, sizeof(_float4));
}

void CDynamic_Terrain::Late_Tick(_float fTimeDelta)
{
	if (FAILED(m_pGameInstance->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this)))
		return;
}

HRESULT CDynamic_Terrain::Render()
{
	//TODO 셰이더에게 행렬을 던져주는 행위는 반드시 셰이더의 비긴함수를 호출하기 이전에 해야한다.
	//! 그 이유는, 셰이더의 비긴함수 내에서 pPass->Apply(0, m_pContext); 코드를 수행한다.
	//! Apply 호출 후에 행렬을 던져줘도 에러는 나지 않지만, 안정성이 떨어진다.
	//! Apply 호출 후에 행렬을 던져주면, 어떤 때에는 정상적으로 수행되고, 어떤 때에는 값이 제대로 안 넘어가는 경우가 있다.
	if (FAILED(Bind_ShaderResources()))
		return E_FAIL;

	//! 이 셰이더에 0번째 패스로 그릴거야.
	m_pShaderCom->Begin(0);

	if (m_pVIBufferCom != nullptr) // 지형 생성할때 삭제하고 만들기전에 랜더를 타면 터지기때문에, 예외처리 해두자.
	{
		//! 내가 그리려고 하는 정점, 인덱스 버퍼를 장치에 바인딩해
		m_pVIBufferCom->Bind_VIBuffers();

		//! 바인딩된 정점, 인덱스를 그려
		m_pVIBufferCom->Render();
	}

	return S_OK;
}

void CDynamic_Terrain::Write_Json(json& Out_Json)
{
	__super::Write_Json(Out_Json);

	//Out_Json.emplace("Texture", TEXT("Prototype_Component_Texture_Terrain"));
	//Out_Json.emplace("Buffer", TEXT("Prototype_Component_VIBuffer_Dynamic_Terrain"));
	//Out_Json.emplace("Shader", TEXT("Prototype_Component_Shader_VtxNorTex"));
	Out_Json.emplace("SizeX", m_tDynamicInfo.fX);
	Out_Json.emplace("SizeY", m_tDynamicInfo.fY);
	Out_Json.emplace("SizeZ", m_tDynamicInfo.fZ);
}

void CDynamic_Terrain::Load_FromJson(const json& In_Json)
{
	__super::Load_FromJson(In_Json);

	m_tDynamicInfo.fX = In_Json["SizeX"];
	m_tDynamicInfo.fY = In_Json["SizeY"];
	m_tDynamicInfo.fZ = In_Json["SizeZ"];

	Delete_Component(TEXT("Com_VIBuffer"));

	if (FAILED(m_pGameInstance->Add_CloneObject(LEVEL_TOOL, TEXT("Layer_BackGround"), TEXT("Prototype_GameObject_Dynamic_Terrain"), &m_tDynamicInfo)))
		return;

	//__super::Add_Component(LEVEL_GAMEPLAY,In_Json["Texture"], TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom), nullptr);
	
	_int i = 0;

}


void CDynamic_Terrain::Picking_Terrain(EDIT_MODE eMode)
{
	if (nullptr == m_pVIBufferCom)
	{
		MSG_BOX("Picking_Terrain Buffer is nullptr");
		return;
	}
		
	//_bool bInputZ = m_pGameInstance->Key_Pressing(DIK_Z);
	//_bool bInputX = m_pGameInstance->Key_Pressing(DIK_X);
	//
	//if(!bInputZ && !bInputX)
	//	return;
	_uint WinCX, WinCY;
	WinCX = g_iWinSizeX; WinCY = g_iWinSizeY;

	RAY WorldRay = m_pGameInstance->Get_Ray(WinCX, WinCY);


	if (m_pVIBufferCom->Compute_MousePos(WorldRay,m_pTransformCom->Get_WorldMatrix(), &m_fPickingPos))
	{
		if(m_pGameInstance->Mouse_Down(DIM_LB))
		m_pVIBufferCom->Update(XMLoadFloat3(&m_fPickingPos), m_fDrawRadious, m_fPower, (_uint)eMode);
	}
	
}

_float3 CDynamic_Terrain::GetMousePos()
{
	if (nullptr == m_pVIBufferCom)
	{
		MSG_BOX("Picking_Terrain Buffer is nullptr");
		return _float3();
	}
	_uint WinCX, WinCY;
	WinCX = g_iWinSizeX; WinCY = g_iWinSizeY;

	RAY WorldRay = m_pGameInstance->Get_Ray(WinCX, WinCY);


	if (m_pVIBufferCom->Compute_MousePos(WorldRay, m_pTransformCom->Get_WorldMatrix(), &m_fPickingPos))
	{
		return m_fPickingPos;
	}

	return _float3();
}

_bool CDynamic_Terrain::MouseOnTerrain()
{
	if (nullptr == m_pVIBufferCom)
	{
		MSG_BOX("Picking_Terrain Buffer is nullptr");
		return false;
	}

	_uint WinCX, WinCY;
	WinCX = g_iWinSizeX; WinCY = g_iWinSizeY;

	RAY WorldRay = m_pGameInstance->Get_Ray(WinCX, WinCY);

	if (m_pVIBufferCom->Compute_MousePos(WorldRay, m_pTransformCom->Get_WorldMatrix(), &m_fPickingPos))
	{
		return true;
	}

	return false;

	
}

HRESULT CDynamic_Terrain::Ready_Components()
{
	//! For.Com_Shader
	if (FAILED(__super::Add_Component(m_eCurrentLevelID, TEXT("Prototype_Component_Shader_VtxNorTex"),
		TEXT("Com_Shader"), reinterpret_cast<CComponent**>(&m_pShaderCom))))
		return E_FAIL;

	//! For.Com_VIBuffer_Dynamic
	if (FAILED(__super::Add_Component(m_eCurrentLevelID, TEXT("Prototype_Component_VIBuffer_Dynamic_Terrain"),	// 생성까지 됐음
		TEXT("Com_VIBuffer"), reinterpret_cast<CComponent**>(&m_pVIBufferCom), &m_tDynamicInfo)))
		return E_FAIL;

	//! For.Com_Texture
	if (FAILED(__super::Add_Component(m_eCurrentLevelID, TEXT("Prototype_Component_Texture_Terrain"),
		TEXT("Com_Texture"), reinterpret_cast<CComponent**>(&m_pTextureCom))))
		return E_FAIL;

	/* For.Com_Mask */ // 지형을 섞기위한 Mask // 에러 : 툴 레벨로 ID를 주지않아서 터레인 생성에 실패했었고, 제대로 동작이 안되니, Imgui shutdown에 들어가 터졌음
	if (FAILED(__super::Add_Component(m_eCurrentLevelID, TEXT("Prototype_Component_Texture_Terrain_Mask"),
		TEXT("Com_Mask"), reinterpret_cast<CComponent**>(&m_pTextureCom[TYPE_MASK]))))
		return E_FAIL;

	if (FAILED(__super::Add_Component(m_eCurrentLevelID, TEXT("Prototype_Component_Texture_Terrain_Brush"),
		TEXT("Com_Brush"), reinterpret_cast<CComponent**>(&m_pTextureCom[TYPE_BRUSH]))))
		return E_FAIL;

	

	return S_OK;
}

HRESULT CDynamic_Terrain::Bind_ShaderResources()
{

	if (FAILED(m_pTransformCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &m_pGameInstance->Get_TransformFloat4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	// 디퓨즈
	if (FAILED(m_pTextureCom[TYPE_DIFFUSE]->Bind_ShaderResourceArray(m_pShaderCom, "g_DiffuseTexture")))
		return E_FAIL;

	// 마스크
	if (FAILED(m_pTextureCom[TYPE_MASK]->Bind_ShaderResource(m_pShaderCom, "g_MaskTexture", 0)))
		return E_FAIL;

	//// 브러쉬
	if (FAILED(m_pTextureCom[TYPE_BRUSH]->Bind_ShaderResource(m_pShaderCom, "g_BrushTexture", 0))) // error : 
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vCamPosition", &m_pGameInstance->Get_CamPosition(), sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_vBrushPos", &m_fPickingPos, sizeof(_float4))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_RawValue("g_fBrushRange", &m_fDrawRadious, sizeof(float))))
		return E_FAIL;
	//if (FAILED(m_pTextureCom->Bind_ShaderResource(m_pShaderCom, "g_DiffuseTexture")))	// 수정
	//	return E_FAIL;

	return S_OK;
}

void CDynamic_Terrain::ReceiveInfo(DINFO pInfo)
{
	m_tDynamicInfo = { pInfo.fX, pInfo.fY, pInfo.fZ };
}

void CDynamic_Terrain::Delete_Component(const wstring& strComTag)
{
	__super::Delete_Component(strComTag);

	// ! 삭제가 안될때 !
	// Delete_Component에서는 복사된 녀석을 Safe_Release로 래퍼런스 카운트를 줄이기만 한것이라,
	// 제대로 타는지 확인하고 여기서 멤버변수도 삭제 시켜줘야한다.

	if(m_pVIBufferCom)
	Safe_Release(m_pVIBufferCom);
	

}

CDynamic_Terrain* CDynamic_Terrain::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, LEVEL eLevel)
{
	CDynamic_Terrain* pInstance = new CDynamic_Terrain(pDevice, pContext);

	/* 원형 객체를 초기화한다. */
	if (FAILED(pInstance->Initialize_Prototype(eLevel)))
	{
		MSG_BOX("Failed to Created : CDynamic_Terrain");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CGameObject* CDynamic_Terrain::Clone(void* pArg) // 여기서 사본을 만들때 지형을 설정할 값을 받아준다.
{
	CDynamic_Terrain* pInstance = new CDynamic_Terrain(*this);

	/* 사본 객체를 초기화한다. */
	if (FAILED(pInstance->Initialize(pArg))) // 넘겨줌
	{
		MSG_BOX("Failed to Cloned : CDynamic_Terrain");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CDynamic_Terrain::Free()
{
	__super::Free();

	
	Safe_Release(m_pVIBufferCom);

	Safe_Release(m_pShaderCom);

	//Safe_Release(m_pTextureCom);
	// 배열 형식, ->Release 에러 : 텍스처 컴포넌트를 배열 형식으로 바꿨기 때문에, 하나하나 삭제 시켜주도록 바꿔줘야한다.
	for (size_t i = 0; i < TYPE_END; i++)
	{
		Safe_Release(m_pTextureCom[i]);
	}
}
