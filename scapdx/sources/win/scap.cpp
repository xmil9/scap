//
// Dec-2016, Michael Lindner
//
#include "stdafx.h"
#include "scap.h"
#include "dx_util.h"
#include "libpng/util/libpng_util.h"
// Lower warning level for external headers.
#pragma warning(push, 3)
#include <D3d11.h>
#include <DXGI1_2.h>
#include <wrl\client.h>
#include <cassert>
#include <tuple>
#include <vector>
// Restore warning level.
#pragma warning(pop)

using namespace dx;
using Microsoft::WRL::ComPtr;


namespace {

///////////////////

// Returns the output duplication API for a given output.
ComPtr<IDXGIOutputDuplication>
getOutputDuplicationApi(IDXGIOutput* targetOutput, ID3D11Device* d3dDevice) {
   assert(targetOutput && d3dDevice);

   ComPtr<IDXGIOutput1> output1Api;
   HRESULT hr =
      targetOutput->QueryInterface(__uuidof(IDXGIOutput1), (void**)&output1Api);
   if (FAILED(hr))
      throw dx::Error(hr);

   ComPtr<IDXGIOutputDuplication> duplicationApi;
   hr = output1Api->DuplicateOutput(d3dDevice, &duplicationApi);
   if (FAILED(hr))
      throw dx::Error(hr);

   return duplicationApi;
}


// Acquires the next frame of a given output.
ComPtr<IDXGIResource> acquireOutputFrame(IDXGIOutputDuplication* duplicationApi) {
   assert(duplicationApi);

   HRESULT hr = S_OK;
   DXGI_OUTDUPL_FRAME_INFO duplicatedFrameInfo;
   ZeroMemory(&duplicatedFrameInfo, sizeof(DXGI_OUTDUPL_FRAME_INFO));
   ComPtr<IDXGIResource> frame;

   // Try to acquire a frame until we get one.
   while (duplicatedFrameInfo.AccumulatedFrames == 0) {
      // Release previous frame, if any.
      duplicationApi->ReleaseFrame();
      hr = duplicationApi->AcquireNextFrame(INFINITE, &duplicatedFrameInfo,
                                            &frame);
      if (FAILED(hr))
         throw dx::Error(hr);
   }

   return frame;
}


// RAII class to release acquired desktop frames.
class ScopedFrameRelease {
public:
   explicit ScopedFrameRelease(ComPtr<IDXGIOutputDuplication> duplicationApi);
   ~ScopedFrameRelease();
   ScopedFrameRelease(ScopedFrameRelease const&) = delete;
   ScopedFrameRelease(ScopedFrameRelease&&) = default;
   ScopedFrameRelease& operator=(ScopedFrameRelease const&) = delete;
   ScopedFrameRelease& operator=(ScopedFrameRelease&&) = default;

private:
   ComPtr<IDXGIOutputDuplication> m_duplicationApi;
};

inline ScopedFrameRelease::ScopedFrameRelease(
   ComPtr<IDXGIOutputDuplication> duplicationApi)
: m_duplicationApi(duplicationApi) {
}

inline ScopedFrameRelease::~ScopedFrameRelease() {
   if (m_duplicationApi)
      m_duplicationApi->ReleaseFrame();
}


// Copies a given resource to a new texture object.
ComPtr<ID3D11Texture2D> copyAsTexture(IDXGIResource* resource,
                                      ID3D11Device* device,
                                      ID3D11DeviceContext* context) {
   assert(resource && device && context);

   ComPtr<ID3D11Texture2D> tex;
   HRESULT hr = resource->QueryInterface(
      __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(tex.GetAddressOf()));
   if (FAILED(hr))
      throw dx::Error(hr);

   D3D11_TEXTURE2D_DESC texInfo;
   tex->GetDesc(&texInfo);

   ComPtr<ID3D11Texture2D> copiedTex = createDxTextureForCpu(
      device, texInfo.Width, texInfo.Height, DXGI_FORMAT_B8G8R8A8_UNORM);
   context->CopyResource(copiedTex.Get(), tex.Get());
   return copiedTex;
}


// Captures an image of the given output (monitor).
ComPtr<ID3D11Texture2D> captureOutputImage(IDXGIOutput* output,
                                           ID3D11Device* device,
                                           ID3D11DeviceContext* context) {
   assert(output && device && context);

   ComPtr<IDXGIOutputDuplication> duplicationApi =
      getOutputDuplicationApi(output, device);

   ComPtr<IDXGIResource> screenFrame = acquireOutputFrame(duplicationApi.Get());
   // Make sure that the acquired frame gets released for any return situation.
   ScopedFrameRelease autoFrameRelease(duplicationApi);

   return copyAsTexture(screenFrame.Get(), device, context);
}


///////////////////

png_uint_32 pngFormatFromDxgiFormat(DXGI_FORMAT dxgiFormat) {
#pragma warning(push)
#pragma warning(disable: 4061)

   switch (dxgiFormat) {
      case DXGI_FORMAT_B8G8R8A8_UNORM:
         return PNG_FORMAT_BGRA;
      case DXGI_FORMAT_R8G8B8A8_UNORM:
         return PNG_FORMAT_RGBA;
      default:
         throw scap::Error("Unsupported DXGI texture format.");
   }

#pragma warning(pop)
}


// Returns the size in bytes of a block of RGBA data with given dimensions.
std::size_t calcTextureDataSize(D3D11_TEXTURE2D_DESC const& texInfo) {
#pragma warning(push)
#pragma warning(disable: 4061)

   std::size_t bytesPerSample = 0;

   switch (texInfo.Format) {
      case DXGI_FORMAT_B8G8R8A8_UNORM:
      case DXGI_FORMAT_R8G8B8A8_UNORM:
         bytesPerSample = 4;
         break;
      default:
         throw scap::Error("Unsupported DXGI texture format.");
   }

   return bytesPerSample * texInfo.Width * texInfo.Height;

#pragma warning(pop)
}


void saveAsPng(ID3D11Texture2D* tex, std::string const& file) {
   assert(tex);

   D3D11_TEXTURE2D_DESC texInfo;
   tex->GetDesc(&texInfo);
   ComPtr<IDXGISurface> surf = getTextureSurface(tex);
   MappedSurfaceData surfData(surf);

   pngutil::libpng::PngInfo imageInfo(pngFormatFromDxgiFormat(texInfo.Format),
                                      texInfo.Width, texInfo.Height);
   pngutil::libpng::savePng(file, imageInfo, surfData.bits(),
                            calcTextureDataSize(texInfo));
}


///////////////////

void captureScreenDx(std::string const& outFilePath) {
   ComPtr<ID3D11Device> d3dDevice;
   ComPtr<ID3D11DeviceContext> d3dContext;
   std::tie(d3dDevice, d3dContext, std::ignore) =
      dx::createDxDevice({D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP,
         D3D_DRIVER_TYPE_REFERENCE});

   ComPtr<IDXGIDevice> dxgiDevice = getDxgiDeviceFromD3DDevice(d3dDevice.Get());
   ComPtr<IDXGIAdapter> adapter = getAdapterFromDevice(dxgiDevice.Get());
   ComPtr<IDXGIOutput> output  = getPrimaryOuputOfAdapter(adapter.Get());

   ComPtr<ID3D11Texture2D> screenTex =
      captureOutputImage(output.Get(), d3dDevice.Get(), d3dContext.Get());

   saveAsPng(screenTex.Get(), outFilePath);
}

} // namespace


namespace scap {

SCAP_API void captureScreen(std::string const& outFilePath) {
   if (outFilePath.empty())
      throw scap::Error("Invalid arguments passed.");

   try {
      captureScreenDx(outFilePath);
   } catch (std::runtime_error& ex) {
      throw scap::Error(ex.what());
   }
}

} //namespace scap