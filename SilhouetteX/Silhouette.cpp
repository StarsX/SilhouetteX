//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#include "ObjLoader.h"
#include "Silhouette.h"

using namespace DirectX;
using namespace DX;
using namespace std;
using namespace XSDX;

CPDXInputLayout	Silhouette::m_pVertexLayout;

Silhouette::Silhouette(const CPDXDevice &pDXDevice, const spShader &pShader, const spState &pState) :
	m_pDXDevice(pDXDevice),
	m_pShader(pShader),
	m_pState(pState),
	m_uVertexStride(0),
	m_uNumIndices(0)
{
	m_pDXDevice->GetImmediateContext(&m_pDXContext);
}

Silhouette::~Silhouette()
{
}

void Silhouette::Init(const uint32_t uWidth, const uint32_t uHeight, const char *szFileName)
{
	ObjLoader objLoader;
	if (!objLoader.Import(szFileName)) return;

	createVB(objLoader.GetNumVertices(), objLoader.GetVertexStride(), objLoader.GetVertices());
	createIB(objLoader.GetNumIndices(), objLoader.GetIndices());

	createCBs();
}

void Silhouette::UpdateFrame(DirectX::CXMVECTOR vEyePt, DirectX::CXMMATRIX mViewProj)
{
	m_cbMatrices.mWorld = XMMatrixIdentity();
	m_cbMatrices.mWorldViewProj = XMMatrixTranspose(mViewProj);
	m_cbMatrices.mWorldIT = XMMatrixIdentity();

	XMStoreFloat4(&m_cbPerFrame.vEyePos, vEyePt);

	if (m_pCBMatrices) m_pDXContext->UpdateSubresource(m_pCBMatrices.Get(), 0, nullptr, &m_cbMatrices, 0, 0);
	if (m_pCBPerFrame) m_pDXContext->UpdateSubresource(m_pCBPerFrame.Get(), 0, nullptr, &m_cbPerFrame, 0, 0);
}

void Silhouette::Render(RenderMode renderMode)
{
	renderDepth();

	m_pDXContext->OMSetDepthStencilState(m_pState->DepthRead().Get(), 0);

	switch (renderMode)
	{
	case RENDER_TESS:
		renderTess();
		break;
	case RENDER_STYLIZED:
		renderStylized();
		break;
	default:
		renderGS();
	}

	m_pDXContext->OMSetDepthStencilState(nullptr, 0);
}

