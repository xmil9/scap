//
// Jan-2017, Michael Lindner
//
#include "stdafx.h"
#include "dx_util.h"
#include "string_util.h"
// Lower warning level for external headers.
#pragma warning(push, 3)
#include <comdef.h>
#include <cassert>
// Restore warning level.
#pragma warning(pop)

using Microsoft::WRL::ComPtr;


namespace {

// Returns error text for a HRESULT.
std::string comErrorText(HRESULT hr) {
   _com_error err(hr);
   return strutil::stringFromWString(err.ErrorMessage());
}

} // namespace


namespace dx {

///////////////////

Error::Error(HRESULT hr)
: std::runtime_error(comErrorText(hr)) {
}


///////////////////

CreateDxDeviceResult
createDxDevice(std::vector<D3D_DRIVER_TYPE> const& acceptedDriverTypes,
               std::vector<D3D_FEATURE_LEVEL> const* acceptedFeatureLevel) {
   constexpr IDXGIAdapter* kNoExistingAdapter = nullptr;
   constexpr HMODULE kNoSoftwareModule = NULL;

   D3D_FEATURE_LEVEL const* const acceptedLevels =
      acceptedFeatureLevel ? acceptedFeatureLevel->data() : nullptr;
   const UINT numAcceptedLevels =
      acceptedFeatureLevel ? static_cast<UINT>(acceptedFeatureLevel->size())
                           : 0;

   HRESULT hr = S_OK;
   ComPtr<ID3D11Device> d3dDevice;
   D3D_FEATURE_LEVEL acquiredFeatureLevel = D3D_FEATURE_LEVEL_11_1;
   ComPtr<ID3D11DeviceContext> d3dContext;

   for (D3D_DRIVER_TYPE const& driverType : acceptedDriverTypes) {
      hr =
         D3D11CreateDevice(kNoExistingAdapter, driverType, kNoSoftwareModule, 0,
                           acceptedLevels, numAcceptedLevels, D3D11_SDK_VERSION,
                           &d3dDevice, &acquiredFeatureLevel, &d3dContext);
      if (SUCCEEDED(hr))
         break;
   }

   if (FAILED(hr))
      throw Error(hr);

   return std::make_tuple(d3dDevice, d3dContext, acquiredFeatureLevel);
}


ComPtr<IDXGIDevice> getDxgiDeviceFromD3DDevice(ID3D11Device* d3dDevice) {
   if (!d3dDevice)
      throw Error("Invalid arguments passed.");

   ComPtr<IDXGIDevice> dxgiDevice;
   HRESULT hr = d3dDevice->QueryInterface(
      __uuidof(IDXGIDevice),
      reinterpret_cast<void**>(dxgiDevice.GetAddressOf()));
   if (FAILED(hr))
      throw Error(hr);
   return dxgiDevice;
}


ComPtr<IDXGIAdapter> getAdapterFromDevice(IDXGIDevice* dxgiDevice) {
   if (!dxgiDevice)
      throw Error("Invalid arguments passed.");

   ComPtr<IDXGIAdapter> adapter;
   HRESULT hr = dxgiDevice->GetParent(
      __uuidof(IDXGIAdapter), reinterpret_cast<void**>(adapter.GetAddressOf()));
   if (FAILED(hr))
      throw Error(hr);
   return adapter;
}


ComPtr<IDXGIFactory2> getFactoryFromAdapter(IDXGIAdapter* adapter) {
   if (!adapter)
      throw Error("Invalid arguments passed.");

   ComPtr<IDXGIFactory2> dxgiFactory;
   HRESULT hr =
      adapter->GetParent(__uuidof(IDXGIFactory2),
                         reinterpret_cast<void**>(dxgiFactory.GetAddressOf()));
   if (FAILED(hr))
      throw Error(hr);
   return dxgiFactory;
}


ComPtr<IDXGIOutput> getPrimaryOuputOfAdapter(IDXGIAdapter* adapter) {
   if (!adapter)
      throw Error("Invalid arguments passed.");

   constexpr UINT kPrimaryOutputIdx = 0;
   ComPtr<IDXGIOutput> primaryOutput;
   HRESULT hr = adapter->EnumOutputs(kPrimaryOutputIdx, &primaryOutput);
   if (FAILED(hr))
      throw Error(hr);
   return primaryOutput;
}


ComPtr<ID3D11Texture2D> createDxTextureForCpu(ID3D11Device* device,
                                              std::size_t width,
                                              std::size_t height,
                                              DXGI_FORMAT format) {
   if (!device)
      throw Error("Invalid arguments passed.");

   D3D11_TEXTURE2D_DESC texInfo;
   texInfo.Usage = D3D11_USAGE_STAGING;
   texInfo.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
   texInfo.Height = static_cast<UINT>(height);
   texInfo.Width = static_cast<UINT>(width);
   texInfo.Format = format;
   texInfo.BindFlags = 0;
   texInfo.MiscFlags = 0;
   texInfo.MipLevels = 1;
   texInfo.ArraySize = 1;
   texInfo.SampleDesc.Count = 1;
   texInfo.SampleDesc.Quality = 0;

   ComPtr<ID3D11Texture2D> tex;
   HRESULT hr = device->CreateTexture2D(&texInfo, NULL, &tex);
   if (FAILED(hr))
      throw Error(hr);

   return tex;
}


ComPtr<IDXGISurface> getTextureSurface(ID3D11Texture2D* tex) {
   ComPtr<IDXGISurface> surf;
   if (!tex)
      throw Error("Invalid arguments passed.");

   HRESULT hr = tex->QueryInterface(
      __uuidof(IDXGISurface), reinterpret_cast<void**>(surf.GetAddressOf()));
   if (FAILED(hr))
      throw Error(hr);
   return surf;
}


///////////////////

MappedSurfaceData::MappedSurfaceData(ComPtr<IDXGISurface> surf)
   : m_surf(surf) {
   if (!surf)
      throw Error("Invalid arguments passed.");

   ZeroMemory(&m_data, sizeof(DXGI_MAPPED_RECT));

   HRESULT hr = surf->Map(&m_data, DXGI_MAP_READ);
   if (FAILED(hr))
   {
      m_surf = nullptr;
      throw Error(hr);
   }
}

} // namespace dx