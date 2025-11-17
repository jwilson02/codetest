#!/usr/bin/env node
/**
 * Code Validation Script
 * Validates all JavaScript files for syntax errors and import issues
 * Does not require Electron to be installed
 */

const fs = require('fs');
const path = require('path');
const { execSync } = require('child_process');

// Terminal colors
const colors = {
  reset: '\x1b[0m',
  green: '\x1b[32m',
  red: '\x1b[31m',
  yellow: '\x1b[33m',
  blue: '\x1b[34m',
  bold: '\x1b[1m'
};

class CodeValidator {
  constructor() {
    this.errors = [];
    this.warnings = [];
    this.successes = [];
    this.filesChecked = 0;
  }

  /**
   * Find all JavaScript files in directory
   */
  findJSFiles(dir, fileList = []) {
    const files = fs.readdirSync(dir);

    files.forEach(file => {
      const filePath = path.join(dir, file);
      const stat = fs.statSync(filePath);

      if (stat.isDirectory()) {
        // Skip node_modules, build, dist
        if (!['node_modules', 'build', 'dist', '.git', 'poe-build-guide'].includes(file)) {
          this.findJSFiles(filePath, fileList);
        }
      } else if (file.endsWith('.js') && !file.includes('.test.') && !file.includes('.spec.')) {
        fileList.push(filePath);
      }
    });

    return fileList;
  }

  /**
   * Check syntax of a JavaScript file
   */
  checkSyntax(filePath) {
    try {
      execSync(`node -c "${filePath}"`, { encoding: 'utf8', stdio: 'pipe' });
      return { valid: true };
    } catch (error) {
      return { valid: false, error: error.message };
    }
  }

  /**
   * Check for require/import consistency
   */
  checkImports(filePath) {
    const content = fs.readFileSync(filePath, 'utf8');
    const issues = [];

    // Check for mixed module systems (excluding TypeScript files)
    const hasRequire = /require\(['"]/.test(content);
    const hasImport = /^import .* from ['"]/.test(content);
    const hasExportDefault = /^export default /.test(content);
    const hasExportConst = /^export const /.test(content);
    const hasModuleExports = /module\.exports/.test(content);

    if ((hasImport || hasExportDefault || hasExportConst) && (hasRequire || hasModuleExports)) {
      issues.push('Mixed module systems detected (ES6 and CommonJS)');
    }

    // Check for missing file extensions in imports
    const importMatches = content.match(/require\(['"]\.\.?\/[^'"]+['"]\)/g);
    if (importMatches) {
      importMatches.forEach(match => {
        const importPath = match.match(/['"](.+)['"]/)[1];
        if (!importPath.endsWith('.js') && !importPath.endsWith('.json')) {
          const fullPath = path.resolve(path.dirname(filePath), importPath + '.js');
          if (!fs.existsSync(fullPath)) {
            const fullPathNoExt = path.resolve(path.dirname(filePath), importPath);
            if (!fs.existsSync(fullPathNoExt)) {
              issues.push(`Potential missing import: ${importPath}`);
            }
          }
        }
      });
    }

    return issues;
  }

  /**
   * Validate a single file
   */
  validateFile(filePath) {
    this.filesChecked++;
    const relativePath = path.relative(process.cwd(), filePath);

    // Check syntax
    const syntaxCheck = this.checkSyntax(filePath);
    if (!syntaxCheck.valid) {
      this.errors.push(`${relativePath}: Syntax error - ${syntaxCheck.error}`);
      return false;
    }

    // Check imports
    const importIssues = this.checkImports(filePath);
    if (importIssues.length > 0) {
      importIssues.forEach(issue => {
        this.warnings.push(`${relativePath}: ${issue}`);
      });
    }

    this.successes.push(relativePath);
    return true;
  }

  /**
   * Run validation on all files
   */
  async run() {
    console.log(colors.bold + colors.blue + '\n🔍 Starting Code Validation\n' + colors.reset);

    const rootDir = path.join(__dirname, 'src');
    const electronDir = path.join(__dirname, 'electron');

    console.log('Finding JavaScript files...');
    let files = [];

    if (fs.existsSync(rootDir)) {
      files = files.concat(this.findJSFiles(rootDir));
    }

    if (fs.existsSync(electronDir)) {
      files = files.concat(this.findJSFiles(electronDir));
    }

    console.log(`Found ${files.length} JavaScript files\n`);

    // Validate each file
    files.forEach(file => {
      this.validateFile(file);
    });

    // Print results
    this.printResults();
  }

  /**
   * Print validation results
   */
  printResults() {
    console.log('\n' + colors.bold + '='.repeat(60) + colors.reset);
    console.log(colors.bold + colors.blue + '📊 Validation Results' + colors.reset);
    console.log(colors.bold + '='.repeat(60) + colors.reset + '\n');

    console.log(`${colors.bold}Files Checked:${colors.reset} ${this.filesChecked}`);
    console.log(`${colors.green}✓ Passed:${colors.reset} ${this.successes.length}`);
    console.log(`${colors.yellow}⚠ Warnings:${colors.reset} ${this.warnings.length}`);
    console.log(`${colors.red}✗ Errors:${colors.reset} ${this.errors.length}\n`);

    if (this.errors.length > 0) {
      console.log(colors.red + colors.bold + '❌ ERRORS:' + colors.reset);
      this.errors.forEach(error => {
        console.log(colors.red + '  ✗ ' + error + colors.reset);
      });
      console.log('');
    }

    if (this.warnings.length > 0) {
      console.log(colors.yellow + colors.bold + '⚠️  WARNINGS:' + colors.reset);
      this.warnings.forEach(warning => {
        console.log(colors.yellow + '  ⚠ ' + warning + colors.reset);
      });
      console.log('');
    }

    // Summary
    console.log(colors.bold + '='.repeat(60) + colors.reset);
    if (this.errors.length === 0) {
      console.log(colors.green + colors.bold + '✅ All files passed syntax validation!' + colors.reset);
    } else {
      console.log(colors.red + colors.bold + '❌ Validation failed with errors!' + colors.reset);
    }
    console.log(colors.bold + '='.repeat(60) + colors.reset + '\n');

    // Exit with error code if there are errors
    process.exit(this.errors.length > 0 ? 1 : 0);
  }
}

// Run validation
const validator = new CodeValidator();
validator.run().catch(error => {
  console.error(colors.red + 'Validation error:', error + colors.reset);
  process.exit(1);
});
