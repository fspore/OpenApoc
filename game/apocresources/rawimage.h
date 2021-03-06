#pragma once
#include "library/sp.h"

#include "library/vec.h"
#include <memory>

namespace OpenApoc
{

class Data;
class UString;
class PaletteImage;
class ImageSet;

class RawImage
{
  public:
	static sp<PaletteImage> load(Data &data, const UString &fileName, const Vec2<int> &size);
	static sp<ImageSet> load_set(Data &data, const UString &fileName, const Vec2<int> &size);
};

}; // namespace OpenApoc
