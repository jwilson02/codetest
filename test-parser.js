#!/usr/bin/env node
/**
 * Test Script for POE Item Parser
 * Run with: node test-parser.js
 */

const ItemParser = require('./src/services/itemParser');
const { testItems, listTestItems } = require('./src/utils/testData');

const parser = new ItemParser();

console.log('='.repeat(80));
console.log('POE ITEM PARSER TEST');
console.log('='.repeat(80));

// Test all items
const itemTypes = listTestItems();

itemTypes.forEach(itemType => {
  console.log(`\n${'='.repeat(80)}`);
  console.log(`Testing: ${itemType.toUpperCase()}`);
  console.log('='.repeat(80));

  const itemText = testItems[itemType];
  console.log('\nRAW ITEM TEXT:');
  console.log(itemText);

  const parsed = parser.parseItem(itemText);

  if (parsed) {
    console.log('\n--- PARSED RESULT ---');
    console.log(`Rarity: ${parsed.rarity} (Tier ${parsed.rarityTier})`);
    console.log(`Name: ${parsed.name}`);
    console.log(`Base: ${parsed.baseName}`);
    console.log(`Item Class: ${parsed.itemClass || 'N/A'}`);
    console.log(`Item Level: ${parsed.itemLevel || 'N/A'}`);
    console.log(`Quality: ${parsed.quality || 0}%`);
    console.log(`Corrupted: ${parsed.corrupted}`);
    console.log(`Identified: ${parsed.identified}`);

    if (parsed.sockets) {
      console.log(`\nSockets: ${parsed.socketString}`);
      console.log(`Links: ${parsed.links}L`);
      console.log(`Socket Breakdown: R=${parsed.sockets.R} G=${parsed.sockets.G} B=${parsed.sockets.B} W=${parsed.sockets.W}`);
    }

    if (parsed.influences && parsed.influences.length > 0) {
      console.log(`\nInfluences: ${parsed.influences.join(', ')}`);
    }

    if (Object.keys(parsed.requirements).length > 0) {
      console.log('\nRequirements:');
      Object.entries(parsed.requirements).forEach(([key, value]) => {
        console.log(`  ${key}: ${value}`);
      });
    }

    if (parsed.properties.length > 0) {
      console.log('\nProperties:');
      parsed.properties.forEach(prop => {
        console.log(`  ${prop.name}: ${prop.values}`);
      });
    }

    const modTypes = ['implicit', 'explicit', 'crafted', 'fractured', 'enchant'];
    modTypes.forEach(modType => {
      if (parsed.mods[modType] && parsed.mods[modType].length > 0) {
        console.log(`\n${modType.toUpperCase()} Mods:`);
        parsed.mods[modType].forEach(mod => {
          console.log(`  ${mod.text}`);
          if (mod.values.length > 0) {
            console.log(`    Values: ${mod.values.join(', ')}`);
          }
        });
      }
    });

    if (Object.keys(parsed.stats).length > 0) {
      console.log('\nExtracted Stats:');
      Object.entries(parsed.stats).forEach(([key, value]) => {
        console.log(`  ${key}: ${value}`);
      });
    }

    console.log(`\nCategory: ${parser.getItemCategory(parsed)}`);

    console.log('\n✅ PARSING SUCCESSFUL');
  } else {
    console.log('\n❌ PARSING FAILED');
  }
});

console.log('\n' + '='.repeat(80));
console.log('TEST COMPLETE');
console.log('='.repeat(80));
