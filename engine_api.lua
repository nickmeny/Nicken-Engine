---@meta This is a file only for autocomplete in VS Code. It has not any other use 

---@class Engine
Engine = {}


---Creates a new entity in the ECS
---* `config` table declaration of components (position, mesh, velocity)
---@return integer entity_id The id of the entity
function Engine.create_entity(config) end

---Updates the postion of a entity
---* `id` integer Entity ID
---* `x `number the x position
---* `y `number the y postion
function Engine.set_position(id, x, y) end



---Updates the velocity of an Entity.
---*`id` integer Entity ID
---*`vx` number The velocity in X axis
---*`vy` number The velocity in Y axis
function Engine.set_velocity(id, vx, vy) end

---It Checks if a a action has been pressed
---* `action_name` string name of Action (e.χ. "MoveRight")
---@return boolean is_pressed
function Engine.IsActionPressed(action_name) end