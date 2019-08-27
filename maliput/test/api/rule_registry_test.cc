#include "maliput/api/rules/rule_registry.h"

#include <gtest/gtest.h>

#include "maliput/common/assertion_error.h"

namespace maliput {
namespace api {
namespace rules {
namespace test {

// Evaluates queries to an empty RuleRegistry.
GTEST_TEST(EmptyRuleRegistry, AccessorsTest) {
  const RuleRegistry dut;

  EXPECT_TRUE(dut.RangeValueRuleTypes().empty());
  EXPECT_TRUE(dut.DiscreteValueRuleTypes().empty());

  const drake::optional<RuleRegistry::QueryResult> result =
      dut.GetPossibleStatesOfRuleType(Rule::TypeId("any_rule_type"));
  EXPECT_FALSE(result.has_value());
}

// Evaluates queries after registering RangeValueRule types.
GTEST_TEST(RegisterRangeValueRule, RegisterAndQueryTest) {
  const Rule::TypeId kTypeA("RangeValueRuleTypeA");
  const Rule::TypeId kTypeB("RangeValueRuleTypeB");
  const Rule::TypeId kTypeC("RangeValueRuleTypeC");
  const RangeValueRule::Range kRangeA{"range_description_a", 123. /* min */, 456. /* max */};
  const RangeValueRule::Range kRangeB{"range_description_b", 456. /* min */, 789. /* max */};

  RuleRegistry dut;
  // Registers RangeValueRule types.
  EXPECT_NO_THROW(dut.RegisterRangeValueRule(kTypeA, {kRangeA, kRangeB}));
  EXPECT_NO_THROW(dut.RegisterRangeValueRule(kTypeB, {kRangeA}));
  // Throws because of duplicated type ID.
  EXPECT_THROW(dut.RegisterRangeValueRule(kTypeB, {kRangeA}), maliput::common::assertion_error);
  EXPECT_THROW(dut.RegisterDiscreteValueRule(kTypeB, {"SomeValue"}), maliput::common::assertion_error);
  // Throws because of empty range vector.
  EXPECT_THROW(dut.RegisterRangeValueRule(kTypeC, {} /* ranges */), maliput::common::assertion_error);
  // Throws because of duplicated ranges.
  EXPECT_THROW(dut.RegisterRangeValueRule(kTypeC, {kRangeA, kRangeA}), maliput::common::assertion_error);

  EXPECT_TRUE(dut.DiscreteValueRuleTypes().empty());

  const std::map<Rule::TypeId, std::vector<RangeValueRule::Range>> kExpectedRuleTypes{{kTypeA, {kRangeA, kRangeB}},
                                                                                      {kTypeB, {kRangeA}}};

  const std::map<Rule::TypeId, std::vector<RangeValueRule::Range>> range_value_rule_types = dut.RangeValueRuleTypes();
  EXPECT_EQ(range_value_rule_types.size(), kExpectedRuleTypes.size());
  for (const auto& rule_type : kExpectedRuleTypes) {
    const auto found_rule_values = range_value_rule_types.find(rule_type.first);
    EXPECT_NE(found_rule_values, range_value_rule_types.end());
    EXPECT_EQ(found_rule_values->second.size(), rule_type.second.size());
    for (const RangeValueRule::Range& range : found_rule_values->second) {
      EXPECT_NE(std::find(rule_type.second.begin(), rule_type.second.end(), range), rule_type.second.end());
    }
  }

  // Finds each type.
  drake::optional<RuleRegistry::QueryResult> result = dut.GetPossibleStatesOfRuleType(kTypeA);
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result->type_id, kTypeA);
  EXPECT_TRUE(result->range_values.has_value());
  EXPECT_EQ(result->range_values->size(), 2);
  EXPECT_EQ(result->range_values->at(0), kRangeA);
  EXPECT_EQ(result->range_values->at(1), kRangeB);
  EXPECT_FALSE(result->discrete_values.has_value());

  result = dut.GetPossibleStatesOfRuleType(kTypeB);
  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(result->type_id, kTypeB);
  EXPECT_TRUE(result->range_values.has_value());
  EXPECT_EQ(result->range_values->size(), 1);
  EXPECT_EQ(result->range_values->at(0), kRangeA);
  EXPECT_FALSE(result->discrete_values.has_value());

