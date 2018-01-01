//--------------------------------------------------------------------------------------
// By Stars XU Tianchen
//--------------------------------------------------------------------------------------

#pragma once

#include "XSDXState.h"
#include "XSDXShader.h"
#include "XSDXShaderCommon.h"

namespace XSDX
{
	class Buffer
	{
	public:
		Buffer(const CPDXDevice &pDXDevice);
		virtual ~Buffer(void);

		const CPDXShaderResourceView	&GetSRV() const;

		static void CreateReadBuffer(const CPDXDevice &pDXDevice,
			CPDXBuffer &pDstBuffer, const CPDXBuffer &pSrcBuffer);
	protected:
		CPDXShaderResourceView			m_pSRV;

		CPDXDevice						m_pDXDevice;
	};

	using upBuffer = std::unique_ptr<Buffer>;
	using spBuffer = std::shared_ptr<Buffer>;
	using vuBuffer = std::vector<upBuffer>;
	using vpBuffer = std::vector<spBuffer>;


	class Texture2D :
		public Buffer
	{
	public:
		Texture2D(const CPDXDevice &pDXDevice);
		void Create(const bool bUAV, const bool bDyn,
			const uint32_t uWidth, const uint32_t uHeight,
			const uint32_t uArraySize, const DXGI_FORMAT eFormat,
			const uint8_t uMips = 1ui8,
			const lpcvoid pInitialData = nullptr,
			const uint8_t uStride = sizeof(float));
		void Create(const bool bUAV, const bool bDyn,
			const uint32_t uWidth, const uint32_t uHeight,
			const DXGI_FORMAT eFormat, const uint8_t uMips = 1ui8,
			const lpcvoid pInitialData = nullptr,
			const uint8_t uStride = sizeof(float));
		void CreateSRV(const uint32_t uArraySize, const uint8_t uSamples = 1ui8);

		const CPDXTexture2D				&GetBuffer() const;
		const CPDXUnorderedAccessView	&GetUAV(const uint8_t i = 0ui8) const;
	protected:
		CPDXTexture2D					m_pTexture;
		vCPDXUAV						m_vpUAVs;
	};

	using upTexture2D = std::unique_ptr<Texture2D>;
	using spTexture2D = std::shared_ptr<Texture2D>;
	using vuTexture2D = std::vector<upTexture2D>;
	using vpTexture2D = std::vector<spTexture2D>;


	class RenderTarget :
		public Texture2D
	{
	public:
		RenderTarget(const CPDXDevice &pDXDevice);
		void Create(const uint32_t uWidth, const uint32_t uHeight, const uint32_t uArraySize,
			const DXGI_FORMAT eFormat, const uint8_t uSamples = 1ui8, const uint8_t uMips = 1ui8);
		void Create(const uint32_t uWidth, const uint32_t uHeight, const DXGI_FORMAT eFormat,
			const uint8_t uSamples = 1ui8, const uint8_t uMips = 1ui8);
		void CreateArray(const uint32_t uWidth, const uint32_t uHeight, const uint32_t uArraySize,
			const DXGI_FORMAT eFormat, const uint8_t uSamples = 1ui8, const uint8_t uMips = 1ui8);
		void Populate(const CPDXShaderResourceView &pSRVSrc, const spShader &pShader,
			const uint8_t uSRVSlot = 0ui8, const uint8_t uSlice = 0ui8, const uint8_t uMip = 0ui8);

		const CPDXRenderTargetView		&GetRTV(const uint8_t uSlice = 0ui8,
											const uint8_t uMip = 0ui8) const;
		const uint8_t					GetArraySize() const;
		const uint8_t					GetNumMips(const uint8_t uSlice = 0ui8) const;
	protected:
		void create(const uint32_t uWidth, const uint32_t uHeight, const uint32_t uArraySize,
			const DXGI_FORMAT eFormat, const uint8_t uSamples, const uint8_t uMips);
		using vvCPDXRTV = std::vector<vCPDXRTV>;
		vvCPDXRTV						m_vvpRTVs;
	};

	using upRenderTarget = std::unique_ptr<RenderTarget>;
	using spRenderTarget = std::shared_ptr<RenderTarget>;
	using vuRenderTarget = std::vector<upRenderTarget>;
	using vpRenderTarget = std::vector<spRenderTarget>;


