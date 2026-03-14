# Commit Guidelines

## Commit Format
We use a simple and clear format:

[Module Name] Short description of the change

### Examples
- [Firmware] Added temperature sensor init function
- [Docs] Added BOM

## Suggested Module Names
- Firmware
- PCB
- Server
- Web
- Docs
- Images

## Commit Rules
- One commit = one logical change
- The description should be short and clear
- Every commit must begin with a module name in brackets [...]

# Branch Workflow
## Branch structure
We use a three-level branching model:
1. main - stable, production-ready code
2. dev - working branch where unfinished or messy changes may exist
3. feature-example_name - feature branches created from dev
    - Example: feature-error_logging

## Workflow
1. Create a feature branch from dev
2. Work on the feature branch normally
3. When finished, merge feature -> dev
4. When full functionality of the project is ready, merge dev -> main

## Merge Rules
- All merges MUST use --no-ff
- Use Git's default merge commit message
- Prepend the module name in brackets [..]
    - Example: [Firmware] Merge branch 'feature-error_logging' into dev

# Versioning with Tags
We use Git tags to mark stable released versions

## Tag Format
Use semantic versioning:
- vMAJOR.MINOR.PATCH

### Examples:
- v1.0.0 - first stable release
- v1.1.0 - new features
- v1.1.1 - small fixes

## When to Create a Tag
- After merging dev -> main and confirming the code is stable
- When a feature set or milestone is completed
