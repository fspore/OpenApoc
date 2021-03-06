
#pragma once
#include "library/sp.h"

#include "framework/includes.h"

namespace OpenApoc
{

class Palette;
class Event;
class Image;

class ApocCursor
{

  private:
	std::vector<sp<Image>> images;
	Vec2<int> cursorPos;

  public:
	enum CursorType
	{
		Normal,
		ThrowTarget,
		PsiTarget,
		NoTarget,
		Add,
		Shoot,
		Control,
		Teleport,
		NoTeleport
	};

	CursorType CurrentType;

	const Vec2<int> &getPosition() { return cursorPos; }

	ApocCursor(sp<Palette> ColourPalette);
	~ApocCursor();

	void EventOccured(Event *e);
	void Render();
};
}; // namespace OpenApoc