	class DepthStencil :
		public Texture2D
	{
	public:
		DepthStencil(const CPDXDevice &pDXDevice);
		void Create(const uint32_t uWidth, const uint32_t uHeight, const uint32_t uArraySize,
			const bool bRead, DXGI_FORMAT eFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
			const uint8_t uSamples = 1ui8, const uint8_t uMips = 1ui8);
		void Create(const uint32_t uWidth, const uint32_t uHeight, const bool bRead,
			DXGI_FORMAT eFormat = DXGI_FORMAT_D24_UNORM_S8_UINT,
			const uint8_t uSamples = 1ui8, const uint8_t uMips = 1ui8);

		const CPDXDepthStencilView		&GetDSV(const uint8_t uMip = 0ui8) const;
		const CPDXDepthStencilView		&GetDSVRO(const uint8_t uMip = 0ui8) const;
		const uint8_t					GetNumMips() const;
	protected:
		vCPDXDSV						m_vpDSVs;
		vCPDXDSV						m_vpDSVROs;
	};

	using upDepthStencil = std::unique_ptr<DepthStencil>;
	using spDepthStencil = std::shared_ptr<DepthStencil>;
	using vuDepthStencil = std::vector<upDepthStencil>;
	using vpDepthStencil = std::vector<spDepthStencil>;


	class Texture3D :
		public Buffer
	{
	public:
		Texture3D(const CPDXDevice &pDXDevice);
		void Create(const bool bUAV, const bool bDyn,
			const uint32_t uWidth, const uint32_t uHeight, const uint32_t uDepth,
			const DXGI_FORMAT eFormat, const uint8_t uMips = 1ui8,
			const lpcvoid pInitialData = nullptr,
			const uint8_t uStride = sizeof(float));

		const CPDXTexture3D				&GetBuffer() const;
		const CPDXUnorderedAccessView	&GetUAV(const uint8_t i = 0ui8) const;
	protected:
		CPDXTexture3D					m_pTexture;
		vCPDXUAV						m_vpUAVs;
	};

	using upTexture3D = std::unique_ptr<Texture3D>;
	using spTexture3D = std::shared_ptr<Texture3D>;
	using vuTexture3D = std::vector<upTexture3D>;
	using vpTexture3D = std::vector<spTexture3D>;


	class RawBuffer :
		public Buffer
	{
	public:
		RawBuffer(const CPDXDevice &pDXDevice);
		void Create(const bool bVB, const bool bSO, const bool bSRV,
			const bool bUAV, const bool bDyn, const uint32_t uByteWidth,
			const lpcvoid pInitialData = nullptr,
			const uint8_t uUAVFlags = D3D11_BUFFER_UAV_FLAG_RAW);
		void CreateSRV(const uint32_t uByteWidth);

		const CPDXBuffer				&GetBuffer() const;
		const CPDXUnorderedAccessView	&GetUAV() const;
	protected:
		CPDXBuffer						m_pBuffer;
		CPDXUnorderedAccessView			m_pUAV;
	};

	using upRawBuffer = std::unique_ptr<RawBuffer>;
	using spRawBuffer = std::shared_ptr<RawBuffer>;
	using vuRawBuffer = std::vector<upRawBuffer>;
	using vpRawBuffer = std::vector<spRawBuffer>;


	class TypedBuffer :
	public RawBuffer
	{
	public:
		TypedBuffer(const CPDXDevice &pDXDevice);
		void Create(const bool bUAV, const bool bDyn, const uint32_t uNumElement,
			const uint32_t uStride, const DXGI_FORMAT eFormat,
			const lpcvoid pInitialData = nullptr, const uint8_t uUAVFlags = 0ui8);
		void CreateSRV(const uint32_t uNumElement, const DXGI_FORMAT eFormat);
	};

	using upTypedBuffer = std::unique_ptr<TypedBuffer>;
	using spTypedBuffer = std::shared_ptr<TypedBuffer>;
	using vuTypedBuffer = std::vector<upTypedBuffer>;
	using vpTypedBuffer = std::vector<spTypedBuffer>;


	class StructuredBuffer :
		public RawBuffer
	{
	public:
		StructuredBuffer(const CPDXDevice &pDXDevice);
		void Create(const bool bUAV, const bool bDyn,
			const uint32_t uNumElement, const uint32_t uStride,
			const lpcvoid pInitialData = nullptr,
			const uint8_t uUAVFlags = 0ui8);
		void CreateSRV(const uint32_t uNumElement);
	};

	using upStructuredBuffer = std::unique_ptr<StructuredBuffer>;
	using spStructuredBuffer = std::shared_ptr<StructuredBuffer>;
	using vuStructuredBuffer = std::vector<upStructuredBuffer>;
	using vpStructuredBuffer = std::vector<spStructuredBuffer>;
}
