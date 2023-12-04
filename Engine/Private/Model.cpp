#include "Model.h"
#include "Mesh.h"
#include "Texture.h"

CModel::CModel(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent(pDevice,pContext)
{
}

CModel::CModel(const CModel& rhs)
	: CComponent(rhs)
	, m_eModelType(rhs.m_eModelType)
	, m_iNumMeshes(rhs.m_iNumMeshes)
	, m_Meshes(rhs.m_Meshes)
	, m_iNumMaterials(rhs.m_iNumMaterials)
	, m_Materials(rhs.m_Materials)
{
	for (auto& MaterialDesc : m_Materials) //!  ���͸����ũ���� ����ü�� ����ִ� ���� ��ȸ
	{
		for(auto& pTexture : MaterialDesc.pMtrlTextures) 
			Safe_AddRef(pTexture); //!  ����ü���� 18�� ���� �������ִ� �迭�� �ؽ�ó ���۷��� ī��Ʈ ī��Ʈ �÷�����
	}
	
	for (auto& pMesh : m_Meshes)
	{
		Safe_AddRef(pMesh);
	}
}

HRESULT CModel::Initialize_Prototype(TYPE eType, const string& strModelFilePath, _fmatrix PivotMatrix)
{
	//TODO m_Importer ��ü�� �������ִ� ReadFile �Լ��� ȣ���Ѵ�
	
	//! ReadFile �Լ��� ���� ������ ����ִ� AiScene*�� �������ش�.
	//! ���� ���� : aiProcess_PreTransformVertices | aiProcess_GlobalScale
	
	//! aiProcess_PreTransformVertices == ������ �̸� ��ȯ��Ű�ڴ�. �̰� ����ϸ� �ִϸ��̼� �������ư���. �ݵ�� �ִԸ��� �ƴѰ�쿡�� �������
	//! ������ ���� ���� ��ǥ���¿��� ���� �����϶����� ������ �޽ÿ��� ����� �����ִ� �Ը´�. �� �ɼ��� �ָ� �ִ������� ���ư��� ó���ϱⰡ ��ٷο�����.
	//! �ִԸ��� ��쿡�� �� �ɼ��� �����ʰ� �츮�� ���� �۾�������. ����ü�� �������ٰ��̴�.
	
	//! aiProcess_GlobalScale == ������ �ε��ߴ� ���� ���� ���¿��� �������� ���� Ű��ٺ��� ������ �̻�������. 
	//! �׷��� �����̳� �е��� ���ʿ� 100�� �����ϸ� �س��� ���·� �Ѱ��ش�. �̹� ū �������� �۰� ����°� �� �� ���� �����ΰ�����.
	//! GlobalScale�� �ְԵȴٸ� �ε��Ҷ����� 0.01 �����ϸ��� ���ִµ�, Ȥ�� 100�� �����ϸ��� �ȵ��ִ� ������ �𵨵� ������ �ֱ� ������ ��õ���� �ʴ´�.

	//! aiProcess_ConvertToLeftHanded == �޼���ǥ�踦 ����Ҳ�
	//! aiProcessPreset_TargetRealtime_Fast == ����Ƽ�� ���� ���������� ���� ���� ����� �����ؼ� �о���ϰ�
	
	m_eModelType = eType;

	_uint iFlag = aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast;

	//!  ���ִϸ��̼��� ���¸��̸� PreTransformVertices �� ������
	if(TYPE_NONANIM == eType)
	iFlag |= aiProcess_PreTransformVertices;
	
	m_pAIScene = m_Importer.ReadFile(strModelFilePath, iFlag);

	if(nullptr == m_pAIScene)
		return E_FAIL;

	//#PivotMatrix
	//! �Ǻ���Ʈ������ �����̳ʵ��� ���� �츮�� ȯ�濡 �°� �Ϻ��ϰ� �����������ʴ´�. �׷��� 180�� ���ư��ִ� ���·� �ε��Ű�� ��� �ٸ��� ���� �ݴ븦 �ٶ󺸴� ���簡 �Ͼ��.
	//! �ε� ���Ĵ� �̹� ������ ������ ��ȯ�ȰͰ� ���������̴� �ε��Ҷ����� 180�� �������� �����ϱ� ���� �ִ� ���
	if(FAILED(Ready_Meshes(PivotMatrix)))
		return E_FAIL;

	//#������_Ready_Materials
	if(FAILED(Ready_Materials(strModelFilePath)))
		return E_FAIL;

	return S_OK;
}

HRESULT CModel::Initialize(void* pArg)
{

	return S_OK;
}

HRESULT CModel::Render(_uint iMeshIndex)
{
	//TODO �޽ú��� �׸��ٶ�� ������ �ϰ��ִ�.
	//! SHIFT + ALT + G #���͸𵨷��� 
	//!���� ���Ͷ�� ���� �����Լ��� ���캸��
	//! ���̴� ��� �Լ��� ������ �Ŀ� �޽ø� ��ȸ�ϸ鼭 ������ ������ �־���.
	//!  ������ �������� �޽��� ���͸��� ����ü�� �����ؼ� �ؽ�ó�� ���������� �޾ƿͼ� �׷����ϰ�, �������� �޽��� ��ǻ�� �ؽ�ó�� �����ɼ��ִ�.
	//! ��, �޽��� ������ ���� ���͸��� ����ü�� ������ �� �۰ų� ������ �ִ�.
	//! �ݴ�� �ϳ��� ����ü�� �������� �޽��� �� ǥ���ϱ� ���� �� �� ������ �������� ����ü�� �������� �ֵ��� ����̴�.
	//! �� �޽� �����ϱ�? == �޽ú��� �����ϰ��ִµ�, Ư���޽��� �׸��� ���� ���̴��ؽ�ó�� ��ü�Ѵٶ�� �̾߱�� �����ϴϱ� �Ϲ����� ��.
	 
	 //TODO ���� ���� �������Ҷ� �޽��� ��ؽ�ó, ����͸��� ����ü�� �̿��ϴ°��� ���� ������ ��Ȯ�� �ʿ��ϴ�.
	 //! �׷��� ���͸��� �ε����� ��������� ����.

	 //! �޽ð� � ������ ��������� ����Ѵ�. �޽ø� ��ȸ�ϸ鼭 �ؽ�ó�� ������ ������ ���� Ŭ���̾�Ʈ�� ������ ������ �ٲ���.
	//for (auto& pMesh : m_Meshes)
	//{
	//	if (nullptr != pMesh)
	//	{
	//		pMesh->Bind_VIBuffers();
	//		pMesh->Render();
	//	}
	//}

	if(iMeshIndex >= m_iNumMeshes)
		return E_FAIL;

	m_Meshes[iMeshIndex]->Bind_VIBuffers();
	m_Meshes[iMeshIndex]->Render();


	return S_OK;
}

HRESULT CModel::Bind_ShaderResource(CShader* pShader, const _char* pConstantName, _uint iMeshIndex, aiTextureType eTextureType)
{
	_uint		iMaterialIndex = m_Meshes[iMeshIndex]->Get_MaterialIndex();
	if(iMaterialIndex >= m_iNumMaterials)
		return E_FAIL;

	return m_Materials[iMaterialIndex].pMtrlTextures[eTextureType]->Bind_ShaderResource(pShader,pConstantName); //! �츮�� ��ó�� 1���̴�. textureIndex�� ���� �� �ʿ����.
}

HRESULT CModel::Ready_Meshes(_fmatrix PivotMatrix)
{
	m_iNumMeshes = m_pAIScene->mNumMeshes; //! �о���� �޽������� ��������� ä������

	m_Meshes.reserve(m_iNumMeshes); //! �޽� ������ �˰Ե����� ���͸� ����������

	for (size_t i = 0; i < m_iNumMeshes; i++)
	{
		CMesh*	pMesh = CMesh::Create(m_pDevice,m_pContext,m_pAIScene->mMeshes[i], PivotMatrix);

		if(nullptr == pMesh)
			return E_FAIL;

		m_Meshes.push_back(pMesh);
	}
	
	return S_OK;
}

HRESULT CModel::Ready_Materials(const string& strModelFilePath)
{
	m_iNumMaterials = m_pAIScene->mNumMaterials; //! AIScene���� ���� �о�� ���͸����� ������ ����.

	for (size_t i = 0; i < m_iNumMaterials; i++)
	{
		aiMaterial* pAIMaterial = m_pAIScene->mMaterials[i];

		MATERIAL_DESC	MaterialDesc = {}; //! ���� �޸�

		for (size_t j = 1; j < AI_TEXTURE_TYPE_MAX; j++)
		{
			//! �� �޽þȿ� ��ǻ�� ���͸����� ������ ���������ִ�. ������ 3�߷����� �������ϳ� ������ 1���� ����Ѵ�. ���� ���� �̻��ϴٸ� pAIMaterial->GetTextureCount �Լ��� ȣ���ؼ� Ȯ���غ���
			/* 
			for (size_t k = 0; k < pAIMaterial->GetTextureCount(aiTextureType(j)); k++)
			{
				pAIMaterial->GetTexture(aiTextureType(j), k, );
			}; */

			//TODO ������� ���� �ؽ�ó�� ��δ� ���ʿ� �����Ҷ� ����ߴ� �ؽ�ó�� ��η� �ν��Ѵ�. ���ϸ��� Ȯ���ڸ� ���� fbx �������� �ִ� ��ο� �ٿ��ְ� fbx �����ִ°��� ���� �̹��� ���ϵ��� �־�����
			_char	szDrive[MAX_PATH ] = "";
			_char	szDirectory[MAX_PATH] = "";

			_splitpath_s(strModelFilePath.c_str(), szDrive, MAX_PATH, szDirectory, MAX_PATH, nullptr, 0, nullptr, 0);

			aiString		strPath;

			//! GetTexture �Լ��� ���ϰ��� ���� aiReturn�ε� �����ϸ� 0 �ƴϸ� 0�� �ƴѰ��� �����ؼ� HRESULT ���İ� ���Ƽ� üũ �����ϴ�.
			if (FAILED(pAIMaterial->GetTexture(aiTextureType(j), 0, &strPath))) //!  ������� ������ �޾ƿ� �ؽ�ó�� �ε��� ��θ� strPath ������ ����
				continue;

			_char	szFileName[MAX_PATH] = "";
			_char	szEXT[MAX_PATH] = "";

			_splitpath_s(strPath.data, nullptr, 0, nullptr, 0, szFileName, MAX_PATH, szEXT, MAX_PATH);

			_char szTemp[MAX_PATH] = "";
			strcpy_s(szTemp, szDrive); //! �Ʊ� ������ ����̺� ��ι���.
			strcat_s(szTemp, szDirectory); //! ���� ��� �ٿ�����
			strcat_s(szTemp, szFileName); //! ���ϸ� �ٿ�����

			_char szTest[MAX_PATH] = ".dds";

			strcat_s(szTemp, szTest); //! Ȯ���� �ٿ�����

			_tchar szFullPath[MAX_PATH] = TEXT("");

			MultiByteToWideChar(CP_ACP, 0, szTemp, strlen(szTemp), szFullPath, MAX_PATH); //! TextureŬ������ �����ڰ����� wstring�� �ް��ֵ���. �ٲ�����

			MaterialDesc.pMtrlTextures[j] = CTexture::Create(m_pDevice,m_pContext,szFullPath, 1);
			if(nullptr == MaterialDesc.pMtrlTextures[j])
				return E_FAIL;
		}

		m_Materials.push_back(MaterialDesc);
	}

	return S_OK;
}



CModel* CModel::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, TYPE eType, const string& strModelFilePath, _fmatrix PivotMatrix)
{
	CModel* pInstance = new CModel(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(eType, strModelFilePath, PivotMatrix)))
	{
		MSG_BOX("Failed to Created : CModel");
		Safe_Release(pInstance);
	}
	return pInstance;
}

CComponent* CModel::Clone(void* pArg)
{
	CModel* pInstance = new CModel(*this);

	/* ������ü�� �ʱ�ȭ�Ѵ�.  */
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CModel");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CModel::Free()
{
	__super::Free();

	for (auto& MaterialDesc : m_Materials)
	{
		for (auto& pTexture : MaterialDesc.pMtrlTextures) //! �����迭�� Clear���� �ʿ� ����.
			Safe_Release(pTexture);
	}
	m_Materials.clear();

	for (auto& pMesh : m_Meshes)
	{
		Safe_Release(pMesh);
	}

	m_Meshes.clear();

	if(false == m_isCloned)
		m_Importer.FreeScene(); //! ������ü�϶��� �����Ϳ� ������ ȣ���ؼ� ����������
}