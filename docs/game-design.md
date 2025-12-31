# City - Game Design Document

## Vision Statement

**City** is a top-down, grid-based multiplayer roleplay game where up to 100 players participate in extended rounds (~3 hours) set in a customizable town environment. Inspired by Space Station 13's emergent gameplay and round structure, City focuses on player interaction, role-based gameplay, and community-driven content through server customization.

## Core Pillars

### 1. Emergent Roleplay
Players are given roles, tools, and a shared space. Gameplay emerges from their interactions rather than scripted events.

### 2. Persistent Sessions
Each round is a self-contained story lasting several hours, allowing deep engagement and meaningful consequences.

### 3. Community Ownership
Servers fork the codebase to create unique experiences. Different servers can be medieval villages, modern cities, sci-fi colonies, etc.

### 4. Accessible Complexity
Simple controls and clear mechanics that combine into complex emergent situations.

---

## Game Structure

### Round Lifecycle

```
┌─────────────────────────────────────────────────────────────────┐
│                         ROUND FLOW                              │
├─────────┬───────────┬─────────────────────────┬────────────────┤
│  LOBBY  │  STARTING │        PLAYING          │    ENDING      │
│         │           │                         │                │
│ Players │ Countdown │ Main gameplay phase     │ Results        │
│ join    │ 10-30 sec │ ~3 hours                │ 30-60 sec      │
│ Pick    │ Assign    │ Events unfold           │ Statistics     │
│ roles   │ spawns    │ Win conditions checked  │ Awards         │
└─────────┴───────────┴─────────────────────────┴────────────────┘
```

### Lobby Phase
- Players connect to server
- Browse available roles
- Ready up
- Server waits for minimum players or admin start

### Starting Phase
- Roles are assigned (random, preference-based, or chosen)
- Players spawn at role-specific locations
- Brief countdown with role information displayed

### Playing Phase
- Main gameplay loop
- Players perform role duties
- Events trigger (random, scheduled, or player-caused)
- Objectives are pursued
- Round ends via:
  - Time limit reached
  - Win condition met
  - Admin intervention

### Ending Phase
- Summary screen
- Statistics displayed
- Awards given (best [role], most [action], etc.)
- Return to lobby

---

## World Design

### Grid System

The world is a 2D grid where each tile is a discrete position:

```
┌───┬───┬───┬───┬───┐
│   │ W │ W │ W │   │  W = Wall (impassable)
├───┼───┼───┼───┼───┤  D = Door
│   │   │   │   │   │  F = Floor (passable)
├───┼───┼───┼───┼───┤
│ W │   │ D │   │ W │
├───┼───┼───┼───┼───┤
│   │   │   │   │   │
├───┼───┼───┼───┼───┤
│   │ W │ W │ W │   │
└───┴───┴───┴───┴───┘
```

### Tile Properties

| Property | Description |
|----------|-------------|
| Floor | Base terrain type (grass, stone, wood, etc.) |
| Wall | Blocks movement and sight |
| Overlay | Decorations, items on ground |
| Flags | Solid, opaque, liquid, stairs |

### Chunks

World is divided into 16x16 tile chunks for:
- Efficient rendering (only visible chunks)
- Network optimization (only sync nearby chunks)
- Memory management (unload distant chunks)

### Zones

Logical areas with special properties:
- **Public**: Town square, streets
- **Restricted**: Private buildings, back rooms
- **Dangerous**: Outskirts, abandoned areas

---

## Roles & Jobs

### Role System Overview

Each player has a role that defines:
- **Spawn location**: Where they start
- **Equipment**: Starting items
- **Access**: Which doors/areas they can enter
- **Objectives**: What they should accomplish (optional)
- **Abilities**: Special actions or permissions

### Example Role Categories

#### Leadership
- **Mayor**: Town administration, can call votes, broadcast announcements
- **Sheriff**: Law enforcement, can arrest, access jail

#### Services
- **Doctor**: Healing, access to clinic
- **Shopkeeper**: Commerce, owns a store
- **Bartender**: Social hub, rumors and information

