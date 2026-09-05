---@meta This is a file only for autocomplete in VS Code. It has not any other use 
---@meta

---@class EngineAPI
---@field create_entity fun(config: table): integer
---@field set_position fun(id: integer, x: number, y: number)
---@field get_position fun(id: integer): number, number
---@field set_velocity fun(id: integer, vx: number, vy: number)
---@field is_action_pressed fun(action_name: string): boolean

---@type EngineAPI
Engine = {}


---Creates a new entity in the ECS
---@param config table declaration of components (position, mesh, velocity)
---@return integer entity_id The id of the entity
---@function Engine.create_entity(config) end

---Updates the postion of a entity
---@param id integer Entity ID
---@param x number the x position
---@param y number the y postion
function Engine.set_position(id, x, y) end



---Updates the velocity of an Entity.
---@param id integer Entity ID
---@param vx number The velocity in X axis
---@param vy number The velocity in Y axis
function Engine.set_velocity(id, vx, vy) end

---It Checks if a a action has been pressed
---@param action_name string name of Action (e.χ. "MoveRight")
---@return boolean is_pressed
function Engine.IsActionPressed(action_name) end