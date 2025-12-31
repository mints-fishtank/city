# City - Content Authoring Guide

This guide explains how to create custom content for City servers, including sprites, maps, items, and roles.

## Overview

City's content system is designed for server customization. Each server can fork the base game and add or modify:

- **Sprites**: Character, tile, and item graphics
- **Maps**: Town layouts and building interiors
- **Items**: Objects players can interact with
- **Roles**: Jobs with specific spawns and equipment
- **Events**: Random occurrences during rounds

## Content Structure

### Directory Layout

```
your_server/
├── city/                      # Base game (git submodule or copy)
│
├── server_content/            # Your custom content
│   ├── manifest.json          # Server metadata
│   ├── sprites/
│   │   ├── characters/
│   │   │   ├── player_base.png
│   │   │   └── player_variants/
│   │   ├── tiles/
│   │   │   ├── floors.png
│   │   │   └── walls.png
│   │   └── items/
│   │       └── tools.png
│   ├── maps/
│   │   ├── town.json
│   │   └── buildings/
│   │       ├── tavern.json
│   │       └── shop.json
│   ├── definitions/
│   │   ├── roles.json
│   │   ├── items.json
│   │   └── events.json
│   └── sounds/                # Future
│       └── ambient/
│
├── CMakeLists.txt             # Build configuration
└── README.md                  # Your server's readme
```

### manifest.json

Every server needs a manifest:

```json
{
    "id": "frontier-town",
    "name": "Frontier Town RP",
    "description": "A Wild West roleplay experience",
    "version": 1,
    "authors": ["YourName"],
    "website": "https://example.com",
    "rules": [
        "Respect other players",
        "Stay in character",
        "No griefing"
    ]
}
```

## Sprites

### Sprite Sheets

Sprites are organized in sprite sheets (multiple sprites in one image):

```
sprite_sheet.png
┌────┬────┬────┬────┐
│ 0  │ 1  │ 2  │ 3  │  Each cell is 32x32 pixels
├────┼────┼────┼────┤
│ 4  │ 5  │ 6  │ 7  │  Sprites indexed left-to-right,
├────┼────┼────┼────┤  top-to-bottom
│ 8  │ 9  │ 10 │ 11 │
└────┴────┴────┴────┘
```

### Character Sprites

Characters use 8-directional sprites with animation frames:

```
characters/player_base.png

Direction Layout (4 frames each):
Row 0: North     [idle, walk1, walk2, walk3]
Row 1: Northeast [idle, walk1, walk2, walk3]
Row 2: East      [idle, walk1, walk2, walk3]
Row 3: Southeast [idle, walk1, walk2, walk3]
Row 4: South     [idle, walk1, walk2, walk3]
Row 5: Southwest [idle, walk1, walk2, walk3]
Row 6: West      [idle, walk1, walk2, walk3]
Row 7: Northwest [idle, walk1, walk2, walk3]
```

### Tile Sprites

Tiles are 32x32 pixels. Include variations for visual variety:

```json
// In definitions/tiles.json
{
    "tiles": [
        {
            "id": "grass",
            "name": "Grass",
            "sprite_sheet": "tiles/terrain.png",
            "variants": [0, 1, 2, 3],
            "flags": []
        },
        {
            "id": "stone_wall",
            "name": "Stone Wall",
            "sprite_sheet": "tiles/walls.png",
            "sprite_index": 0,
            "flags": ["solid", "opaque"]
        }
    ]
}
```

### Item Sprites

Items can be 32x32 (single tile) or larger:

```json
// In definitions/items.json
{
    "items": [
        {
            "id": "hammer",
            "name": "Hammer",
            "sprite_sheet": "items/tools.png",
            "sprite_index": 0,
            "size": 1,
            "type": "tool"
        }
    ]
}
```

## Maps

### Map Format

Maps are JSON files describing tile layout:

```json
{
    "name": "Town Center",
    "width": 64,
    "height": 64,
    "spawn_points": {
        "default": {"x": 32, "y": 32},
        "sheriff": {"x": 10, "y": 20},
        "doctor": {"x": 50, "y": 15}
    },
    "layers": {
        "floor": {
            "default": "grass",
            "tiles": [
                {"x": 10, "y": 10, "tile": "wood_floor"},
                {"x": 11, "y": 10, "tile": "wood_floor"}
            ]
        },
        "walls": {
            "tiles": [
                {"x": 10, "y": 9, "tile": "wood_wall"},
                {"x": 11, "y": 9, "tile": "wood_wall"}
            ]
        },
        "objects": {
            "entities": [
                {"x": 10, "y": 10, "type": "chair"},
                {"x": 15, "y": 15, "type": "table"}
            ]
        }
    },
    "zones": [
        {
            "name": "Sheriff Office",
            "bounds": {"x": 8, "y": 8, "width": 8, "height": 8},
            "access": ["sheriff", "mayor"]
        }
    ]
}
```

