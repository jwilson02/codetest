# Contributing to SCUMM Style ARPG

Thank you for your interest in contributing to the SCUMM Style ARPG game engine! This document provides guidelines and instructions for contributing to the project.

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [How to Contribute](#how-to-contribute)
- [Development Workflow](#development-workflow)
- [Coding Guidelines](#coding-guidelines)
- [Commit Messages](#commit-messages)
- [Pull Request Process](#pull-request-process)
- [Testing Requirements](#testing-requirements)
- [Documentation](#documentation)
- [Community](#community)

## Code of Conduct

### Our Pledge

We are committed to providing a welcoming and inclusive experience for everyone. We expect all contributors to:

- Be respectful and considerate
- Welcome newcomers and help them learn
- Focus on constructive feedback
- Accept responsibility for mistakes
- Show empathy towards others

### Unacceptable Behavior

- Harassment, discrimination, or offensive comments
- Trolling, insulting, or derogatory remarks
- Personal or political attacks
- Publishing others' private information
- Any conduct inappropriate in a professional setting

## Getting Started

### Prerequisites

Before contributing, ensure you have:

1. **Development Environment Set Up**
   - Read [BUILDING.md](BUILDING.md) for build instructions
   - Read [DEVELOPER.md](DEVELOPER.md) for development guidelines

2. **Familiarity with the Codebase**
   - Explore the source code
   - Read [ARCHITECTURE.md](ARCHITECTURE.md) for system design
   - Run the existing examples

3. **GitHub Account**
   - Fork the repository
   - Set up Git on your machine

### Setting Up Your Development Environment

```bash
# Fork the repository on GitHub, then clone your fork
git clone https://github.com/YOUR_USERNAME/scumm-arpg.git
cd scumm-arpg

# Add upstream remote
git remote add upstream https://github.com/original/scumm-arpg.git

# Create a development branch
git checkout -b feature/my-new-feature

# Install dependencies
./scripts/install_deps.sh  # Linux/macOS
# or
scripts\install_deps_windows.bat  # Windows

# Build the project
./scripts/build_linux.sh  # Linux
./scripts/build_macos.sh  # macOS
scripts\build_windows.bat  # Windows

# Run tests
cd build && ctest
```

## How to Contribute

### Types of Contributions

We welcome various types of contributions:

1. **Bug Fixes**
   - Fix crashes, errors, or unexpected behavior
   - Improve error messages
   - Fix memory leaks or performance issues

2. **New Features**
   - Add new game systems or mechanics
   - Implement requested features from issues
   - Enhance existing systems

3. **Documentation**
   - Improve existing documentation
   - Add code examples
   - Write tutorials or guides
   - Fix typos or clarify explanations

4. **Performance Improvements**
   - Optimize hot paths
   - Reduce memory usage
   - Improve rendering performance

5. **Tests**
   - Add unit tests
   - Add integration tests
   - Improve test coverage

6. **Code Quality**
   - Refactor code for better maintainability
   - Improve code organization
   - Add comments and documentation
   - Fix compiler warnings

### Finding Issues to Work On

1. **Good First Issues**
   - Look for issues labeled `good-first-issue`
   - These are suitable for newcomers

2. **Help Wanted**
   - Check issues labeled `help-wanted`
   - These are issues where we need community help

3. **Bugs**
   - Look for issues labeled `bug`
   - Bug fixes are always appreciated

4. **Feature Requests**
   - Check issues labeled `enhancement`
   - Discuss implementation before starting

## Development Workflow

### 1. Create an Issue (Optional but Recommended)

Before starting work, create or find an issue:

```markdown
**Issue Title**: Add particle system pooling for better performance

**Description**:
Currently, particle systems allocate new particles every frame,
causing performance issues. Implement object pooling to reuse particles.

**Proposed Solution**:
- Create ParticlePool class
- Implement Acquire/Release methods
- Update ParticleSystem to use pool

**Expected Outcome**:
- Reduced allocations in hot path
- Improved FPS in particle-heavy scenes
```

### 2. Fork and Branch

```bash
# Fork the repo on GitHub, then:
git clone https://github.com/YOUR_USERNAME/scumm-arpg.git
cd scumm-arpg
git remote add upstream https://github.com/original/scumm-arpg.git

# Create a feature branch
git checkout -b feature/particle-pooling
```

**Branch Naming Conventions:**
- `feature/feature-name` - New features
- `fix/bug-description` - Bug fixes
- `docs/what-changed` - Documentation updates
- `refactor/what-refactored` - Code refactoring
- `test/what-tested` - Test additions

### 3. Make Changes

Follow the [Coding Guidelines](#coding-guidelines) when making changes.

```bash
# Make your changes
vim src/Rendering/ParticleSystem.cpp

# Test your changes
./scripts/build_linux.sh --debug
./build-linux/bin/SCUMMStyleARPG

# Run tests
cd build-linux && ctest
```

### 4. Commit Changes

Follow the [Commit Message Guidelines](#commit-messages).

```bash
git add src/Rendering/ParticleSystem.cpp
git commit -m "feat: add object pooling to particle system

- Implement ParticlePool class with acquire/release
- Update ParticleSystem to use pool instead of allocating
- Reduce allocations in hot path by 90%
- Add unit tests for particle pooling

Closes #123"
```

### 5. Push and Create Pull Request

```bash
# Push to your fork
git push origin feature/particle-pooling

# Create Pull Request on GitHub
```

## Coding Guidelines

### C++ Style

**Naming Conventions:**

```cpp
// Classes, structs, enums: PascalCase
class PlayerController { };
struct GameConfig { };
enum class ItemType { };

// Functions, methods: PascalCase
void Initialize();
bool IsValid() const;

// Variables: camelCase
int playerHealth;
std::string levelName;

// Member variables: m_camelCase
class MyClass {
    int m_health;
    std::string m_name;
};

// Constants: UPPER_SNAKE_CASE
const int MAX_PLAYERS = 4;
constexpr float PI = 3.14159f;

// Namespaces: PascalCase
namespace Engine { }
namespace Rendering { }
```

**Code Formatting:**

```cpp
// Use 4 spaces for indentation (no tabs)
class Example {
public:
    Example()
        : m_value(0)
        , m_name("default") {
        // Constructor body
    }

    void DoSomething() {
        if (m_value > 0) {
            ProcessValue();
        } else {
            HandleError();
        }

        // Blank line before return
        return;
    }

private:
    int m_value;
    std::string m_name;
};

// Function definitions
void MyFunction(int parameter1,
                const std::string& parameter2,
                bool parameter3) {
    // Implementation
}
```

**Best Practices:**

```cpp
// ✅ DO: Use smart pointers
std::unique_ptr<Entity> entity = std::make_unique<Entity>();
std::shared_ptr<Component> component = std::make_shared<Component>();

// ❌ DON'T: Use raw pointers for ownership
Entity* entity = new Entity();  // Don't do this

// ✅ DO: Use const correctness
void ProcessEntity(const Entity& entity) const;

// ❌ DON'T: Ignore const
void ProcessEntity(Entity& entity);  // If you don't modify it, use const

// ✅ DO: Use nullptr
if (entity == nullptr) { }

// ❌ DON'T: Use NULL or 0
if (entity == NULL) { }  // Don't do this

// ✅ DO: Use auto for complex types
auto iterator = myMap.find(key);
auto transform = entity->GetComponent<TransformComponent>();

// ❌ DON'T: Overuse auto where type is unclear
auto x = GetValue();  // What type is x?

// ✅ DO: Initialize variables
int count = 0;
std::string name = "player";

// ❌ DON'T: Leave variables uninitialized
int count;  // Undefined behavior!
```

### Header Files

```cpp
#pragma once  // Use instead of include guards

// System includes (alphabetically)
#include <memory>
#include <string>
#include <vector>

// Third-party includes (alphabetically)
#include <SDL2/SDL.h>
#include <glm/glm.hpp>

// Project includes (alphabetically)
#include "Engine/Component.h"
#include "Engine/Entity.h"

/**
 * @brief Brief description of the class
 *
 * Detailed description of what this class does,
 * its purpose, and how to use it.
 *
 * @example
 * MyClass obj;
 * obj.DoSomething();
 */
class MyClass {
public:
    /**
     * @brief Constructor description
     * @param value Initial value
     */
    explicit MyClass(int value);

    /**
     * @brief Method description
     * @return Result of operation
     */
    int DoSomething();

private:
    int m_value;  ///< Brief member description
};
```

### Error Handling

```cpp
// ✅ DO: Check return values
if (!texture.Load("player.png")) {
    LOG_ERROR("Failed to load texture: player.png");
    return false;
}

// ✅ DO: Use exceptions for exceptional cases
try {
    config.ParseFile("config.json");
} catch (const std::exception& e) {
    LOG_ERROR("Config parse error: " << e.what());
    // Use default config
}

// ✅ DO: Validate input
bool SetHealth(int health) {
    if (health < 0 || health > MAX_HEALTH) {
        LOG_WARNING("Invalid health value: " << health);
        return false;
    }
    m_health = health;
    return true;
}
```

## Commit Messages

### Format

```
<type>(<scope>): <subject>

<body>

<footer>
```

### Types

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation changes
- `style`: Code style changes (formatting, no logic change)
- `refactor`: Code refactoring
- `perf`: Performance improvements
- `test`: Adding or updating tests
- `chore`: Build process, dependencies, or tooling changes

### Examples

```
feat(rendering): add bloom post-processing effect

- Implement bright pass shader
- Add Gaussian blur for bloom
- Integrate with post-processing pipeline
- Add configurable threshold and intensity

Closes #145
```

```
fix(audio): resolve memory leak in music player

The music player was not properly freeing Mix_Music resources
on track switch. Added proper cleanup in SwitchTrack().

Fixes #198
```

```
docs(api): add examples to component documentation

Added usage examples for:
- Creating custom components
- Component lifecycle
- Inter-component communication

Related to #167
```

### Commit Message Guidelines

1. **Subject Line**
   - Use imperative mood ("add" not "added" or "adds")
   - Limit to 50 characters
   - Don't end with a period
   - Capitalize first letter

2. **Body**
   - Wrap at 72 characters
   - Explain *what* and *why*, not *how*
   - Use bullet points for multiple changes
   - Leave blank line between subject and body

3. **Footer**
   - Reference issues (Closes, Fixes, Related to)
   - Note breaking changes (BREAKING CHANGE:)

## Pull Request Process

### Before Submitting

1. **Code Quality**
   - [ ] Code compiles without errors
   - [ ] All tests pass
   - [ ] No new compiler warnings
   - [ ] Code follows style guidelines
   - [ ] Comments added for complex logic

2. **Testing**
   - [ ] Added/updated unit tests
   - [ ] Manual testing completed
   - [ ] Tested on target platforms

3. **Documentation**
   - [ ] Code comments added
   - [ ] API documentation updated
   - [ ] User documentation updated (if needed)
   - [ ] CHANGELOG.md updated (if applicable)

### Pull Request Template

```markdown
## Description
Brief description of changes.

## Motivation and Context
Why is this change needed? What problem does it solve?
Closes #(issue_number)

## Type of Change
- [ ] Bug fix (non-breaking change fixing an issue)
- [ ] New feature (non-breaking change adding functionality)
- [ ] Breaking change (fix or feature causing existing functionality to change)
- [ ] Documentation update

## How Has This Been Tested?
Describe the tests you ran and the results.

## Screenshots (if applicable)
Add screenshots showing the changes.

## Checklist
- [ ] My code follows the project's style guidelines
- [ ] I have performed a self-review of my code
- [ ] I have commented my code, particularly in hard-to-understand areas
- [ ] I have made corresponding changes to the documentation
- [ ] My changes generate no new warnings
- [ ] I have added tests that prove my fix is effective or that my feature works
- [ ] New and existing unit tests pass locally with my changes
- [ ] Any dependent changes have been merged and published
```

### Review Process

1. **Automated Checks**
   - CI/CD pipeline runs
   - Code style checks
   - Tests must pass

2. **Code Review**
   - At least one maintainer reviews
   - Address feedback promptly
   - Be open to suggestions

3. **Approval and Merge**
   - Once approved, maintainer will merge
   - Delete your feature branch after merge

## Testing Requirements

### Unit Tests

```cpp
#include <gtest/gtest.h>
#include "RPG/CharacterStats.h"

TEST(CharacterStatsTest, InitialLevel) {
    CharacterStats stats;
    EXPECT_EQ(stats.GetLevel(), 1);
}

TEST(CharacterStatsTest, GainExperience) {
    CharacterStats stats;
    stats.GainExperience(100);
    EXPECT_EQ(stats.GetExperience(), 100);
}

TEST(CharacterStatsTest, LevelUp) {
    CharacterStats stats;
    int initialHealth = stats.GetMaxHealth();

    stats.GainExperience(1000);  // Enough for level 2

    EXPECT_EQ(stats.GetLevel(), 2);
    EXPECT_GT(stats.GetMaxHealth(), initialHealth);
}
```

### Integration Tests

```cpp
TEST(GameEngineTest, InitializeAndShutdown) {
    EngineConfig config;
    auto& engine = GameEngine::GetInstance();

    ASSERT_TRUE(engine.Initialize(config));
    engine.Shutdown();
}
```

### Running Tests

```bash
# Build with tests
cmake .. -DBUILD_TESTS=ON
cmake --build .

# Run all tests
ctest --output-on-failure

# Run specific test
./build/bin/CharacterStats_test
```

## Documentation

### Code Documentation

Use Doxygen-style comments:

```cpp
/**
 * @brief Brief description of function
 *
 * Detailed description of what the function does,
 * its behavior, and any important notes.
 *
 * @param parameter1 Description of parameter
 * @param parameter2 Description of parameter
 * @return Description of return value
 * @throws ExceptionType When this exception is thrown
 *
 * @example
 * int result = MyFunction(10, "test");
 */
int MyFunction(int parameter1, const std::string& parameter2);
```

### User Documentation

Update relevant documentation files:
- README.md - For user-facing changes
- docs/BUILDING.md - For build process changes
- docs/DEVELOPER.md - For API or development changes
- docs/ARCHITECTURE.md - For architectural changes

## Community

### Communication Channels

- **GitHub Issues**: Bug reports and feature requests
- **GitHub Discussions**: Questions and general discussion
- **Pull Requests**: Code review and feedback

### Getting Help

If you need help:
1. Check existing documentation
2. Search closed issues
3. Ask in GitHub Discussions
4. Tag maintainers if needed (but please be patient)

### Recognition

Contributors are recognized in:
- GitHub contributors page
- CONTRIBUTORS.md file (if you make significant contributions)
- Release notes

## Questions?

If you have questions about contributing, feel free to:
- Open a discussion on GitHub
- Comment on an existing issue
- Contact the maintainers

Thank you for contributing to SCUMM Style ARPG! 🎮

---

**Remember**: Every contribution, no matter how small, is valuable and appreciated!
