#pragma once

#include <memory>
#include <string>

#include "maliput/api/rules/phase_ring_book.h"
#include "maliput/api/rules/road_rulebook.h"
#include "maliput/api/rules/traffic_light_book.h"

namespace maliput {

/// Instantiates and returns an api::rules::PhaseRingBook instance based on the
/// specified @p rulebook, and @p input document. Based on old rule API.
///
/// @param rulebook Contains the rules.
///
/// @param traffic_light_book Contains the traffic lights.
///
/// @param input The YAML PhaseRings document.
///
/// @return The newly created api::rules::PhaseRingBook instance.
std::unique_ptr<api::rules::PhaseRingBook> LoadPhaseRingBookOldRules(
    const api::rules::RoadRulebook* rulebook, const api::rules::TrafficLightBook* traffic_light_book,
    const std::string& input);

/// Instantiates and returns an api::rules::PhaseRingBook instance based on the
/// specified @p rulebook, and @p filename. Based on old rule API.
///
/// @param rulebook Contains the rules.
///
/// @param traffic_light_book Contains the traffic lights.
///
/// @param filename The YAML file that contains a PhaseRings document.
///
/// @return The newly created api::rules::PhaseRingBook instance.
std::unique_ptr<api::rules::PhaseRingBook> LoadPhaseRingBookFromFileOldRules(
    const api::rules::RoadRulebook* rulebook, const api::rules::TrafficLightBook* traffic_light_book,
    const std::string& filename);

/// Instantiates and returns an api::rules::PhaseRingBook instance based on the
/// specified @p rulebook, and @p input document.
///
/// @param rulebook Contains the rules.
///
/// @param traffic_light_book Contains the traffic lights.
///
/// @param input The YAML PhaseRings document.
///
/// @return The newly created api::rules::PhaseRingBook instance.
std::unique_ptr<api::rules::PhaseRingBook> LoadPhaseRingBook(const api::rules::RoadRulebook* rulebook,
                                                             const api::rules::TrafficLightBook* traffic_light_book,
                                                             const std::string& input);

/// Instantiates and returns an api::rules::PhaseRingBook instance based on the
/// specified @p rulebook, and @p filename.
///
/// @param rulebook Contains the rules.
///
/// @param traffic_light_book Contains the traffic lights.
///
/// @param filename The YAML file that contains a PhaseRings document.
///
/// @return The newly created api::rules::PhaseRingBook instance.
std::unique_ptr<api::rules::PhaseRingBook> LoadPhaseRingBookFromFile(
    const api::rules::RoadRulebook* rulebook, const api::rules::TrafficLightBook* traffic_light_book,
    const std::string& filename);

}  // namespace maliput
