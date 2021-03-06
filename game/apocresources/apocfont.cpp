#include "library/sp.h"

#include "game/apocresources/apocfont.h"
#include "framework/framework.h"
#include "framework/image.h"
#include "framework/palette.h"

#include <boost/locale.hpp>

namespace OpenApoc
{

sp<ApocalypseFont> ApocalypseFont::loadFont(tinyxml2::XMLElement *fontElement)
{
	int spacewidth = 0;
	int height = 0;
	int width = 0;
	UString fileName;
	UString fontName;

	const char *attr = fontElement->Attribute("name");
	if (!attr)
	{
		LogError("apocfont element with no \"name\" attribute");
		return nullptr;
	}
	fontName = attr;
	attr = fontElement->Attribute("path");
	if (!attr)
	{
		LogError("apocfont \"%s\" with no \"path\" attribute", fontName.c_str());
		return nullptr;
	}
	fileName = attr;

	auto err = fontElement->QueryIntAttribute("height", &height);
	if (err != tinyxml2::XML_NO_ERROR || height <= 0)
	{
		LogError("apocfont \"%s\" with invalid \"height\" attribute", fontName.c_str());
		return nullptr;
	}
	err = fontElement->QueryIntAttribute("width", &width);
	if (err != tinyxml2::XML_NO_ERROR || width <= 0)
	{
		LogError("apocfont \"%s\" with invalid \"width\" attribute", fontName.c_str());
		return nullptr;
	}
	err = fontElement->QueryIntAttribute("spacewidth", &spacewidth);
	if (err != tinyxml2::XML_NO_ERROR || spacewidth <= 0)
	{
		LogError("apocfont \"%s\" with invalid \"spacewidth\" attribute", fontName.c_str());
		return nullptr;
	}

	auto file = fw().data->fs.open(fileName);
	if (!file)
	{
		LogError("apocfont \"%s\" - Failed to open font path \"%s\"", fontName.c_str(),
		         fileName.c_str());
		return nullptr;
	}

	auto fileSize = file.size();
	int glyphSize = height * width;
	int glyphCount = fileSize / glyphSize;

	if (!glyphCount)
	{
		LogError("apocfont \"%s\" - file \"%s\" contains no glyphs", fontName.c_str(),
		         fileName.c_str());
	}
	sp<ApocalypseFont> font(new ApocalypseFont);

	font->name = fontName;
	font->spacewidth = spacewidth;
	font->fontheight = height;
	font->averagecharacterwidth = 0;
	font->palette = fw().data->load_palette(fontElement->Attribute("palette"));

	for (auto *glyphNode = fontElement->FirstChildElement(); glyphNode;
	     glyphNode = glyphNode->NextSiblingElement())
	{
		UString nodeName = glyphNode->Name();
		if (nodeName != "glyph")
		{
			LogError("apocfont \"%s\" has unexpected child node \"%s\", skipping", fontName.c_str(),
			         nodeName.c_str());
			continue;
		}
		int offset;
		err = glyphNode->QueryIntAttribute("offset", &offset);
		if (err != tinyxml2::XML_NO_ERROR)
		{
			LogError(
			    "apocfont \"%s\" has glyph with invalid/missing offset attribute - skipping glyph",
			    fontName.c_str());
			continue;
		}
		attr = glyphNode->Attribute("string");
		if (!attr)
		{
			LogError("apocfont \"%s\" has glyph with missing string attribute - skipping glyph",
			         fontName.c_str());
			continue;
		}

		auto pointString = boost::locale::conv::utf_to_utf<UniChar>(attr);

		if (pointString.length() != 1)
		{
			LogError("apocfont \"%s\" glyph w/offset %d has %d codepoints, expected one - skipping "
			         "glyph",
			         fontName.c_str(), offset, pointString.length());
			continue;
		}
		if (offset >= glyphCount)
		{
			LogError("apocfont \"%s\" glyph \"%s\" has invalid offset %d - file contains a max of "
			         "%d - skipping glyph",
			         fontName.c_str(), attr, offset, glyphCount);
			continue;
		}

		UniChar c = pointString[0];

		if (font->fontbitmaps.find(c) != font->fontbitmaps.end())
		{
			LogError(
			    "apocfont \"%s\" glyph \"%s\" has multiple definitions - skipping re-definition",
			    fontName.c_str(), attr);
			continue;
		}
		file.seekg(glyphSize * offset, std::ios::beg);
		int glyphWidth = 0;

		auto glyphImage = mksp<PaletteImage>(Vec2<int>(width, height));
		{
			PaletteImageLock imgLock(glyphImage, ImageLockUse::Write);

			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x++)
				{
					uint8_t idx;
					file.read(reinterpret_cast<char *>(&idx), 1);
					imgLock.set(Vec2<int>{x, y}, idx);
					if (idx != 0 && glyphWidth < x)
						glyphWidth = x;
				}
			}
		}
		auto trimmedGlyph = mksp<PaletteImage>(Vec2<int>{glyphWidth + 2, height});
		PaletteImage::blit(glyphImage, Vec2<int>{0, 0}, trimmedGlyph);
		font->fontbitmaps[c] = trimmedGlyph;

		font->averagecharacterwidth += glyphWidth + 2;
	}

	// FIXME: Bit of a hack to handle spaces?
	auto spaceImage = mksp<PaletteImage>(Vec2<int>{spacewidth, height});
	// Defaults to transparent (0)
	font->fontbitmaps[UString::u8Char(' ')] = spaceImage;

	font->averagecharacterwidth /= font->fontbitmaps.size();

	return font;
}

sp<PaletteImage> ApocalypseFont::getGlyph(UniChar codepoint)
{
	if (fontbitmaps.find(codepoint) == fontbitmaps.end())
	{
		// FIXME: Hack - assume all missing glyphs are spaces
		// TODO: Fallback fonts?
		LogWarning("Font %s missing glyph for character \"%s\" (codepoint %u)",
		           this->getName().c_str(), UString(codepoint).c_str(), codepoint);
		auto missingGlyph = this->getGlyph(UString::u8Char(' '));
		fontbitmaps.emplace(codepoint, missingGlyph);
	}
	return fontbitmaps[codepoint];
}

ApocalypseFont::~ApocalypseFont() {}

int ApocalypseFont::GetFontHeight() { return fontheight; }

UString ApocalypseFont::getName() { return this->name; }

int ApocalypseFont::GetEstimateCharacters(int FitInWidth)
{
	return FitInWidth / averagecharacterwidth;
}

sp<Palette> ApocalypseFont::getPalette() { return this->palette; }

}; // namespace OpenApoc
