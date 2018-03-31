//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

#include "XSDXShader.h"
#include "XSDXState.h"
#include "XSDXBuffer.h"

class Silhouette
{
public:
	enum RenderMode
	{
		RENDER_GS,
		RENDER_TESS,
		RENDER_STYLIZED
	};

	enum VertexShaderID : uint32_t
	{
		VS_BASEPASS
	};

	enum HullShaderID : uint32_t
	{
		HS_SILHOUETTE,
		HS_SILHOUETTE_TESS
	};

	enum DomainShaderID : uint32_t
	{
		DS_SILHOUETTE
	};

	enum GeometryShaderID : uint32_t
	{
		GS_SILHOUETTE,
		GS_PARTICLE
	};

	enum PixelShaderID : uint32_t
	{
		PS_SIMPLE,
		PS_GAUSS
	};

	Silhouette(const XSDX::CPDXDevice &pDXDevice, const XSDX::spShader &pShader, const XSDX::spState &pState);
	virtual ~Silhouette();

	void Init(const uint32_t uWidth, const uint32_t uHeight, const char *szFileName = "Media\\bunny.obj");
	void UpdateFrame(DirectX::CXMVECTOR vEyePt, DirectX::CXMMATRIX mViewProj);
	void Render(RenderMode renderMode = RENDER_GS);

	static void CreateVertexLayout(const XSDX::CPDXDevice &pDXDevice, XSDX::CPDXInputLayout &pVertexLayout,
		const XSDX::spShader &pShader, const uint8_t uVS);

	static XSDX::CPDXInputLayout &GetVertexLayout();

protected:
	struct CBMatrices
	{
		DirectX::XMMATRIX mWorldViewProj;
		DirectX::XMMATRIX mWorld;
		DirectX::XMMATRIX mWorldIT;
	};

	struct CBPerFrame
	{
		DirectX::XMFLOAT4 vEyePos;
	};

	void createVB(const uint32_t uNumVert, const uint32_t uStride, const uint8_t *pData);
	void createIB(const uint32_t uNumIndices, const uint32_t *pData);
	void createCBs();
	void renderDepth();
	void renderGS();
	void renderTess();
	void renderStylized();

	CBMatrices						m_cbMatrices;
	CBPerFrame						m_cbPerFrame;
	uint32_t						m_uVertexStride;
	uint32_t						m_uNumIndices;

	XSDX::CPDXBuffer				m_pVB;
	XSDX::CPDXBuffer				m_pIB;
	XSDX::CPDXBuffer				m_pCBMatrices;
	XSDX::CPDXBuffer				m_pCBPerFrame;

	XSDX::spShader					m_pShader;
	XSDX::spState					m_pState;

	XSDX::CPDXDevice				m_pDXDevice;
	XSDX::CPDXContext				m_pDXContext;

	static XSDX::CPDXInputLayout	m_pVertexLayout;
};

using upSilhouette = std::unique_ptr<Silhouette>;
using spSilhouette = std::shared_ptr<Silhouette>;
using vuSilhouette = std::vector<upSilhouette>;
using vpSilhouette = std::vector<spSilhouette>;
