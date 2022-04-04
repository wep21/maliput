#include "maliput/base/manual_discrete_value_rule_state_provider.h"

#include <algorithm>
#include <stdexcept>
#include <string>

#include "maliput/base/rule_filter.h"
#include "maliput/common/logger.h"

namespace maliput {

void ManualDiscreteValueRuleStateProvider::ValidateRuleState(
    const api::rules::DiscreteValueRule& discrete_value_rule,
    const api::rules::DiscreteValueRule::DiscreteValue& state) const {
  if (std::find(discrete_value_rule.values().begin(), discrete_value_rule.values().end(), state) ==
      discrete_value_rule.values().end()) {
    MALIPUT_THROW_MESSAGE("DiscreteValue is not in DiscreteValueRule " + discrete_value_rule.id().string() +
                          "'s' values().");
  }
}

void ManualDiscreteValueRuleStateProvider::SetState(
    const api::rules::Rule::Id& id, const api::rules::DiscreteValueRule::DiscreteValue& state,
    const std::optional<api::rules::DiscreteValueRule::DiscreteValue>& next_state,
    const std::optional<double>& duration_until) {
  const api::rules::DiscreteValueRule rule = rulebook_->GetDiscreteValueRule(id);
  ValidateRuleState(rule, state);
  if (next_state.has_value()) {
    ValidateRuleState(rule, *next_state);
    if (duration_until.has_value()) {
      MALIPUT_THROW_UNLESS(*duration_until > 0.);
    }
  } else {
    MALIPUT_THROW_UNLESS(!duration_until.has_value());
  }

  api::rules::DiscreteValueRuleStateProvider::StateResult state_result;
  state_result.state = state;
  if (next_state.has_value()) {
    state_result.next = {{*next_state, duration_until}};
  }

  states_[id] = state_result;
}

std::optional<api::rules::DiscreteValueRuleStateProvider::StateResult> ManualDiscreteValueRuleStateProvider::DoGetState(
    const api::rules::Rule::Id& id) const {
  const auto it = states_.find(id);
  if (it == states_.end()) {
    return std::nullopt;
  }
  return it->second;
}

std::optional<api::rules::DiscreteValueRuleStateProvider::StateResult> ManualDiscreteValueRuleStateProvider::DoGetState(
    const api::RoadPosition& road_position, const api::rules::Rule::TypeId& rule_type, double tolerance) const {
  const auto filtered_discrete_value_rules = GetFilteredDiscreteValueRules(road_position, rule_type, tolerance);
  if (filtered_discrete_value_rules.size() > 1) {
    maliput::log()->warn(
        "For rule_type: {} and road_position: [LaneId: {}, LanePos: {}] there are more than one possible rules: ",
        rule_type.string(), road_position.lane->id(), road_position.pos.srh().to_str());
    for (const auto& rule : filtered_discrete_value_rules) {
      maliput::log()->warn("\tRule id: {} matches with rule_type: {} and road_position: [LaneId: {}, LanePos: {}]",
                           rule.first.string(), rule_type.string(), road_position.lane->id(),
                           road_position.pos.srh().to_str());
    }
  }
  std::optional<api::rules::DiscreteValueRuleStateProvider::StateResult> current_state{std::nullopt};
  if (!filtered_discrete_value_rules.empty()) {
    const auto state = states_.find(filtered_discrete_value_rules.begin()->first);
    MALIPUT_THROW_UNLESS(state != states_.end());
    current_state = std::make_optional<>(state->second);
  }
  return current_state;
}

std::map<api::rules::DiscreteValueRule::Id, api::rules::DiscreteValueRule>
ManualDiscreteValueRuleStateProvider::GetFilteredDiscreteValueRules(const api::RoadPosition& road_position,
                                                                    const api::rules::Rule::TypeId& rule_type,
                                                                    double tolerance) const {
  MALIPUT_THROW_UNLESS(tolerance >= 0.);
  const auto query_result_rules = rulebook_->Rules();
  const DiscreteValueRuleFilter rule_type_filter = [&rule_type](const api::rules::DiscreteValueRule& rule) {
    return rule.type_id() == rule_type;
  };
  const DiscreteValueRuleFilter zone_filter = [&road_position, tolerance](const api::rules::DiscreteValueRule& rule) {
    const api::LaneSRange lane_s_range{road_position.lane->id(),
                                       api::SRange{road_position.pos.s(), road_position.pos.s()}};
    return rule.zone().Intersects(api::LaneSRoute({lane_s_range}), tolerance);
  };
  const auto filtered_rules = FilterRules(query_result_rules, {rule_type_filter, zone_filter}, {});
  return filtered_rules.discrete_value_rules;
}

}  // namespace maliput
