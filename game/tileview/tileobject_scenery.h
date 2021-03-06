#pragma once

#include "game/city/scenery.h"
#include "game/tileview/tileobject.h"

namespace OpenApoc
{

class TileObjectScenery : public TileObject
{
  public:
	void draw(Renderer &r, TileView &view, Vec2<float> screenPosition, TileViewMode mode) override;
	virtual ~TileObjectScenery();

	std::weak_ptr<Scenery> scenery;

	sp<Scenery> getOwner();

	sp<VoxelMap> getVoxelMap() override;

  private:
	friend class TileMap;
	TileObjectScenery(TileMap &map, sp<Scenery> scenery);
};

} // namespace OpenApoc
