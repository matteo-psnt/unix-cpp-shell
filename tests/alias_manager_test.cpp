#include <gtest/gtest.h>
#include "alias_manager.h"
#include <fstream>
#include <cstdio>

class AliasManagerTest : public ::testing::Test {
protected:
    AliasManager manager;
    std::string test_file = "test_aliases.txt";
    
    void TearDown() override {
        // Clean up test file
        std::remove(test_file.c_str());
    }
};

TEST_F(AliasManagerTest, SetAndGetAlias) {
    manager.set_alias("ll", "ls -l");
    
    EXPECT_TRUE(manager.has_alias("ll"));
    EXPECT_EQ(manager.get_alias("ll"), "ls -l");
    EXPECT_FALSE(manager.has_alias("nonexistent"));
    EXPECT_EQ(manager.get_alias("nonexistent"), "");
}

TEST_F(AliasManagerTest, RemoveAlias) {
    manager.set_alias("ll", "ls -l");
    EXPECT_TRUE(manager.has_alias("ll"));
    
    EXPECT_TRUE(manager.remove_alias("ll"));
    EXPECT_FALSE(manager.has_alias("ll"));
    
    // Removing non-existent alias should return false
    EXPECT_FALSE(manager.remove_alias("nonexistent"));
}

TEST_F(AliasManagerTest, ExpandSimpleAlias) {
    manager.set_alias("ll", "ls -l");
    
    std::vector<std::string> tokens = {"ll", "src/"};
    std::vector<std::string> expanded = manager.expand_aliases(tokens);
    
    EXPECT_EQ(expanded.size(), 3);
    EXPECT_EQ(expanded[0], "ls");
    EXPECT_EQ(expanded[1], "-l");
    EXPECT_EQ(expanded[2], "src/");
}

TEST_F(AliasManagerTest, ExpandComplexAlias) {
    manager.set_alias("greet", "echo Hello && echo World");
    
    std::vector<std::string> tokens = {"greet"};
    std::vector<std::string> expanded = manager.expand_aliases(tokens);
    
    EXPECT_EQ(expanded.size(), 5);
    EXPECT_EQ(expanded[0], "echo");
    EXPECT_EQ(expanded[1], "Hello");
    EXPECT_EQ(expanded[2], "&&");
    EXPECT_EQ(expanded[3], "echo");
    EXPECT_EQ(expanded[4], "World");
}

TEST_F(AliasManagerTest, NoAliasExpansion) {
    std::vector<std::string> tokens = {"ls", "-l"};
    std::vector<std::string> expanded = manager.expand_aliases(tokens);
    
    // Should return original tokens when no alias exists
    EXPECT_EQ(expanded, tokens);
}

TEST_F(AliasManagerTest, EmptyTokensList) {
    std::vector<std::string> tokens = {};
    std::vector<std::string> expanded = manager.expand_aliases(tokens);
    
    EXPECT_TRUE(expanded.empty());
}

TEST_F(AliasManagerTest, GetAllAliases) {
    manager.set_alias("ll", "ls -l");
    manager.set_alias("la", "ls -a");
    
    const auto& all_aliases = manager.get_all_aliases();
    
    EXPECT_EQ(all_aliases.size(), 2);
    EXPECT_EQ(all_aliases.at("ll"), "ls -l");
    EXPECT_EQ(all_aliases.at("la"), "ls -a");
}

TEST_F(AliasManagerTest, LoadFromFile) {
    // Create test file
    std::ofstream file(test_file);
    file << "# Test aliases\n";
    file << "ll='ls -l'\n";
    file << "la=\"ls -a\"\n";
    file << "# Comment line\n";
    file << "\n";  // Empty line
    file << "greet=echo Hello World\n";
    file.close();
    
    EXPECT_TRUE(manager.load_aliases_from_file(test_file));
    
    EXPECT_TRUE(manager.has_alias("ll"));
    EXPECT_EQ(manager.get_alias("ll"), "ls -l");
    EXPECT_TRUE(manager.has_alias("la"));
    EXPECT_EQ(manager.get_alias("la"), "ls -a");
    EXPECT_TRUE(manager.has_alias("greet"));
    EXPECT_EQ(manager.get_alias("greet"), "echo Hello World");
}

TEST_F(AliasManagerTest, LoadFromNonexistentFile) {
    EXPECT_FALSE(manager.load_aliases_from_file("nonexistent_file.txt"));
}

TEST_F(AliasManagerTest, SaveToFile) {
    manager.set_alias("ll", "ls -l");
    manager.set_alias("la", "ls -a");
    
    EXPECT_TRUE(manager.save_aliases_to_file(test_file));
    
    // Verify file contents
    std::ifstream file(test_file);
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    
    EXPECT_TRUE(content.find("ll='ls -l'") != std::string::npos);
    EXPECT_TRUE(content.find("la='ls -a'") != std::string::npos);
}

TEST_F(AliasManagerTest, EmptyAliasName) {
    manager.set_alias("", "some command");
    
    EXPECT_FALSE(manager.has_alias(""));
    EXPECT_EQ(manager.get_all_aliases().size(), 0);
}
