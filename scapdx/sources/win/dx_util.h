//
// DirectX utilities.
//
//
// Jan-2017, Michael Lindner
//
#pragma once
// Lower warning level for external headers.
#pragma warning(push, 3)
#include <D3d11.h>
#include <DXGI1_2.h>
#include <windows.h>
#include <wrl\client.h>
#include <cstddef>
#include <string>
#include <tuple>
#include <vector>
// Restore warning level.
#pragma warning(pop)


namespace dx {

// Error class for COM errors.
struct Error : public std::runtime_error {
   explicit Error(HRESULT hr);
   explicit Error(const std::string& msg);
};


// Collection of values returnd when creating a DX device:
// D3D device, device context, and the acquired feature level.
using CreateDxDeviceResult =
   std::tuple<Microsoft::WRL::ComPtr<ID3D11Device>,
              Microsoft::WRL::ComPtr<ID3D11DeviceContext>, D3D_FEATURE_LEVEL>;
// Creates a D3D device for given driver types and feature levels.
// Passing null for feature levels will use the default feature levels.
CreateDxDeviceResult createDxDevice(
   std::vector<D3D_DRIVER_TYPE> const& acceptedDriverTypes,
   std::vector<D3D_FEATURE_LEVEL> const* acceptedFeatureLevel = nullptr);

// Helpers to access data related to devices, adapters, and outputs.
Microsoft::WRL::ComPtr<IDXGIDevice>
getDxgiDeviceFromD3DDevice(ID3D11Device* d3dDevice);

Microsoft::WRL::ComPtr<IDXGIAdapter>
getAdapterFromDevice(IDXGIDevice* device);

Microsoft::WRL::ComPtr<IDXGIFactory2>
getFactoryFromAdapter(IDXGIAdapter* adapter);

Microsoft::WRL::ComPtr<IDXGIOutput>
getPrimaryOuputOfAdapter(IDXGIAdapter* adapter);

// Create a 2D texture for use by CPU (aka staging texture).
// This textue can be used as destination in ID3D11DeviceContext::CopyResource()
// calls and then mapped to access the data for further processing by the
// CPU.
Microsoft::WRL::ComPtr<ID3D11Texture2D>
createDxTextureForCpu(ID3D11Device* device, std::size_t width,
                      std::size_t height, DXGI_FORMAT format);

// Get the surface of a texture.
Microsoft::WRL::ComPtr<IDXGISurface> getTextureSurface(ID3D11Texture2D* tex);


///////////////////

// RAII class to hold mapped surface data.
class MappedSurfaceData {
public:
   explicit MappedSurfaceData(Microsoft::WRL::ComPtr<IDXGISurface> surf);
   ~MappedSurfaceData();
   MappedSurfaceData(MappedSurfaceData const&) = delete;
   MappedSurfaceData(MappedSurfaceData&&) = default;
   MappedSurfaceData& operator=(MappedSurfaceData const&) = delete;
   MappedSurfaceData& operator=(MappedSurfaceData&&) = default;

   INT pitch() const;
   BYTE* bits() const;

private:
   Microsoft::WRL::ComPtr<IDXGISurface> m_surf;
   DXGI_MAPPED_RECT m_data;
};

} // namespace dx


#include "dx_util.inl.h"