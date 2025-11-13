#ifndef CHARACTER_CLASS_H
#define CHARACTER_CLASS_H

#include <string>
#include <vector>
#include <map>
#include "CharacterStats.h"

namespace RPG {

/**
 * @brief Manages character classes and specializations
 *
 * Supports 5 main classes: Warrior, Mage, Ranger, Rogue, Paladin
 * Each class has unique abilities, stat bonuses, and playstyle
 */
class CharacterClass {
public:
    enum class Class {
        NONE,
        WARRIOR,
        MAGE,
        RANGER,
        ROGUE,
        PALADIN
    };

    enum class Specialization {
        NONE,
        // Warrior specs
        BERSERKER,      // Offensive warrior
        GUARDIAN,       // Defensive warrior
        // Mage specs
        PYROMANCER,     // Fire magic
        CRYOMANCER,     // Ice magic
        // Ranger specs
        MARKSMAN,       // Precision archery
        BEASTMASTER,    // Pet-focused
        // Rogue specs
        ASSASSIN,       // High burst damage
        SHADOWDANCER,   // Stealth and mobility
        // Paladin specs
        CRUSADER,       // Holy damage
        PROTECTOR       // Support and tanking
    };

    struct ClassDefinition {
        Class classType;
        std::string name;
        std::string description;
        std::string lore;

        // Starting bonuses
        std::map<CharacterStats::PrimaryStat, int> startingStatBonuses;
        std::vector<std::string> startingSkills;
        std::vector<std::string> startingEquipment;

        // Class bonuses (passive)
        std::map<std::string, float> classPassives;

        // Available skill trees
        std::vector<std::string> skillTrees;

        // Available specializations
        std::vector<Specialization> availableSpecs;

        // Unique mechanics
        std::string resourceType; // "Rage", "Mana", "Energy", etc.
        float resourceMax;
        float resourceRegen;

        // Playstyle tags
        std::vector<std::string> playstyleTags; // "Melee", "Ranged", "Tank", "DPS", "Support"
    };

    struct SpecializationDefinition {
        Specialization specType;
        Class parentClass;
        std::string name;
        std::string description;

        // Requirements
        int levelRequirement;
        std::vector<std::string> prerequisiteSkills;

        // Spec bonuses
        std::map<CharacterStats::PrimaryStat, int> statBonuses;
        std::map<std::string, float> specPassives;

        // Unlocked abilities
        std::vector<std::string> uniqueAbilities;
        std::string ultimateAbility;

        // Skill tree modifiers
        std::map<std::string, float> skillTreeBonuses; // Enhances specific skill tree
    };

    CharacterClass();
    ~CharacterClass();

    // Class Management
    void setClass(Class classType);
    Class getClass() const { return m_currentClass; }
    const ClassDefinition* getClassDefinition() const;
    bool canChangeClass() const;

    // Specialization Management
    bool setSpecialization(Specialization spec);
    Specialization getSpecialization() const { return m_currentSpec; }
    const SpecializationDefinition* getSpecDefinition() const;
    bool canSpecialize() const;
    std::vector<Specialization> getAvailableSpecializations() const;

    // Class Information
    static std::string getClassName(Class classType);
    static std::string getSpecializationName(Specialization spec);
    static std::string getClassDescription(Class classType);
    static std::vector<Class> getAllClasses();

    // Class Bonuses
    std::map<CharacterStats::PrimaryStat, int> getTotalStatBonuses() const;
    std::map<std::string, float> getTotalPassives() const;
    std::vector<std::string> getUniqueAbilities() const;

    // Resource System
    struct ResourceInfo {
        std::string type;
        float current;
        float maximum;
        float regen;
    };
    ResourceInfo getResourceInfo() const;
    void modifyResource(float amount);
    void setResource(float amount);

    // Class-specific mechanics
    bool hasClassMechanic(const std::string& mechanic) const;
    float getClassMechanicValue(const std::string& mechanic) const;

    // Multi-classing (for advanced systems)
    bool canMultiClass() const { return m_allowMultiClass; }
    void enableMultiClassing(bool enable) { m_allowMultiClass = enable; }
    std::vector<Class> getSecondaryClasses() const { return m_secondaryClasses; }
    bool addSecondaryClass(Class classType);

    // Prestige Classes (unlocked after prestige)
    bool isPrestigeClass() const { return m_isPrestigeClass; }
    void unlockPrestigeClasses();

    // Save/Load
    std::string serialize() const;
    bool deserialize(const std::string& data);

    // Static class data
    static const ClassDefinition& getClassData(Class classType);
    static const SpecializationDefinition& getSpecData(Specialization spec);
    static void initializeClassData();

private:
    void applyClassBonuses();
    void removeClassBonuses();
    void applySpecBonuses();
    void removeSpecBonuses();

    static std::map<Class, ClassDefinition> s_classDefinitions;
    static std::map<Specialization, SpecializationDefinition> s_specDefinitions;
    static bool s_initialized;

    Class m_currentClass;
    Specialization m_currentSpec;
    float m_currentResource;

    bool m_allowMultiClass;
    std::vector<Class> m_secondaryClasses;
    bool m_isPrestigeClass;

    // For tracking what bonuses have been applied
    std::map<CharacterStats::PrimaryStat, int> m_appliedStatBonuses;
    std::map<std::string, float> m_appliedPassives;
};

} // namespace RPG

#endif // CHARACTER_CLASS_H
