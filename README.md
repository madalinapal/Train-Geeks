# Rail Maintenance 

Rail Maintenance is a 3D railway management and maintenance game built in C++ and OpenGL.  
The player takes control of a small maintenance vehicle responsible for keeping an active railway network operational while trains continuously travel across the map.

As the game progresses, rail segments randomly deteriorate and become unsafe. Damaged tracks block train movement, creating traffic jams and increasing pressure on the player. The objective is to quickly locate broken rail sections, repair them in time, and prevent the railway system from collapsing.

The gameplay combines resource management, navigation, time pressure, and environmental awareness inside a stylized 3D world containing bridges, tunnels, stations, rivers, and mountain areas.

---

# 🎮 Core Gameplay

The railway network is constantly active:

- Trains travel automatically between stations
- Rail segments can randomly become damaged
- Damaged rails stop train movement
- Waiting trains create congestion and increase game difficulty
- The player must navigate the map and repair tracks before the system becomes overloaded

The challenge comes from balancing:
- Fast navigation
- Route planning
- Prioritizing critical repairs
- Managing multiple failures at once

The game progressively becomes harder by increasing:
- Damage frequency
- Number of active failures
- Train traffic intensity

---

#  Main Features

##  Dynamic Train System

- Autonomous train movement
- Continuous railway traffic
- Train stopping behavior at damaged rails
- Station waiting system
- Smooth movement interpolation between rail segments

---

## 🛤️Railway Network

The map contains a complete interconnected railway system featuring:

- Straight rail segments
- Curved tracks
- Bridge sections
- Tunnel sections
- Station areas

Each segment can dynamically switch between:
- Operational state
- Damaged state
- Repaired state

---

##  Damage & Repair Mechanics

Rail failures occur dynamically during gameplay.

Damaged rails:
- Change color
- Become visually deformed
- Block train movement

The player can repair rails by:
1. Reaching the damaged location
2. Positioning near the segment
3. Activating the repair action

Efficient repair management becomes essential as multiple failures start appearing simultaneously.

---

##  Maintenance Vehicle

The player controls a rail maintenance vehicle in third-person view.

Features include:
- Smooth directional movement
- Real-time navigation across the railway
- Collision interaction with repair zones
- Responsive camera tracking

---

##  Environment

The game world includes multiple environmental elements designed to create visual variety and improve spatial awareness:

- Terrain generation
- Rivers and water areas
- Mountains
- Railway bridges
- Tunnel passages
- Train stations

---

##  Collision System

The game uses AABB-based collision detection for:

- Repair interaction zones
- Environment boundaries
- Rail interaction logic
- Train obstruction detection

---

#  Advanced Systems

- Third-person dynamic camera
- Minimap rendering
- HUD displaying:
  - Active damaged rails
  - Gameplay timer
  - Failure state
- Progressive difficulty scaling
- Train congestion logic
- Real-time rail state updates

---

# 🎹 Controls

| Key | Action |
|---|---|
| `W` | Move Forward |
| `S` | Move Backward |
| `A` | Rotate Left |
| `D` | Rotate Right |
| `F` | Repair Rail |
| `ESC` | Exit Game |

---

#  Technologies Used

- C++
- OpenGL
- GLFW
- GLM

---


#  Future Improvements

Possible future extensions include:

- Multiple train types
- Weather effects
- Dynamic day/night cycle
- Procedural railway generation
- Multiplayer maintenance mode
- Resource management system
- AI-controlled maintenance crews

