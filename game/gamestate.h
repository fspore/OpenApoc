#pragma once

#include "framework/includes.h"
#include "game/rules/rules.h"
#include "game/organisation.h"

namespace OpenApoc {

class City;

class GameState {
public:
	GameState(Framework &fw, Rules &rules);
	std::unique_ptr<City> city;

	std::vector<Organisation> organisations;
};

}; //namespace OpenApoc