void Silhouette::CreateVertexLayout(const CPDXDevice &pDXDevice, CPDXInputLayout &pVertexLayout, const spShader &pShader, const uint8_t uVS)
{
	// Define our vertex data layout for skinned objects
	const auto offset = D3D11_APPEND_ALIGNED_ELEMENT;
	const auto vLayout = vector<D3D11_INPUT_ELEMENT_DESC>
	{
		{ "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, 0,		D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL",		0, DXGI_FORMAT_R32G32B32_FLOAT,	0, offset,	D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ThrowIfFailed(pDXDevice->CreateInputLayout(vLayout.data(), uint32_t(vLayout.size()),
		pShader->GetVertexShaderBuffer(uVS)->GetBufferPointer(),
		pShader->GetVertexShaderBuffer(uVS)->GetBufferSize(),
		&pVertexLayout));
}

CPDXInputLayout &Silhouette::GetVertexLayout()
{
	return m_pVertexLayout;
}

void Silhouette::createVB(const uint32_t uNumVert, const uint32_t uStride, const uint8_t *pData)
{
	m_uVertexStride = uStride;
	const auto desc = CD3D11_BUFFER_DESC(uStride * uNumVert, D3D11_BIND_VERTEX_BUFFER);
	D3D11_SUBRESOURCE_DATA ssd = { pData };
	
	ThrowIfFailed(m_pDXDevice->CreateBuffer(&desc, &ssd, &m_pVB));
}

void Silhouette::createIB(const uint32_t uNumIndices, const uint32_t *pData)
{
	m_uNumIndices = uNumIndices;
	const auto desc = CD3D11_BUFFER_DESC(sizeof(uint32_t) * uNumIndices, D3D11_BIND_INDEX_BUFFER);
	D3D11_SUBRESOURCE_DATA ssd = { pData };

	ThrowIfFailed(m_pDXDevice->CreateBuffer(&desc, &ssd, &m_pIB));
}

void Silhouette::createCBs()
{
	auto desc = CD3D11_BUFFER_DESC(sizeof(CBMatrices), D3D11_BIND_CONSTANT_BUFFER);
	ThrowIfFailed(m_pDXDevice->CreateBuffer(&desc, nullptr, &m_pCBMatrices));

	desc.ByteWidth = sizeof(CBPerFrame);
	ThrowIfFailed(m_pDXDevice->CreateBuffer(&desc, nullptr, &m_pCBPerFrame));
}

void Silhouette::renderDepth()
{
	const auto uOffset = 0u;
	const LPDXBuffer cbs[] = { m_pCBMatrices.Get(), m_pCBPerFrame.Get() };
	m_pDXContext->VSSetConstantBuffers(0, 2, cbs);

	m_pDXContext->IASetInputLayout(m_pVertexLayout.Get());
	m_pDXContext->IASetVertexBuffers(0, 1, m_pVB.GetAddressOf(), &m_uVertexStride, &uOffset);
	m_pDXContext->IASetIndexBuffer(m_pIB.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_pDXContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_pDXContext->VSSetShader(m_pShader->GetVertexShader(VS_BASEPASS).Get(), nullptr, 0);
	m_pDXContext->PSSetShader(nullptr, nullptr, 0);

	m_pDXContext->DrawIndexed(m_uNumIndices, 0, 0);
}

void Silhouette::renderGS()
{
	const auto uOffset = 0u;
	const LPDXBuffer cbs[] = { m_pCBMatrices.Get(), m_pCBPerFrame.Get() };
	m_pDXContext->VSSetConstantBuffers(0, 2, cbs);

	m_pDXContext->IASetInputLayout(m_pVertexLayout.Get());
	m_pDXContext->IASetVertexBuffers(0, 1, m_pVB.GetAddressOf(), &m_uVertexStride, &uOffset);
	m_pDXContext->IASetIndexBuffer(m_pIB.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_pDXContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_pDXContext->VSSetShader(m_pShader->GetVertexShader(VS_BASEPASS).Get(), nullptr, 0);
	m_pDXContext->GSSetShader(m_pShader->GetGeometryShader(GS_SILHOUETTE).Get(), nullptr, 0);
	m_pDXContext->PSSetShader(m_pShader->GetPixelShader(PS_SIMPLE).Get(), nullptr, 0);

	m_pDXContext->DrawIndexed(m_uNumIndices, 0, 0);

	m_pDXContext->GSSetShader(nullptr, nullptr, 0);
}

void Silhouette::renderTess()
{
	const auto uOffset = 0u;
	const LPDXBuffer cbs[] = { m_pCBMatrices.Get(), m_pCBPerFrame.Get() };
	m_pDXContext->VSSetConstantBuffers(0, 2, cbs);

	m_pDXContext->IASetInputLayout(m_pVertexLayout.Get());
	m_pDXContext->IASetVertexBuffers(0, 1, m_pVB.GetAddressOf(), &m_uVertexStride, &uOffset);
	m_pDXContext->IASetIndexBuffer(m_pIB.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_pDXContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	m_pDXContext->VSSetShader(m_pShader->GetVertexShader(VS_BASEPASS).Get(), nullptr, 0);
	m_pDXContext->HSSetShader(m_pShader->GetHullShader(HS_SILHOUETTE).Get(), nullptr, 0);
	m_pDXContext->DSSetShader(m_pShader->GetDomainShader(DS_SILHOUETTE).Get(), nullptr, 0);
	m_pDXContext->PSSetShader(m_pShader->GetPixelShader(PS_SIMPLE).Get(), nullptr, 0);

	m_pDXContext->DrawIndexed(m_uNumIndices, 0, 0);

	m_pDXContext->DSSetShader(nullptr, nullptr, 0);
	m_pDXContext->HSSetShader(nullptr, nullptr, 0);
}

void Silhouette::renderStylized()
{
	const auto uOffset = 0u;
	const LPDXBuffer cbs[] = { m_pCBMatrices.Get(), m_pCBPerFrame.Get() };
	m_pDXContext->VSSetConstantBuffers(0, 2, cbs);

	m_pDXContext->IASetInputLayout(m_pVertexLayout.Get());
	m_pDXContext->IASetVertexBuffers(0, 1, m_pVB.GetAddressOf(), &m_uVertexStride, &uOffset);
	m_pDXContext->IASetIndexBuffer(m_pIB.Get(), DXGI_FORMAT_R32_UINT, 0);
	m_pDXContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

	m_pDXContext->VSSetShader(m_pShader->GetVertexShader(VS_BASEPASS).Get(), nullptr, 0);
	m_pDXContext->HSSetShader(m_pShader->GetHullShader(HS_SILHOUETTE_TESS).Get(), nullptr, 0);
	m_pDXContext->DSSetShader(m_pShader->GetDomainShader(DS_SILHOUETTE).Get(), nullptr, 0);
	m_pDXContext->GSSetShader(m_pShader->GetGeometryShader(GS_PARTICLE).Get(), nullptr, 0);
	m_pDXContext->PSSetShader(m_pShader->GetPixelShader(PS_GAUSS).Get(), nullptr, 0);

	m_pDXContext->OMSetBlendState(m_pState->NonPremultiplied().Get(), nullptr, D3D11_DEFAULT_SAMPLE_MASK);
	m_pDXContext->DrawIndexed(m_uNumIndices, 0, 0);
	m_pDXContext->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

	m_pDXContext->GSSetShader(nullptr, nullptr, 0);
	m_pDXContext->DSSetShader(nullptr, nullptr, 0);
	m_pDXContext->HSSetShader(nullptr, nullptr, 0);
}