### Map Editor

For now, maps are edited manually in JSON. A visual editor is planned for future development.

**Tips for manual editing:**
1. Start with a small area
2. Use a spreadsheet to plan layouts
3. Test frequently in-game
4. Use zones to organize areas

### Regions and Zones

Zones define areas with special properties:

```json
{
    "zones": [
        {
            "name": "Town Square",
            "type": "public",
            "bounds": {"x": 20, "y": 20, "width": 24, "height": 24},
            "properties": {
                "ambient_sound": "crowd_chatter"
            }
        },
        {
            "name": "Jail",
            "type": "restricted",
            "bounds": {"x": 5, "y": 5, "width": 10, "height": 10},
            "access": ["sheriff"],
            "properties": {
                "can_arrest": true
            }
        }
    ]
}
```

## Roles

### Role Definition

Roles define player jobs:

```json
{
    "roles": [
        {
            "id": "sheriff",
            "name": "Sheriff",
            "description": "Maintain law and order in the town.",
            "color": "#FFD700",
            "icon": "roles/sheriff_icon.png",

            "spawn_point": "sheriff",
            "max_players": 1,

            "equipment": [
                {"item": "badge", "slot": "chest"},
                {"item": "revolver", "slot": "hand"},
                {"item": "handcuffs", "slot": "belt"}
            ],

            "access": [
                "sheriff_office",
                "jail",
                "town_hall"
            ],

            "abilities": [
                "arrest",
                "search"
            ],

            "objectives": [
                {
                    "type": "maintain",
                    "target": "order",
                    "description": "Keep the peace"
                }
            ]
        },
        {
            "id": "bartender",
            "name": "Bartender",
            "description": "Run the tavern and serve drinks.",
            "color": "#8B4513",

            "spawn_point": "tavern",
            "max_players": 2,

            "equipment": [
                {"item": "apron", "slot": "chest"},
                {"item": "bottle", "slot": "hand"}
            ],

            "access": [
                "tavern",
                "tavern_cellar"
            ],

            "objectives": [
                {
                    "type": "serve",
                    "target": "drinks",
                    "count": 20,
                    "description": "Serve 20 drinks"
                }
            ]
        }
    ]
}
```

### Role Properties

| Property | Type | Description |
|----------|------|-------------|
| id | string | Unique identifier |
| name | string | Display name |
| description | string | Role description |
| color | hex | Name color |
| icon | path | Role icon sprite |
| spawn_point | string | Named spawn location |
| max_players | number | Max players with this role |
| equipment | array | Starting items |
| access | array | Accessible zone names |
| abilities | array | Special actions |
| objectives | array | Optional goals |

### Hidden Roles

Some roles can be secret:

```json
{
    "id": "saboteur",
    "name": "Visitor",
    "true_name": "Saboteur",
    "hidden": true,
    "description": "Blend in while completing secret objectives.",

    "reveal_conditions": [
        "caught_in_act",
        "round_end"
    ],

    "objectives": [
        {
            "type": "destroy",
            "target": "water_supply",
            "secret": true
        }
    ]
}
```

## Items

### Item Definition

```json
{
    "items": [
        {
            "id": "revolver",
            "name": "Revolver",
            "description": "A six-shooter pistol.",
            "sprite_sheet": "items/weapons.png",
            "sprite_index": 0,

            "type": "weapon",
            "size": 1,
            "weight": 2,

            "properties": {
                "damage": 25,
                "range": 5,
                "ammo_type": "bullet",
                "max_ammo": 6
            },

            "actions": [
                {"name": "shoot", "verb": "shoots"},
                {"name": "reload", "verb": "reloads"}
            ]
        },
        {
            "id": "bandage",
            "name": "Bandage",
            "description": "Stops bleeding and heals minor wounds.",
            "sprite_sheet": "items/medical.png",
            "sprite_index": 0,

            "type": "consumable",
            "size": 1,
            "stackable": true,
            "max_stack": 5,

            "properties": {
                "heal_amount": 15,
                "use_time": 3.0
            },

            "actions": [
                {"name": "use", "verb": "applies", "target": "self"},
                {"name": "apply", "verb": "applies to", "target": "other"}
            ]
        }
    ]
}
```

### Item Properties