#### Labor
- **Farmer**: Food production
- **Builder**: Construction and repair
- **Miner**: Resource gathering

#### Special
- **Visitor**: No fixed role, explores and interacts
- **Criminal**: Hidden role, secret objectives (server-defined)

### Role Customization (Server-Defined)

Servers define roles via JSON:

```json
{
  "id": "sheriff",
  "name": "Sheriff",
  "description": "Maintain law and order in the town.",
  "spawn_point": "sheriff_office",
  "equipment": ["badge", "handcuffs", "revolver"],
  "access": ["jail", "sheriff_office", "town_hall"],
  "max_players": 1,
  "objectives": [
    {"type": "maintain", "target": "order"}
  ]
}
```

---

## Player Systems

### Movement

- **Grid-based**: Move one tile at a time
- **8-directional**: Cardinal + diagonal
- **Speed**: Tiles per second (default: 4)
- **Collision**: Cannot enter solid tiles
- **Diagonal blocking**: Cannot cut through wall corners

```
Movement Speed Reference:
- Walking: 4 tiles/sec
- Running: 6 tiles/sec (stamina cost)
- Injured: 2 tiles/sec
- Carrying: 3 tiles/sec
```

### Interaction

Players interact with:
- **Tiles**: Open doors, pick up items
- **Objects**: Use machines, read signs
- **Other players**: Talk, trade, attack (if enabled)

```
Interaction Range:
- Adjacent: 1 tile (most actions)
- Near: 2-3 tiles (conversation)
- Far: Line of sight (observation)
```

### Inventory

Simple slot-based inventory:
- **Hands**: 2 slots (actively held)
- **Pockets**: 2-4 slots (quick access)
- **Backpack**: 6-12 slots (storage)

Items have:
- **Size**: How many slots required
- **Type**: Tool, weapon, consumable, etc.
- **Weight**: Affects movement speed

### Health & Status

| Stat | Description |
|------|-------------|
| Health | Damage taken, 0 = incapacitated |
| Stamina | Running, heavy actions |
| Hunger | Decreases over time, affects stamina |
| Mood | Affects interactions (optional) |

Damage sources:
- Combat (if enabled)
- Environmental (fire, falling, etc.)
- Starvation (extreme hunger)

---

## Communication

### Chat System

Multiple channels for different contexts:

| Channel | Scope | Use |
|---------|-------|-----|
| Local | Nearby players (~5 tiles) | Normal conversation |
| Whisper | Adjacent player | Private, quiet |
| Shout | Large radius (~15 tiles) | Alerts, emergencies |
| Radio | Same frequency holders | Long-range, role-based |
| Global | All players | Announcements (admin) |
| OOC | All players | Out-of-character |

### Emotes & Actions

Visible actions for non-verbal communication:
- *waves*
- *points north*
- *nods*
- *shrugs*

Custom emotes via `/me` command:
```
/me adjusts their hat nervously
> PlayerName adjusts their hat nervously
```

---

## Objects & Items

### Object Types

| Type | Examples | Interaction |
|------|----------|-------------|
| Furniture | Chairs, tables, beds | Sit, sleep, place items |
| Containers | Chests, lockers, crates | Store/retrieve items |
| Machines | Stoves, crafting tables | Perform actions |
| Doors | Regular, locked, airlock | Open, close, lock |
| Signs | Readable text | Read |

### Item Categories

| Category | Examples | Purpose |
|----------|----------|---------|
| Tools | Hammer, wrench, key | Interact with objects |
| Consumables | Food, drinks, medicine | Status effects |
| Weapons | Knife, gun, bat | Combat (if enabled) |
| Materials | Wood, metal, cloth | Crafting |
| Special | ID cards, badges | Access, identification |

### Crafting (Optional)

Server-defined recipes:
```json
{
  "output": "wooden_chair",
  "inputs": ["wood_plank", "wood_plank", "nails"],
  "station": "workbench",
  "time": 10
}
```

---

## Events & Objectives

### Random Events

Servers can define events that trigger during rounds:

