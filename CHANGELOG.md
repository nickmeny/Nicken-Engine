# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.4.6] - 2026-10-02
### Added
    1. Add the animation state machine
    2. Add the `play_animation` function for lua
    3. Add the `delete_entity` function for lua
    4. Add the ability to use multy column spritesheets fro textures and animations
    5. Added the Changeog.md file to log the versions
### Changed
    The fowllings files were edited:
        - `lua_bindings.c`
        - `ECS.c`
        - `ECS.h`
        - `ECS_Types.h`
    In the previews version the engine can't handle the multy rows for animations sprites. Now it can. Also in this version, is there, finally, the animation state machine. User can create animations with names ( multiple animations) and cahnge them when it is needed.
### Deprecated
    The lua bindings for auto complete in VS CODE is Deprecated and it needs a update
### Removed
### Fixed 
## [0.4.5] - 2026-10-01

### Added
    1. Add The animation system and the animaiton component
    2. Add the debug for the collisions
    4. Add the flip in the sprite
### Changed
    1. Make micro changes in the rendering
    
    - In this release we worked in animation system. At this time the system is not so good. Maybe it has some bugs or the interface for the lua bindings are ot working as it should( the api is a little bit dificult)
### Deprecated
    The lua bindings for auto complete in VS CODE is Deprecated and it needs a update
### Removed
### Fixed
    Fix some bugs in lua bindings with stack pop/push

## [0.4.4]
### Added
    Nothing new added in this realese
### Changed
    1. Optimize the whole ECS system using ComponentPools instead of bitmasks
    
    In this release we worked hard to optimize the ECS system. Before hand, we had a system that use the `bitmask` method. If you had checked the code
    you had probalby seen sth like 
    
    ```c
    uint32_t mask = COMPOMENT_X | COMPOMENT_Y;
    if ((ecs.entinty_bitmask[i] & mask) != mask) continue;
    ```
    This system was good but as the rendering is going more complex and the entities get more it has a lot of probems.
    One of the problem is the cahce misses. BEcause i have a Big entities tables with a global counter the cpu had a lot of cache misses.
    Now with the new system, using pools for spare and dense tables the cahche is more cpu friendly. Also, i get rid of the vector and the Map
    in the batch rendering. This stop the extra time to malloc/realloc in every frame
### Deprecated
    The lua bindings for auto complete in VS CODE is Deprecated and it needs a update
### Removed
### Fixed
    1.Fix the render so the mesh and the sprite be in one render loop.
    2. Fix some bugs in lua bindings with stack pop/push

## [0.4.3] 
### Added
    1. Sprite draw
    2. collisions sytem
    3. Export button work on editor
### Changed
### Deprecated
    The lua bindings for auto complete in VS CODE is Deprecated and it needs a update
### Removed
### Fixed
    - Create a True batch rendering for the meshes. Previewsly the gpu has to flash sthe shader

## [0.4.0]
### Added 
    1. Components visual editor
### Changed
### Deprecated
### Removed
### Fixed