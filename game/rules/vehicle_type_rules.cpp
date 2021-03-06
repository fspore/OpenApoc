#include "library/sp.h"
#include "game/rules/rules_private.h"
#include "library/strings.h"
#include "framework/framework.h"
#include "game/rules/rules_helper.h"
#include "game/tileview/voxel.h"

#include <glm/glm.hpp>
#include <map>

namespace OpenApoc
{

static std::map<UString, VehicleType::Direction> direction_map = {
    {"n", VehicleType::Direction::N}, {"ne", VehicleType::Direction::NE},
    {"e", VehicleType::Direction::E}, {"se", VehicleType::Direction::SE},
    {"s", VehicleType::Direction::S}, {"sw", VehicleType::Direction::SW},
    {"w", VehicleType::Direction::W}, {"nw", VehicleType::Direction::NW},

};

static std::map<UString, VehicleType::Banking> banking_map = {
    {"flat", VehicleType::Banking::Flat},
    {"left", VehicleType::Banking::Left},
    {"right", VehicleType::Banking::Right},
    {"ascending", VehicleType::Banking::Ascending},
    {"descending", VehicleType::Banking::Descending},
};

static std::map<UString, VehicleType::ArmourDirection> armour_map = {
    {"top", VehicleType::ArmourDirection::Top},
    {"bottom", VehicleType::ArmourDirection::Bottom},
    {"front", VehicleType::ArmourDirection::Front},
    {"rear", VehicleType::ArmourDirection::Rear},
    {"left", VehicleType::ArmourDirection::Left},
    {"right", VehicleType::ArmourDirection::Right},
};

static std::map<UString, VehicleType::AlignmentX> align_x_map = {
    {"left", VehicleType::AlignmentX::Left},
    {"centre", VehicleType::AlignmentX::Centre},
    {"right", VehicleType::AlignmentX::Right},
};

static std::map<UString, VehicleType::AlignmentY> align_y_map = {
    {"top", VehicleType::AlignmentY::Top},
    {"centre", VehicleType::AlignmentY::Centre},
    {"bottom", VehicleType::AlignmentY::Bottom},
};

static std::map<UString, VEquipmentType::Type> equipment_slot_type_map = {
    {"engine", VEquipmentType::Type::Engine},
    {"weapon", VEquipmentType::Type::Weapon},
    {"general", VEquipmentType::Type::General},
};

static bool ParseVehicleStratmapSprites(tinyxml2::XMLElement *parentNode, VehicleType &vehicle)
{
	std::map<VehicleType::Direction, UString> sprites;

	for (tinyxml2::XMLElement *node = parentNode->FirstChildElement(); node != nullptr;
	     node = node->NextSiblingElement())
	{
		UString node_name = node->Name();
		if (node_name == "sprite")
		{
			VehicleType::Direction direction;
			if (!ReadAttribute(node, "direction", direction_map, direction))
			{
				LogError(
				    "Invalid direction \"%s\" in stratmap 'direction' attribute on vehicle_type "
				    "ID \"%s\"",
				    node->Attribute("direction"), vehicle.id.c_str());
				return false;
			}
			UString stratmap_path;
			if (!ReadElement(node, stratmap_path))
			{
				LogError("Stratmap direction \"%s\" for vehicle_type ID \"%s\" missing sprite path",
				         node->Attribute("direction"), vehicle.id.c_str());
				return false;
			}
			sprites[direction] = stratmap_path;
			// FIXME: Read 'offset' attribute to handle larger-than-1x1 stratmap sprites
			continue;
		}
		else
		{
			LogError("Unexpected node \"%s\" in stratmap section for vehicle_type ID \"%s\"",
			         node_name.c_str(), vehicle.id.c_str());
			return false;
		}
	}

	vehicle.strategy_sprite_paths = sprites;
	return true;
}

static bool ParseVehicleShadowSprites(tinyxml2::XMLElement *parentNode, VehicleType &vehicle)
{
	std::map<VehicleType::Direction, UString> sprites;

	for (tinyxml2::XMLElement *node = parentNode->FirstChildElement(); node != nullptr;
	     node = node->NextSiblingElement())
	{
		UString node_name = node->Name();
		if (node_name == "sprite")
		{
			VehicleType::Direction direction;
			if (!ReadAttribute(node, "direction", direction_map, direction))
			{
				LogError("Invalid direction \"%s\" in shadow 'direction' attribute on vehicle_type "
				         "ID \"%s\"",
				         node->Attribute("direction"), vehicle.id.c_str());
				return false;
			}
			UString stratmap_path;
			if (!ReadElement(node, stratmap_path))
			{
				LogError("Shadow direction \"%s\" for vehicle_type ID \"%s\" missing sprite path",
				         node->Attribute("direction"), vehicle.id.c_str());
				return false;
			}
			sprites[direction] = stratmap_path;
			// FIXME: Read 'offset' attribute to handle larger-than-1x1 stratmap sprites
			continue;
		}
		else
		{
			LogError("Unexpected node \"%s\" in stratmap section for vehicle_type ID \"%s\"",
			         node_name.c_str(), vehicle.id.c_str());
			return false;
		}
	}

	vehicle.shadow_sprite_paths = sprites;
	return true;
}

static bool ParseVehicleAnimationSprites(tinyxml2::XMLElement *parentNode, VehicleType &vehicle)
{
	std::list<UString> sprites;

	for (tinyxml2::XMLElement *node = parentNode->FirstChildElement(); node != nullptr;
	     node = node->NextSiblingElement())
	{
		UString node_name = node->Name();
		if (node_name == "frame")
		{
			UString frame_path;
			if (!ReadElement(node, frame_path))
			{
				LogError("Animation frame %u  for vehicle_type ID \"%s\" missing sprite path",
				         static_cast<unsigned>(sprites.size()), vehicle.id.c_str());
				return false;
			}
			sprites.push_back(frame_path);
			// FIXME: Read 'offset' attribute to handle larger-than-1x1 stratmap sprites
			continue;
		}
		else
		{
			LogError("Unexpected node \"%s\" in animation section for vehicle_type ID \"%s\"",
			         node_name.c_str(), vehicle.id.c_str());
			return false;
		}
	}

	vehicle.animation_sprite_paths = sprites;
	return true;
}

static bool ParseVehicleDirectionalSprites(tinyxml2::XMLElement *parentNode, VehicleType &vehicle)
{
	std::map<VehicleType::Banking, std::map<VehicleType::Direction, UString>> sprites;

	for (tinyxml2::XMLElement *node = parentNode->FirstChildElement(); node != nullptr;
	     node = node->NextSiblingElement())
	{
		UString node_name = node->Name();
		if (node_name == "sprite")
		{
			VehicleType::Direction direction;
			if (!ReadAttribute(node, "direction", direction_map, direction))
			{
				LogError("Invalid direction \"%s\" in directional_sprites 'direction' attribute on "
				         "vehicle_type "
				         "ID \"%s\"",
				         node->Attribute("direction"), vehicle.id.c_str());
				return false;
			}

			// Allow missing banking to mean 'flat'?
			VehicleType::Banking banking = VehicleType::Banking::Flat;
			if (!ReadAttribute(node, "banking", banking_map, banking))
			{
				LogError("Directional_sprites banking \"%s\" for vehicle_type ID \"%s\" invalid",
				         node->Attribute("banking", vehicle.id.c_str()));
				return false;
			}

			UString sprite_path;
			if (!ReadElement(node, sprite_path))
			{
				LogError("Directional_sprites direction \"%s\" for vehicle_type ID \"%s\" missing "
				         "sprite path",
				         node->Attribute("direction"), vehicle.id.c_str());
				return false;
			}
			sprites[banking][direction] = sprite_path;
			continue;
		}
		else
		{
			LogError(
			    "Unexpected node \"%s\" in directional_sprites section for vehicle_type ID \"%s\"",
			    node_name.c_str(), vehicle.id.c_str());
			return false;
		}
	}

	vehicle.sprite_paths = sprites;
	return true;
}

static bool ParseVehicleInitialEquipment(tinyxml2::XMLElement *parentNode, VehicleType &vehicle)
{
	for (tinyxml2::XMLElement *node = parentNode->FirstChildElement(); node != nullptr;
	     node = node->NextSiblingElement())
	{
		UString node_name(node->Name());
		if (node_name == "equipment")
		{
			Vec2<int> pos;
			UString type;
			bool found_type = false;
			bool found_pos = false;
			for (tinyxml2::XMLElement *child = node->FirstChildElement(); child != nullptr;
			     child = child->NextSiblingElement())
			{
				UString child_name(child->Name());
				if (child_name == "type")
				{
					if (found_type)
					{
						LogError("Multiple \"type\" nodes in initial equipment for vehicle_type ID "
						         "\"%s\"",
						         vehicle.id.c_str());
						return false;
					}
					found_type = true;
					ReadElement(child, type);
				}
				else if (child_name == "position")
				{
					if (found_pos)
					{
						LogError("Multiple \"position\" nodes in initial equipent for vehicle_type "
						         "ID \"%s\"",
						         vehicle.id.c_str());
						return false;
					}
					found_pos = true;
					ReadElement(child, pos);
				}
				else
				{
					LogError("Unexpected node \"%s\" in initial_equipment equipment for "
					         "vehicle_type ID \"%s\"",
					         child_name.c_str(), vehicle.id.c_str());
					return false;
				}
			}
			if (!found_type)
			{
				LogError("Missing \"type\" node in initial equipment for vehicle_type ID \"%s\"",
				         vehicle.id.c_str());
				return false;
			}
			if (!found_pos)
			{
				LogError("Missing \"position\" node in initial equipment for vehicle_type ID "
				         "\"%s\"",
				         vehicle.id.c_str());
				return false;
			}
			vehicle.initial_equipment_list.emplace_back(std::make_pair(pos, type));
		}
		else
		{
			LogError(
			    "Unexpected node \"%s\" in initial_equipment section for vehicle_type ID \"%s\"",
			    node_name.c_str(), vehicle.id.c_str());
			return false;
		}
	}
	LogInfo("Added %d initial_equipment objects to vehicle_type ID \"%s\"",
	        vehicle.initial_equipment_list.size(), vehicle.id.c_str());
	return true;
}

static bool ParseVehicleEquipmentLayout(tinyxml2::XMLElement *parentNode, VehicleType &vehicle)
{
	for (tinyxml2::XMLElement *node = parentNode->FirstChildElement(); node != nullptr;
	     node = node->NextSiblingElement())
	{
		UString node_name(node->Name());
		if (node_name == "slot")
		{
			VEquipmentType::Type type;
			if (!ReadAttribute(node, "type", equipment_slot_type_map, type))
			{
				LogError(
				    "Failed to read equipment slot type attribute \"%s\" on vehicle_type ID \"%s\"",
				    node->Attribute("type"), vehicle.id.c_str());
				return false;
			}
			VehicleType::AlignmentX alignX;
			if (!ReadAttribute(node, "align_x", align_x_map, alignX))
			{
				LogError("Failed to read equipment slot align_x attribute \"%s\" on vehicle_type "
				         "ID \"%s\"",
				         node->Attribute("align_x"), vehicle.id.c_str());
				return false;
			}
			VehicleType::AlignmentY alignY;
			if (!ReadAttribute(node, "align_y", align_y_map, alignY))
			{
				LogError("Failed to read equipment slot align_y attribute \"%s\" on vehicle_type "
				         "ID \"%s\"",
				         node->Attribute("align_y"), vehicle.id.c_str());
				return false;
			}
			bool p0_found = false;
			Vec2<int> p0{0, 0};
			bool p1_found = false;
			Vec2<int> p1{0, 0};
			for (tinyxml2::XMLElement *subNode = node->FirstChildElement(); subNode != nullptr;
			     subNode = subNode->NextSiblingElement())
			{
				UString subNodeName(subNode->Name());
				if (subNodeName == "p0")
				{
					if (p0_found)
					{
						LogError(
						    "Multiple \"p0\" nodes in equipment slot on vehicle_type ID \"%s\"",
						    vehicle.id.c_str());
						return false;
					}
					p0_found = true;
					if (!ReadElement(subNode, p0))
					{
						LogError("Failed to read \"p0\" nodes in equipment slot on vehicle_type ID "
						         "\"%s\"",
						         vehicle.id.c_str());
						return false;
					}
				}
				else if (subNodeName == "p1")
				{
					if (p1_found)
					{
						LogError(
						    "Multiple \"p1\" nodes in equipment slot on vehicle_type ID \"%s\"",
						    vehicle.id.c_str());
						return false;
					}
					p1_found = true;
					if (!ReadElement(subNode, p1))
					{
						LogError("Failed to read \"p1\" nodes in equipment slot on vehicle_type ID "
						         "\"%s\"",
						         vehicle.id.c_str());
						return false;
					}
				}
				else
				{
					LogError("Unexpected sub-node \"%s\" in equipment_layout section for "
					         "vehicle_type ID \"%s\"",
					         node_name.c_str(), vehicle.id.c_str());
					return false;
				}
			}
			if (!p0_found)
			{
				LogError("Missing \"p0\" subnode on vehicle_type "
				         "ID \"%s\"",
				         vehicle.id.c_str());
				return false;
			}
			if (!p1_found)
			{
				LogError("Missing \"p1\" subnode on vehicle_type "
				         "ID \"%s\"",
				         vehicle.id.c_str());
				return false;
			}
			vehicle.equipment_layout_slots.emplace_back(type, alignX, alignY, Rect<int>{p0, p1});
		}
		else
		{
			LogError(
			    "Unexpected node \"%s\" in equipment_layout section for vehicle_type ID \"%s\"",
			    node_name.c_str(), vehicle.id.c_str());
			return false;
		}
	}
	return true;
}

static bool ParseVehicleTypeNode(tinyxml2::XMLElement *node, VehicleType &vehicle)
{
	UString node_name(node->Name());

	if (node_name == "name")
	{
		if (!ReadElement(node, vehicle.name))
		{
			LogError("Failed to read name text \"%s\" for vehicle_type ID \"%s\"", node->GetText(),
			         vehicle.id.c_str());
			return false;
		}
		return true;
	}
	else if (node_name == "manufacturer")
	{
		if (!ReadElement(node, vehicle.manufacturer))
		{
			LogError("Failed to read manufacturer text \"%s\" for vehicle_type ID \"%s\"",
			         node->GetText(), vehicle.id.c_str());
			return false;
		}
		return true;
	}
	else if (node_name == "size")
	{
		if (!ReadElement(node, vehicle.size))
		{
			LogError("Failed to read size text \"%s\" on vehicle_type ID \"%s\"", node->GetText(),
			         vehicle.id.c_str());
			return false;
		}
		return true;
	}
	else if (node_name == "image_offset")
	{
		if (!ReadElement(node, vehicle.image_offset))
		{
			LogError("Failed to read image_offset text \"%s\" on vehicle_type ID \"%s\"",
			         node->GetText(), vehicle.id.c_str());
			return false;
		}
		return true;
	}
	else if (node_name == "shadow_offset")
	{
		if (!ReadElement(node, vehicle.shadow_offset))
		{
			LogError("Failed to read shadow_offset text \"%s\" on vehicle_type ID \"%s\"",
			         node->GetText(), vehicle.id.c_str());
			return false;
		}
		return true;
	}
	else if (node_name == "stratmap")
	{
		// We replace any previous 'stratmap' declaration here
		return ParseVehicleStratmapSprites(node, vehicle);
	}
	else if (node_name == "directional_sprites")
	{
		// We replace and previous 'directional_sprites' declaration here
		return ParseVehicleDirectionalSprites(node, vehicle);
	}
	else if (node_name == "shadows")
	{
		// We replace and previous 'shadows' declaration here
		return ParseVehicleShadowSprites(node, vehicle);
	}
	else if (node_name == "acceleration")
	{
		if (!ReadElement(node, vehicle.acceleration))
		{
			LogError("Failed to read acceleration text \"%s\" on vehicle_type ID \"%s\"",
			         node->GetText(), vehicle.id.c_str());
			return false;
		}
		return true;
	}
	else if (node_name == "top_speed")
	{
		if (!ReadElement(node, vehicle.top_speed))
		{
			LogError("Failed to read top_speed text \"%s\" on vehicle_type ID \"%s\"",
			         node->GetText(), vehicle.id.c_str());
			return false;
		}
		return true;
	}
	else if (node_name == "health")
	{
		if (!ReadElement(node, vehicle.health))
		{
			LogError("Failed to read health text \"%s\" on vehicle_type ID \"%s\"", node->GetText(),
			         vehicle.id.c_str());
			return false;
		}
		return true;
	}
	else if (node_name == "crash_health")
	{
		if (!ReadElement(node, vehicle.crash_health))
		{
			LogError("Failed to read crash_health text \"%s\" on vehicle_type ID \"%s\"",
			         node->GetText(), vehicle.id.c_str());
			return false;
		}
		return true;
	}
	else if (node_name == "weight")
	{
		if (!ReadElement(node, vehicle.weight))
		{
			LogError("Failed to read weight text \"%s\" on vehicle_type ID \"%s\"", node->GetText(),
			         vehicle.id.c_str());
			return false;
		}
		return true;
	}
	else if (node_name == "armour")
	{
		std::map<UString, VehicleType::ArmourDirection> directions = {
		    {"top", VehicleType::ArmourDirection::Top},
		    {"bottom", VehicleType::ArmourDirection::Bottom},
		    {"front", VehicleType::ArmourDirection::Front},
		    {"rear", VehicleType::ArmourDirection::Rear},
		    {"left", VehicleType::ArmourDirection::Left},
		    {"right", VehicleType::ArmourDirection::Right},
		};
		VehicleType::ArmourDirection dir;
		if (!ReadAttribute(node, "direction", directions, dir))
		{
			LogError("Failed to read understand attribute \"%s\" on vehicle_type ID \"%s\"",
			         node->Attribute("direction"), vehicle.id.c_str());
			return false;
		}
		float value;
		if (!ReadElement(node, value))
		{
			LogError("Failed to read armor direction \"%s\" text \"%s\" on vehicle_type ID \"%s\"",
			         node->Attribute("direction"), node->GetText(), vehicle.id.c_str());
			return false;
		}
		return true;
	}
	else if (node_name == "passengers")
	{
		if (!ReadElement(node, vehicle.passengers))
		{
			LogError("Failed to read passengers text \"%s\" on vehicle_type ID \"%s\"",
			         node->GetText(), vehicle.id.c_str());
			return false;
		}
		return true;
	}
	else if (node_name == "aggressiveness")
	{
		if (!ReadElement(node, vehicle.aggressiveness))
		{
			LogError("Failed to read aggressiveness text \"%s\" on vehicle_type ID \"%s\"",
			         node->GetText(), vehicle.id.c_str());
			return false;
		}
		return true;
	}
	else if (node_name == "score")
	{
		if (!ReadElement(node, vehicle.score))
		{
			LogError("Failed to read score text \"%s\" on vehicle_type ID \"%s\"", node->GetText(),
			         vehicle.id.c_str());
			return false;
		}
		return true;
	}
	else if (node_name == "equipment_screen")
	{
		if (!ReadElement(node, vehicle.equipment_screen_path))
		{
			LogError("Failed to read equipment_screen text \"%s\" on vehicle_type ID \"%s\"",
			         node->GetText(), vehicle.id.c_str());
			return false;
		}
		return true;
	}
	else if (node_name == "icon")
	{
		if (!ReadElement(node, vehicle.icon_path))
		{
			LogError("Failed to read icon text \"%s\" on vehicle_type ID \"%s\"", node->GetText(),
			         vehicle.id.c_str());
			return false;
		}
		return true;
	}
	else if (node_name == "equip_icon_big")
	{
		if (!ReadElement(node, vehicle.equip_icon_big_path))
		{
			LogError("Failed to read big equip icon text \"%s\" on vehicle_type ID \"%s\"",
			         node->GetText(), vehicle.id.c_str());
			return false;
		}
		return true;
	}
	else if (node_name == "equip_icon_small")
	{
		if (!ReadElement(node, vehicle.equip_icon_small_path))
		{
			LogError("Failed to read small equip icon text \"%s\" on vehicle_type ID \"%s\"",
			         node->GetText(), vehicle.id.c_str());
			return false;
		}
		return true;
	}
	else if (node_name == "equipment_layout")
	{
		return ParseVehicleEquipmentLayout(node, vehicle);
	}
	else if (node_name == "initial_equipment")
	{
		return ParseVehicleInitialEquipment(node, vehicle);
	}
	else if (node_name == "animation")
	{
		return ParseVehicleAnimationSprites(node, vehicle);
	}
	else if (node_name == "crashed")
	{
		if (!ReadElement(node, vehicle.crashed_sprite_path))
		{
			LogError("Failed to read crashed text \"%s\" on vehicle_type ID \"%s\"",
			         node->GetText(), vehicle.id.c_str());
			return false;
		}
		return true;
	}
	else if (node_name == "lof")
	{
		if (vehicle.voxelMap)
		{
			LogError("Multiple LOF nodes on vehicle_type ID \"%s\"", vehicle.id.c_str());
			return false;
		}
		int sizeX, sizeY, sizeZ;
		if (!ReadAttribute(node, "sizeX", sizeX))
		{
			LogError("Failed to read \"sizeX\" attribute on LOF for vehicle_type ID \"%s\"",
			         vehicle.id.c_str());
			return false;
		}
		if (!ReadAttribute(node, "sizeY", sizeY))
		{
			LogError("Failed to read \"sizeY\" attribute on LOF for vehicle_type ID \"%s\"",
			         vehicle.id.c_str());
			return false;
		}
		if (!ReadAttribute(node, "sizeZ", sizeZ))
		{
			LogError("Failed to read \"sizeZ\" attribute on LOF for vehicle_type ID \"%s\"",
			         vehicle.id.c_str());
			return false;
		}
		vehicle.voxelMap = mksp<VoxelMap>(Vec3<int>{sizeX, sizeY, sizeZ});

		int layerCount = 0;

		for (tinyxml2::XMLElement *lofNode = node->FirstChildElement(); lofNode != nullptr;
		     lofNode = lofNode->NextSiblingElement())
		{
			UString lofNodeName(lofNode->Name());
			if (lofNodeName == "loflayer")
			{
				if (layerCount >= sizeZ)
				{
					LogError(
					    "LOF section for vehicle_type ID \"%s\" has more than sizeZ (%d) layers",
					    vehicle.id.c_str(), sizeZ);
					return false;
				}
				UString voxelSliceString(lofNode->GetText());
				auto layer = fw().data->load_voxel_slice(voxelSliceString);
				if (!layer)
				{
					LogError("LOF section for vehicle_type ID \"%s\": \"%s\" failed to be loaded",
					         vehicle.id.c_str(), voxelSliceString.c_str());
					return false;
				}
				LogInfo("Vehicle \"%s\" layer %d \"%s\"", vehicle.id.c_str(), layerCount,
				        voxelSliceString.c_str());
				vehicle.voxelMap->setSlice(layerCount, layer);
				layerCount++;
			}
			else
			{
				LogError("Unknown node \"%s\" in LOF section for vehicle_type ID \"%s\"",
				         lofNodeName.c_str(), vehicle.id.c_str());
				return false;
			}
		}
		return true;
	}
	return false;
}

bool RulesLoader::ParseVehicleType(Rules &rules, tinyxml2::XMLElement *rootNode)
{
	TRACE_FN;
	std::ignore = fw;

	if (UString(rootNode->Name()) != "vehicle")
	{
		LogError("Called on unexpected node \"%s\"", rootNode->Name());
		return false;
	}

	UString id;

	if (!ReadAttribute(rootNode, "id", id))
	{
		LogError("No \"id\" in vehicle");
		return false;
	}
	std::map<UString, VehicleType::Type> typeMap{
	    {"flying", VehicleType::Type::Flying},
	    {"ground", VehicleType::Type::Ground},
	    {"ufo", VehicleType::Type::UFO},
	};
	VehicleType::Type type;

	if (!ReadAttribute(rootNode, "type", typeMap, type))
	{
		LogError("Invalid \"type\" in vehicle \"%s\"", id.c_str());
		return false;
	}

	if (rules.vehicle_types[id] == nullptr)
	{
		switch (type)
		{
			case VehicleType::Type::Flying:
			{
				rules.vehicle_types[id].reset(new VehicleType(type, id));
				break;
			}
			case VehicleType::Type::Ground:
			{
				rules.vehicle_types[id].reset(new VehicleType(type, id));
				break;
			}
			case VehicleType::Type::UFO:
			{
				rules.vehicle_types[id].reset(new VehicleType(type, id));
				break;
			}
			default:
				LogError("Invalid vehicle type");
				return false;
		}
	}
	else
	{
		LogInfo("Overriding existing ID \"%s\"", id.c_str());
	}

	auto &vehicle = *rules.vehicle_types[id];
	if (vehicle.type != type)
	{
		LogError("Trying to change type of type_type id \"%s\"", id.c_str());
		return false;
	}

	for (tinyxml2::XMLElement *e = rootNode->FirstChildElement(); e != nullptr;
	     e = e->NextSiblingElement())
	{
		if (!ParseVehicleTypeNode(e, vehicle))
		{
			LogError("Unexpected node \"%s\" on vehicle_type id \"%s\"", e->Name(), id.c_str());
			return false;
		}
	}

	return true;
}

VehicleType::VehicleType(Type type, const UString &id) : numCreated(0), type(type), id(id) {}

static std::map<VehicleType::Direction, Vec3<float>> direction_vectors = {
    {VehicleType::Direction::N, {0, -1, 0}}, {VehicleType::Direction::NE, {1, -1, 0}},
    {VehicleType::Direction::E, {1, 0, 0}},  {VehicleType::Direction::SE, {1, 1, 0}},
    {VehicleType::Direction::S, {0, 1, 0}},  {VehicleType::Direction::SW, {-1, 1, 0}},
    {VehicleType::Direction::W, {-1, 0, 0}}, {VehicleType::Direction::NW, {-1, -1, 0}},
};

static std::map<VehicleType::Banking, Vec3<float>> banking_vectors = {
    {VehicleType::Banking::Flat, Vec3<float>{0, 0, 0}},
    {VehicleType::Banking::Ascending, Vec3<float>{0, 0, 1}},
    {VehicleType::Banking::Descending, Vec3<float>{0, 0, -1}},
};

static Vec3<float> getDirectionVector(VehicleType::Direction dir,
                                      VehicleType::Banking bank = VehicleType::Banking::Flat)
{
	Vec3<float> v = direction_vectors[dir];
	v += banking_vectors[bank];
	return glm::normalize(v);
}

bool VehicleType::isValid(Rules &rules)
{
	TRACE_FN_ARGS1("ID", id);
	if (!RulesLoader::isValidVehicleTypeID(id))
	{
		LogError("Invalid ID \"%s\" on vehicle_type", id.c_str());
		return false;
	}

	if (!RulesLoader::isValidStringID(this->name))
	{
		LogError("vehicle_type \"%s\" has invalid name \"%s\" ", id.c_str(), this->name.c_str());
		return false;
	}

	if (!RulesLoader::isValidOrganisation(rules, this->manufacturer))
	{
		LogError("vehicle_type \"%s\" has unknown manufacturer \"%s\" ", id.c_str(),
		         this->manufacturer.c_str());
		return false;
	}

	if (this->equipment_screen_path != "")
	{
		this->equipment_screen = fw().data->load_image(this->equipment_screen_path);
		if (!this->equipment_screen)
		{
			LogError("vehicle_type \"%s\" failed to load equipment_screen \"%s\"", id.c_str(),
			         this->equipment_screen_path.c_str());
			return false;
		}
	}

	if (this->icon_path == "")
	{
		LogError("vehicle_type \"%s\" has no icon", id.c_str());
		return false;
	}
	this->icon = fw().data->load_image(this->icon_path);
	if (!this->icon)
	{
		LogError("vehicle_type \"%s\" failed to load icon \"%s\"", id.c_str(),
		         this->icon_path.c_str());
		return false;
	}

	if (this->equip_icon_big_path != "")
	{
		this->equip_icon_big = fw().data->load_image(this->equip_icon_big_path);
		if (!this->icon)
		{
			LogError("vehicle_type \"%s\" failed to load big equip icon \"%s\"", id.c_str(),
			         this->equip_icon_big_path.c_str());
			return false;
		}
	}

	if (this->equip_icon_small_path != "")
	{
		this->equip_icon_small = fw().data->load_image(this->equip_icon_small_path);
		if (!this->icon)
		{
			LogError("vehicle_type \"%s\" failed to load small equip icon \"%s\"", id.c_str(),
			         this->equip_icon_small_path.c_str());
			return false;
		}
	}

	if (this->crashed_sprite_path != "")
	{
		this->crashed_sprite = fw().data->load_image(this->crashed_sprite_path);
		if (!this->icon)
		{
			LogError("vehicle_type \"%s\" failed to load crashed_sprite \"%s\"", id.c_str(),
			         this->crashed_sprite_path.c_str());
			return false;
		}
		if (this->crash_health == 0)
		{
			LogError("vehicle_type \"%s\" has crashed_sprite but no crash_health", id.c_str());
			return false;
		}
	}
	if (this->health == 0)
	{
		LogError("vehicle_type \"%s\" has zero crash_health", id.c_str());
		return false;
	}
	if (this->weight == 0)
	{
		LogError("vehicle_type \"%s\" has zero weight", id.c_str());
		return false;
	}

	for (auto &pair : this->strategy_sprite_paths)
	{
		auto direction = pair.first;
		auto sprite_path = pair.second;
		auto vec = getDirectionVector(direction);
		auto sprite = fw().data->load_image(sprite_path);
		if (!sprite)
		{
			LogError("vehicle_type \"%s\" failed to load stratmap sprite \"%s\"", id.c_str(),
			         sprite_path.c_str());
			return false;
		}
		this->directional_strategy_sprites.emplace_back(std::make_pair(vec, sprite));
	}

	if (this->directional_strategy_sprites.empty())
	{
		LogError("vehicle_type \"%s\" has no stratmap sprites", id.c_str());
		return false;
	}

	for (auto &pair : this->shadow_sprite_paths)
	{
		auto direction = pair.first;
		auto sprite_path = pair.second;
		auto vec = getDirectionVector(direction);
		auto sprite = fw().data->load_image(sprite_path);
		if (!sprite)
		{
			LogError("vehicle_type \"%s\" failed to load shadow sprite \"%s\"", id.c_str(),
			         sprite_path.c_str());
			return false;
		}
		this->directional_shadow_sprites.emplace_back(std::make_pair(vec, sprite));
	}

	for (auto &bpair : this->sprite_paths)
	{
		auto bank = bpair.first;
		// FIXME: We don't handle left/right banking at the moment (need a rotation? Change vector
		// to quaternion?)
		if (bank == VehicleType::Banking::Left || bank == VehicleType::Banking::Right)
			continue;
		for (auto &dpair : bpair.second)
		{
			auto direction = dpair.first;
			auto sprite_path = dpair.second;
			auto vec = getDirectionVector(direction, bank);
			auto sprite = fw().data->load_image(sprite_path);
			if (!sprite)
			{
				LogError("vehicle_type \"%s\" failed to load sprite \"%s\"", id.c_str(),
				         sprite_path.c_str());
				return false;
			}
			this->directional_sprites.emplace_back(std::make_pair(vec, sprite));
		}
	}

	for (auto &sprite_path : this->animation_sprite_paths)
	{
		auto sprite = fw().data->load_image(sprite_path);
		if (!sprite)
		{
			LogError("vehicle_type \"%s\" failed to load animation sprite \"%s\"", id.c_str(),
			         sprite_path.c_str());
			return false;
		}
		this->animation_sprites.emplace_back(sprite);
	}
	if (this->directional_sprites.empty() && this->animation_sprites.empty())
	{
		LogError("vehicle_type \"%s\" has no directional or animation sprites", id.c_str());
		return false;
	}

	if (!this->directional_sprites.empty() && !this->animation_sprites.empty())
	{
		LogError("vehicle_type \"%s\" has both directional and animation sprites", id.c_str());
		return false;
	}

	return true;
}
}; // namespace OpenApoc