| Event | Description |
|-------|-------------|
| Storm | Weather affects outdoor areas |
| Market Day | Increased NPC activity |
| Power Outage | Lights go out, machines stop |
| Visitor | NPC arrives with news/quest |
| Fire | Structure fire starts |

### Objectives

Optional goals for players:

| Type | Example |
|------|---------|
| Maintain | Keep the peace (Sheriff) |
| Produce | Harvest 50 crops (Farmer) |
| Accumulate | Earn 1000 coins (Merchant) |
| Discover | Find all secrets (Explorer) |
| Social | Meet every player (Socialite) |
| Hidden | Sabotage the well (Saboteur) |

### Win Conditions

Servers define how rounds end:

- **Time-based**: Round ends after X hours
- **Objective-based**: Specific goal achieved
- **Elimination**: Last faction standing
- **Vote-based**: Players vote to end

---

## Server Customization

### What Servers Can Customize

| Aspect | Examples |
|--------|----------|
| Theme | Medieval, Western, Modern, Sci-Fi |
| Map | Town layout, buildings, zones |
| Roles | Jobs, access, equipment |
| Items | Custom objects, tools |
| Rules | PvP settings, chat rules |
| Events | Random events, triggers |
| Sprites | Visual appearance |

### Content Structure

```
server_content/
├── manifest.json           # Server metadata
├── sprites/
│   ├── characters/         # Player sprites
│   ├── tiles/              # Floor, walls
│   └── items/              # Object sprites
├── maps/
│   └── town.map            # Map data
├── definitions/
│   ├── roles.json          # Role definitions
│   ├── items.json          # Item definitions
│   └── events.json         # Event definitions
└── scripts/                # Future: custom logic
```

### Server Identity

Each server has:
- **ID**: Unique identifier (e.g., "frontier-town")
- **Name**: Display name (e.g., "Frontier Town RP")
- **Description**: What makes this server unique
- **Rules**: Server-specific rules
- **Version**: Content version for caching

---

## Technical Considerations

### Network Authority

| System | Authority | Reason |
|--------|-----------|--------|
| Movement | Server | Prevent teleporting/speedhacks |
| Inventory | Server | Prevent item duplication |
| Combat | Server | Fair hit detection |
| Chat | Server | Moderation, logging |

### Anti-Cheat Measures

- Server-authoritative movement with validation
- Rate limiting on actions
- Sanity checks on client requests
- Logging for admin review

### Scalability

| Players | Considerations |
|---------|----------------|
| 1-20 | No special handling needed |
| 20-50 | Interest management (only sync nearby) |
| 50-100 | Chunk-based sync, delta compression |
| 100+ | Consider multiple server instances |

---

## Future Considerations

### Potential Features (Not in MVP)

| Feature | Description | Complexity |
|---------|-------------|------------|
| NPCs | AI-controlled characters | Medium |
| Vehicles | Cars, horses, boats | Medium |
| Building | Player construction | High |
| Weather | Rain, snow, day/night | Low |
| Electricity | Power systems | High |
| Economy | Currency, trade | Medium |
| Factions | Team-based gameplay | Medium |

### Modding Support

Future goal: Allow servers to add custom logic via:
- Lua scripting for events
- JSON for data definitions
- Custom components via plugins

---

## Design Principles

### Do

- ✅ Make systems composable
- ✅ Keep core mechanics simple
- ✅ Let complexity emerge from interactions
- ✅ Trust server operators
- ✅ Document everything

### Don't

- ❌ Hardcode game-specific logic in core
- ❌ Require complex client setup
- ❌ Make assumptions about theme/setting
- ❌ Add features without server customization
- ❌ Sacrifice stability for features

---

## Glossary

| Term | Definition |
|------|------------|
| Round | A single game session (~3 hours) |
| Tick | Server update cycle (60/sec) |
| Chunk | 16x16 tile region |
| Role | Player's job/function |
| Entity | Any object in the world |
| Component | Data attached to an entity |
| System | Logic that processes components |
| Authority | Who makes final decisions (server) |
| Prediction | Client guessing before server confirms |
| Reconciliation | Fixing wrong predictions |
