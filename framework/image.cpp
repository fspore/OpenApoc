#include "library/sp.h"
#include "framework/image.h"
#include "framework/palette.h"
#include "framework/logger.h"

// Use physfs for RGBImage::saveBitmap directory creation
#include <physfs.h>
#include <SDL_surface.h>

namespace OpenApoc
{

static bool ReadUse(ImageLockUse use)
{
	return (use == ImageLockUse::Read || use == ImageLockUse::ReadWrite);
}

static bool WriteUse(ImageLockUse use)
{
	return (use == ImageLockUse::Write || use == ImageLockUse::ReadWrite);
}

Image::~Image() {}

Image::Image(Vec2<unsigned int> size)
    : size(size), dirty(true), bounds(0, 0, size.x, size.y), indexInSet(0)
{
}

Surface::Surface(Vec2<unsigned int> size) : Image(size) {}

Surface::~Surface() {}

PaletteImage::PaletteImage(Vec2<unsigned int> size, uint8_t initialIndex)
    : Image(size), indices(new uint8_t[size.x * size.y])
{
	for (unsigned int i = 0; i < size.x * size.y; i++)
		this->indices[i] = initialIndex;
}

PaletteImage::~PaletteImage() {}

sp<RGBImage> PaletteImage::toRGBImage(sp<Palette> p)
{
	sp<RGBImage> i = mksp<RGBImage>(size);

	RGBImageLock imgLock{i, ImageLockUse::Write};

	for (unsigned int y = 0; y < this->size.y; y++)
	{
		for (unsigned int x = 0; x < this->size.x; x++)
		{
			uint8_t idx = this->indices[y * this->size.x + x];
			imgLock.set(Vec2<unsigned int>{x, y}, p->GetColour(idx));
		}
	}
	return i;
}

void PaletteImage::blit(sp<PaletteImage> src, Vec2<unsigned int> offset, sp<PaletteImage> dst)
{
	PaletteImageLock reader(src, ImageLockUse::Read);
	PaletteImageLock writer(dst, ImageLockUse::Write);

	for (unsigned int y = 0; y < src->size.y; y++)
	{
		for (unsigned int x = 0; x < src->size.x; x++)
		{
			Vec2<unsigned int> readPos{x, y};
			Vec2<unsigned int> writePos{readPos + offset};
			if (writePos.x >= dst->size.x || writePos.y >= dst->size.y)
				break;
			writer.set(writePos, reader.get(readPos));
		}
	}
}

RGBImage::RGBImage(Vec2<unsigned int> size, Colour initialColour)
    : Image(size), pixels(new Colour[size.x * size.y])
{
	for (unsigned int i = 0; i < size.x * size.y; i++)
		this->pixels[i] = initialColour;
}

void RGBImage::saveBitmap(const UString &filename)
{
	// TODO: Check file's path exists
	std::vector<UString> segs = filename.split('/');
	UString workingdir("");

	for (unsigned int pidx = 0; segs.size() > 1 && pidx < segs.size() - 1; pidx++)
	{
		workingdir += segs.at(pidx);

		if (!PHYSFS_exists(workingdir.c_str()))
		{
			LogInfo("Building %s", workingdir.c_str());
			PHYSFS_mkdir(workingdir.c_str());
		}
		if (workingdir.substr(workingdir.length() - 1, 1) != "/")
		{
			workingdir += "/";
		}
	}

	SDL_Surface *bmp =
	    SDL_CreateRGBSurface(0, size.x, size.y, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	SDL_LockSurface(bmp);

	for (unsigned int y = 0; y < size.y; y++)
	{
		for (unsigned int x = 0; x < size.x; x++)
		{
			int offset = (y * bmp->pitch) + (x * 4);
			uint8_t *bytedata = reinterpret_cast<uint8_t *>(bmp->pixels);
			Colour_ARGB8888LE *pxdata = reinterpret_cast<Colour_ARGB8888LE *>(bytedata + offset);
			Colour c = pixels[(y * size.x) + x];

			pxdata->r = c.r;
			pxdata->g = c.g;
			pxdata->b = c.b;
			pxdata->a = c.a;
		}
	}

	SDL_UnlockSurface(bmp);
	SDL_SaveBMP(bmp, filename.c_str());
	SDL_FreeSurface(bmp);
}

RGBImage::~RGBImage() {}

RGBImageLock::RGBImageLock(sp<RGBImage> img, ImageLockUse use) : img(img), use(use)
{
	// FIXME: Readback from renderer?
	// FIXME: Disallow multiple locks?
}

RGBImageLock::~RGBImageLock() {}

Colour RGBImageLock::get(Vec2<unsigned int> pos)
{
	if (!ReadUse(this->use))
	{
		LogError("Trying to get() on image lock without specifying 'read' use");
		return 0;
	}
	unsigned offset = pos.y * this->img->size.x + pos.x;
	if (pos.x >= this->img->size.x || pos.y >= this->img->size.y)
	{
		LogError("Getting {%u,%u} in image of size {%u,%u}", pos.x, pos.y, this->img->size.x,
		         this->img->size.y);
	}
	assert(offset < this->img->size.x * this->img->size.y);
	return this->img->pixels[offset];
}

void RGBImageLock::set(Vec2<unsigned int> pos, Colour c)
{
	if (!WriteUse(this->use))
	{
		LogError("Trying to set() on image lock without specifying 'write' use");
		return;
	}
	unsigned offset = pos.y * this->img->size.x + pos.x;
	if (pos.x >= this->img->size.x || pos.y >= this->img->size.y)
	{
		LogError("Setting {%u,%u} in image of size {%u,%u}", pos.x, pos.y, this->img->size.x,
		         this->img->size.y);
	}
	assert(offset < this->img->size.x * this->img->size.y);
	this->img->pixels[offset] = c;
}

void *RGBImageLock::getData() { return this->img->pixels.get(); }

PaletteImageLock::PaletteImageLock(sp<PaletteImage> img, ImageLockUse use) : img(img), use(use)
{
	// FIXME: Readback from renderer?
	// FIXME: Disallow multiple locks?
}

PaletteImageLock::~PaletteImageLock() {}

uint8_t PaletteImageLock::get(Vec2<unsigned int> pos)
{
	if (!ReadUse(this->use))
	{
		LogError("Trying to get() on image lock without specifying 'read' use");
		return 0;
	}
	unsigned offset = pos.y * this->img->size.x + pos.x;
	if (pos.x >= this->img->size.x || pos.y >= this->img->size.y)
	{
		LogError("Getting {%u,%u} in image of size {%u,%u}", pos.x, pos.y, this->img->size.x,
		         this->img->size.y);
	}
	assert(offset < this->img->size.x * this->img->size.y);
	return this->img->indices[offset];
}

void PaletteImageLock::set(Vec2<unsigned int> pos, uint8_t idx)
{
	if (!WriteUse(this->use))
	{
		LogError("Trying to set() on image lock without specifying 'write' use");
		return;
	}
	unsigned offset = pos.y * this->img->size.x + pos.x;
	if (pos.x >= this->img->size.x || pos.y >= this->img->size.y)
	{
		LogError("Setting {%u,%u} in image of size {%u,%u}", pos.x, pos.y, this->img->size.x,
		         this->img->size.y);
	}
	assert(offset < this->img->size.x * this->img->size.y);
	this->img->indices[offset] = idx;
}

void *PaletteImageLock::getData() { return this->img->indices.get(); }

void PaletteImage::CalculateBounds()
{
	unsigned int minX = this->size.x, minY = this->size.y, maxX = 0, maxY = 0;

	for (unsigned int y = 0; y < this->size.y; y++)
	{
		for (unsigned int x = 0; x < this->size.x; x++)
		{
			unsigned int offset = y * this->size.x + x;
			if (this->indices[offset])
			{
				if (minX > x)
					minX = x;
				if (minY > y)
					minY = y;
				if (maxX < x)
					maxX = x;
				if (maxY < y)
					maxY = y;
			}
		}
	}
	this->bounds = {minX, minY, maxX, maxY};
}

}; // namespace OpenApoc
