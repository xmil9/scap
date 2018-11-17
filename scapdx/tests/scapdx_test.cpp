//
// Tests for scapdx.
//
// Dec-2016, Michael Lindner
//
#include "file_system.h"
#include "scap.h"
#include "libpng/util/libpng_util.h"
#include "libpng/util/pngutil.h"
#ifdef _MSC_VER
// Lower warning level for external headers.
#  pragma warning(push, 3)
#endif
#include <cassert>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iostream>
#include <string>
#include <vector>
#ifdef _MSC_VER
// Restore warning level.
#  pragma warning(pop)
#endif


namespace {

using pngutil::libpng::loadPng;
using pngutil::libpng::PngInfo;


// Error class for scapdx tests.
class ScapDxTestError : public std::runtime_error {
public:
   explicit ScapDxTestError(const std::string& msg) : std::runtime_error(msg) {
   }
};


// Loads PNG file into passed std::vector. Ensures the format of the loaded PNG
// image is BGRA.
PngInfo loadBGRAPngImage(std::string const& filepath,
   std::vector<png_byte>* buffer) {
   try {
      auto allocatePixelBuffer = [buffer](size_t dataSize) {
         buffer->resize(dataSize);
         return buffer->data();
      };

      PngInfo info = loadPng(filepath, PNG_FORMAT_BGRA, allocatePixelBuffer);
      if (info.format() != PNG_FORMAT_BGRA)
         throw ScapDxTestError("Unexpected PNG data format.");

      return info;
   } catch (pngutil::Error& ex) {
      throw ScapDxTestError(ex.what());
   }
}


inline bool isWhitePixel(png_byte const* pixel) {
   return (pixel[0] == 255 && pixel[1] == 255 && pixel[2] == 255);
}


inline bool isBlackPixel(png_byte const* pixel) {
   return (pixel[0] == 0 && pixel[1] == 0 && pixel[2] == 0);
}


inline bool isBlankPngPixel(png_byte const* pixel) {
   return (isWhitePixel(pixel) || isBlackPixel(pixel));
}


// Checks that passed file is not a completely black or completely white images.
bool isBlankPngImage(std::string const& filepath) {
   std::vector<png_byte> pixelBuffer;
   PngInfo pngInfo = loadBGRAPngImage(filepath, &pixelBuffer);

   const std::size_t kBytesPerPixel = 4;
   png_byte* pixel = pixelBuffer.data();
   png_byte const* const pixelEnd = pixelBuffer.data() + pixelBuffer.size();
   assert(pixelBuffer.size() % kBytesPerPixel == 0);
   while (pixel < pixelEnd) {
      if (!isBlankPngPixel(pixel))
         return false;
      pixel += kBytesPerPixel;
   }

   return true;
}

} // namespace


namespace tests {

///////////////////

// Collection of test data.
struct SavingPngFileDataSet {
   std::string filename;
};


// Data sets for test cases.
const SavingPngFileDataSet savingPngFileTestData[] = {
   {"capture1.png"},
};

// Execute 'saving png file' test for a given data set.
bool testSavingPngFile(std::string const& testDataDir,
   SavingPngFileDataSet const& data) {
   const std::string outFilepath{testDataDir + data.filename};
   scap::captureScreen(outFilepath);

   const bool doesOutfileExists = filesys::fileExists(outFilepath);
   const size_t fileSize = filesys::calcFileSize(outFilepath);
   const bool isBlankImage = isBlankPngImage(outFilepath);

   // Delete the output file.
   std::remove(outFilepath.c_str());

   return doesOutfileExists && (fileSize > 0) && !isBlankImage;
}


// Execute 'saving png file' test for all data sets.
// Returns number of failed tests.
size_t runSavingPngFileTests(std::string const& testDataDir) {
   size_t numFailed{0};
   for (auto& data : savingPngFileTestData) {
      try {
         if (!testSavingPngFile(testDataDir, data))
            ++numFailed;
      } catch (...) {
         ++numFailed;
      }
   }
   return numFailed;
}


///////////////////

// A test unit is a function that executes related test cases and a description
// of the tested functionality.
struct TestUnit {
   std::function<size_t(std::string const&)> test;
   std::string description;
};


// All units to be tested.
const std::vector<TestUnit> testUnits = {
   {runSavingPngFileTests, "Saving captured screen as PNG file"},
};


void runTests(std::string const& testDataDir) {
   size_t totalCasesFailed{0};

   for (auto const& unit : testUnits) {
      size_t numFailed = unit.test(testDataDir);
      if (numFailed > 0) {
         std::cout << "Test unit '" << unit.description << "' failed."
            << std::endl;
         totalCasesFailed += numFailed;
      }
   }

   if (totalCasesFailed == 0)
      std::cout << "scapdx tests SUCCEEDED." << std::endl;
   else
      std::cout << "scapdx tests FAILED. Failed test cases: "
      << std::to_string(totalCasesFailed) << std::endl;
}

} // namespace tests


int main(int argc, char** argv) {
   if (argc != 2) {
      std::cout << "Usage error. Arguments missing." << std::endl;
      return EXIT_FAILURE;
   }

   tests::runTests(filesys::normalizePath(argv[1]));

   std::cout << "Press ENTER to finish." << std::endl;
   getchar();
   return EXIT_SUCCESS;
}
