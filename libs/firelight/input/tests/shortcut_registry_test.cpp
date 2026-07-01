#include <firelight/input/shortcut_registry.hpp>

#include <gtest/gtest.h>

namespace firelight::input {

class ShortcutRegistryTest : public testing::Test {
protected:
  void SetUp() override { ShortcutRegistry::instance().clear(); }
  void TearDown() override { ShortcutRegistry::instance().clear(); }

  static ShortcutAction make(const char *id, const char *name,
                             const char *category) {
    ShortcutAction action;
    action.id = id;
    action.displayName = name;
    action.category = category;
    return action;
  }
};

TEST_F(ShortcutRegistryTest, RegisterAndGet) {
  auto &registry = ShortcutRegistry::instance();
  registry.registerAction(make("save_state", "Save state", "Save States"));

  const auto *action = registry.getAction("save_state");
  ASSERT_NE(action, nullptr);
  EXPECT_EQ(action->displayName, "Save state");
  EXPECT_EQ(registry.getAction("does_not_exist"), nullptr);
}

TEST_F(ShortcutRegistryTest, RegisterOverwritesById) {
  auto &registry = ShortcutRegistry::instance();
  registry.registerAction(make("ff", "Fast forward", "Speed"));
  registry.registerAction(make("ff", "Turbo", "Speed"));

  ASSERT_NE(registry.getAction("ff"), nullptr);
  EXPECT_EQ(registry.getAction("ff")->displayName, "Turbo");
  EXPECT_EQ(registry.listActions().size(), 1u);
}

TEST_F(ShortcutRegistryTest, ListAndClear) {
  auto &registry = ShortcutRegistry::instance();
  registry.registerAction(make("a", "A", "X"));
  registry.registerAction(make("b", "B", "X"));
  EXPECT_EQ(registry.listActions().size(), 2u);

  registry.clear();
  EXPECT_TRUE(registry.listActions().empty());
}

} // namespace firelight::input