  result = dut.GetPossibleStatesOfRuleType(Rule::TypeId("any_rule_type"));
  EXPECT_FALSE(result.has_value());
}

// Evaluates queries after registering DiscreteValueRule types.
GTEST_TEST(RegisterDiscreteValueRule, RegisterAndQueryTest) {
  const Rule::TypeId kTypeA("DiscreteValueTypeA");
  const std::vector<std::string> kValuesA{"ValueA1", "ValueA2"};
  const Rule::TypeId kTypeB("DiscreteValueRuleTypeB");
  const std::vector<std::string> kValuesB{"ValueB1", "ValueB2", "ValueB3"};
  const RangeValueRule::Range kRange{"range_description_a", 123. /* min */, 456. /* max */};

  RuleRegistry dut;
  // Registers DiscreteValueRule types.
  EXPECT_NO_THROW(dut.RegisterDiscreteValueRule(kTypeA, kValuesA));
  EXPECT_NO_THROW(dut.RegisterDiscreteValueRule(kTypeB, kValuesB));
  // Throws because of duplicated type ID.
  EXPECT_THROW(dut.RegisterDiscreteValueRule(kTypeB, {"SomeValue"}), maliput::common::assertion_error);
  EXPECT_THROW(dut.RegisterRangeValueRule(kTypeB, {kRange}), maliput::common::assertion_error);
  // Throws because of empty vector.
  EXPECT_THROW(dut.RegisterDiscreteValueRule(Rule::TypeId("SomeRuleType"), {}), maliput::common::assertion_error);

  EXPECT_TRUE(dut.RangeValueRuleTypes().empty());

  const std::map<Rule::TypeId, std::vector<std::string>> kExpectedRuleTypes{{kTypeA, kValuesA}, {kTypeB, kValuesB}};
  const std::map<Rule::TypeId, std::vector<std::string>> discrete_value_rule_types = dut.DiscreteValueRuleTypes();
  EXPECT_EQ(discrete_value_rule_types.size(), kExpectedRuleTypes.size());
  for (const auto& rule_values : kExpectedRuleTypes) {
    const auto found_rule_values = discrete_value_rule_types.find(rule_values.first);
    EXPECT_NE(found_rule_values, discrete_value_rule_types.end());
    EXPECT_EQ(found_rule_values->second.size(), rule_values.second.size());
    for (const std::string& value : found_rule_values->second) {
      EXPECT_NE(std::find(rule_values.second.begin(), rule_values.second.end(), value), rule_values.second.end());
    }
  }

  // Finds each type.
  drake::optional<RuleRegistry::QueryResult> result = dut.GetPossibleStatesOfRuleType(kTypeA);
  EXPECT_TRUE(result.has_value());
  EXPECT_FALSE(result->range_values.has_value());
  EXPECT_EQ(result->type_id, kTypeA);
  EXPECT_TRUE(result->discrete_values.has_value());
  EXPECT_EQ(result->discrete_values->size(), kValuesA.size());
  for (const std::string& value : *result->discrete_values) {
    EXPECT_NE(std::find(kValuesA.begin(), kValuesA.end(), value), kValuesA.end());
  }

  result = dut.GetPossibleStatesOfRuleType(kTypeB);
  EXPECT_TRUE(result.has_value());
  EXPECT_FALSE(result->range_values.has_value());
  EXPECT_EQ(result->type_id, kTypeB);
  EXPECT_TRUE(result->discrete_values.has_value());
  EXPECT_EQ(result->discrete_values->size(), kValuesB.size());
  for (const std::string& value : *result->discrete_values) {
    EXPECT_NE(std::find(kValuesB.begin(), kValuesB.end(), value), kValuesB.end());
  }

  result = dut.GetPossibleStatesOfRuleType(Rule::TypeId("any_rule_type"));
  EXPECT_FALSE(result.has_value());
}

// Registers RangeValueRules and DiscreteValueRules, then builds rules.
GTEST_TEST(RegisterAndBuildTest, RegisterAndBuild) {
  const Rule::TypeId kRangeValueRuleType("RangeValueRuleType");
  const Rule::Id kRangeRuleId("RangeValueRuleType/RangeRuleId");
  const LaneSRoute kZone({LaneSRange(LaneId("LaneId"), SRange(10., 20.))});
  const RangeValueRule::Range kRange{"range_description", 123. /* min */, 456. /* max */};
  const RangeValueRule::Range kUnregisteredRange{"range_description", 456. /* min */, 789. /* max */};

  const Rule::TypeId kDiscreteValueRuleType("DiscreteValueType");
  const Rule::Id kDiscreteValueRuleId("DiscreteValueType/DiscreteValueRuleId");
  const std::vector<std::string> kDiscreteValues{"Value1", "Value2", "Value3"};

  const Rule::TypeId kUnregisteredRuleType("UnregisteredRuleType");
  const std::string kUnregisteredDiscreteValue{"Value4"};

  RuleRegistry dut;

  dut.RegisterRangeValueRule(kRangeValueRuleType, {kRange});
  dut.RegisterDiscreteValueRule(kDiscreteValueRuleType, kDiscreteValues);

  // Builds and evaluates a RangeValueRule.
  const RangeValueRule range_value_rule =
      dut.BuildRangeValueRule(kRangeRuleId, kRangeValueRuleType, kZone, {} /* related rules */, {kRange});
  EXPECT_EQ(range_value_rule.id(), kRangeRuleId);
  EXPECT_EQ(range_value_rule.type_id(), kRangeValueRuleType);
  EXPECT_EQ(range_value_rule.zone().ranges().size(), 1);
  EXPECT_EQ(range_value_rule.zone().ranges()[0].lane_id(), kZone.ranges()[0].lane_id());
  EXPECT_EQ(range_value_rule.zone().ranges()[0].s_range().s0(), kZone.ranges()[0].s_range().s0());
  EXPECT_EQ(range_value_rule.zone().ranges()[0].s_range().s1(), kZone.ranges()[0].s_range().s1());
  EXPECT_EQ(range_value_rule.related_rules().size(), 0.);
  EXPECT_EQ(range_value_rule.ranges().size(), 1);
  EXPECT_EQ(range_value_rule.ranges().begin()->description, kRange.description);
  EXPECT_EQ(range_value_rule.ranges().begin()->min, kRange.min);
  EXPECT_EQ(range_value_rule.ranges().begin()->max, kRange.max);

  // Unregistered type.
  EXPECT_THROW(
      dut.BuildRangeValueRule(Rule::Id("RuleId"), kUnregisteredRuleType, kZone, {} /* related rules */, {kRange}),
      maliput::common::assertion_error);
  // Unregistered range.
  EXPECT_THROW(dut.BuildRangeValueRule(Rule::Id("RuleId"), kUnregisteredRuleType, kZone, {} /* related rules */,
                                       {kUnregisteredRange}),
               maliput::common::assertion_error);

  // Builds and evaluates a discrete value based rule.
  const DiscreteValueRule discrete_value_rule = dut.BuildDiscreteValueRule(
      kDiscreteValueRuleId, kDiscreteValueRuleType, kZone, {} /* related rules */, {"Value1", "Value3"});
  EXPECT_EQ(discrete_value_rule.id(), kDiscreteValueRuleId);
  EXPECT_EQ(discrete_value_rule.type_id(), kDiscreteValueRuleType);
  EXPECT_EQ(discrete_value_rule.zone().ranges().size(), 1);
  EXPECT_EQ(discrete_value_rule.zone().ranges()[0].lane_id(), kZone.ranges()[0].lane_id());
  EXPECT_EQ(discrete_value_rule.zone().ranges()[0].s_range().s0(), kZone.ranges()[0].s_range().s0());
  EXPECT_EQ(discrete_value_rule.zone().ranges()[0].s_range().s1(), kZone.ranges()[0].s_range().s1());
  EXPECT_EQ(discrete_value_rule.related_rules().size(), 0.);
  EXPECT_EQ(discrete_value_rule.related_rules().size(), 0.);
  EXPECT_EQ(discrete_value_rule.values().size(), 2);
  for (const std::string& discrete_state_value : {"Value1", "Value3"}) {
    EXPECT_NE(std::find(discrete_value_rule.values().begin(), discrete_value_rule.values().end(), discrete_state_value),
              discrete_value_rule.values().end());
  }
  // Unregistered type.
  EXPECT_THROW(dut.BuildDiscreteValueRule(Rule::Id("RuleId"), kUnregisteredRuleType, kZone, {} /* related rules */,
                                          {"Value1", "Value3"}),
               maliput::common::assertion_error);
  // Unregistered discrete value for the type.
  EXPECT_THROW(dut.BuildDiscreteValueRule(kDiscreteValueRuleId, kDiscreteValueRuleType, kZone, {} /* related rules */,
                                          {kUnregisteredDiscreteValue}),
               maliput::common::assertion_error);
}

}  // namespace test
}  // namespace rules
}  // namespace api
}  // namespace maliput