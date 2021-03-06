#pragma once
#include "library/sp.h"

#include <random>
#include <memory>
#include <vector>
#include "library/strings.h"
#include "game/organisation.h"
#include "game/base/base.h"
#include "game/rules/rules.h"

namespace OpenApoc
{

class City;
class Rules;

static const unsigned TICKS_PER_SECOND = 60;

class GameState
{
  private:
	sp<Organisation> player;
	Rules rules;

  public:
	GameState(const UString &rulesFileName);

	std::unique_ptr<City> city;

	std::map<UString, sp<Organisation>> organisations;
	std::vector<sp<Base>> playerBases;

	bool showTileOrigin;
	bool showVehiclePath;
	bool showSelectableBounds;

	std::default_random_engine rng;

	UString getPlayerBalance() const;
	sp<Organisation> getOrganisation(const UString &orgID);
	sp<Organisation> getPlayer() const;

	// The time from game start in ticks
	uint64_t time;

	Rules &getRules() { return this->rules; };
};

}; // namespace OpenApoc