| Property | Type | Description |
|----------|------|-------------|
| id | string | Unique identifier |
| name | string | Display name |
| description | string | Item description |
| sprite_sheet | path | Sprite sheet file |
| sprite_index | number | Index in sheet |
| type | string | tool, weapon, consumable, etc. |
| size | number | Inventory slots required |
| weight | number | Affects carry speed |
| stackable | bool | Can stack in inventory |
| max_stack | number | Max stack size |
| properties | object | Type-specific properties |
| actions | array | Available actions |

## Events

### Event Definition

Events add variety to rounds:

```json
{
    "events": [
        {
            "id": "storm",
            "name": "Thunder Storm",
            "description": "A storm rolls in, reducing visibility.",
            "type": "weather",

            "trigger": {
                "type": "random",
                "chance": 0.1,
                "min_time": 1800,
                "max_time": 7200
            },

            "duration": {
                "min": 300,
                "max": 600
            },

            "effects": [
                {"type": "visibility", "value": 0.5},
                {"type": "ambient", "sound": "thunder"}
            ],

            "announcement": {
                "start": "Dark clouds gather overhead...",
                "end": "The storm passes."
            }
        },
        {
            "id": "market_day",
            "name": "Market Day",
            "description": "Merchants gather for trade.",
            "type": "scheduled",

            "trigger": {
                "type": "time",
                "at": 3600
            },

            "duration": {
                "fixed": 1800
            },

            "spawns": [
                {"type": "npc", "id": "merchant", "count": 3, "zone": "market"}
            ],

            "announcement": {
                "start": "The weekly market is now open!",
                "end": "The merchants pack up their wares."
            }
        }
    ]
}
```

### Event Types

| Type | Trigger | Description |
|------|---------|-------------|
| weather | random | Weather changes |
| scheduled | time | At specific round time |
| triggered | condition | When condition met |
| player | action | Player-caused |

## Building a Server

### Step 1: Fork the Repository

```bash
# Clone your fork
git clone https://github.com/you/city-server.git
cd city-server

# Add base game as submodule
git submodule add https://github.com/original/city.git city
```

### Step 2: Create Content Directory

```bash
mkdir -p server_content/{sprites,maps,definitions,sounds}
```

### Step 3: Create Manifest

```bash
cat > server_content/manifest.json << 'EOF'
{
    "id": "my-server",
    "name": "My City Server",
    "description": "A custom City experience",
    "version": 1
}
EOF
```

### Step 4: Add Content

Add your sprites, maps, and definitions as described above.

### Step 5: Configure Build

Create `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.21)
project(my_city_server)

set(CONTENT_DIR "${CMAKE_SOURCE_DIR}/city/content")
set(SERVER_CONTENT_DIR "${CMAKE_SOURCE_DIR}/server_content")

set(BUILD_CLIENT OFF CACHE BOOL "" FORCE)
set(BUILD_SERVER ON CACHE BOOL "" FORCE)

add_subdirectory(city)
```

### Step 6: Build and Run

```bash
cmake --preset server-debug
cmake --build --preset server-debug
./build/server-debug/city_server
```

## Content Best Practices

### Sprites

- Use 32x32 pixel tiles (or multiples)
- Use PNG format with transparency
- Keep file sizes reasonable (compress when possible)
- Use consistent color palettes
- Include source files (PSD, etc.) in a separate folder

### Maps

- Start small, expand later
- Test navigation paths
- Ensure spawn points are accessible
- Don't over-detail early on
- Keep backup copies

### Definitions

- Use descriptive IDs (not just numbers)
- Write clear descriptions
- Balance items and roles through playtesting
- Document any complex logic

### Organization

- Use consistent naming conventions
- Group related content in subdirectories
- Version control everything
- Keep a changelog

## Testing Content

### In-Game Testing

```bash
# Start server with verbose logging
./city_server --verbose

# Connect with client
./city_client --server localhost
```

### Validation

Before deploying, validate:
1. All referenced files exist
2. JSON is valid syntax
3. Sprite indices are within bounds
4. Spawn points exist on maps
5. Access zones match role definitions

### Common Issues

| Issue | Solution |
|-------|----------|
| Missing sprite | Check file path and sprite_index |
| Can't spawn | Verify spawn_point exists in map |
| Zone inaccessible | Check zone bounds in map |
| Item not appearing | Verify equipment slot names |

## Distribution

### Hosting Your Server

1. Build release version:
   ```bash
   cmake --preset server-release
   cmake --build --preset server-release
   ```

2. Package:
   ```bash
   ./city_server
   ./server_content/
   ```

3. Configure firewall:
   ```bash
   sudo ufw allow 7777/udp
   ```

### Client Updates

Clients automatically download content when connecting. Increment `version` in manifest.json when updating content.

## Future Plans

- Visual map editor
- Sprite sheet helper tools
- Content validation tool
- Hot reload during development
- Lua scripting for custom logic
