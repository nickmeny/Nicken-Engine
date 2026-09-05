# DESCRIPTION
A Small 2D game engine that was made with raylib and C. The gameplay must be writen in luaJIT.
Right now, the project is in his first steps. I have already implement the ECS Rendering and Movment system. The collision system is underdev.
# How TO COMPILE THE PROGRAM
To compile it, just simple run make run. it will automatically fetch libs from  cmakelist.txt

# Start the game Development
To start, compile the code and after run 
```bash
./engine init
```
that will generate a configuration file in your directory.
You can edit the config file as you want.
Also, it will generate a `keys.json` file, that contains some keybindings.
Then, you are ready to create your first game!!

# Create the first game
<b>Before that, make sure you have succesfully do the <a href="#start-the-game-development">Start the game Development</a> and <a href="#how-to-compile-the-program">How TO COMPILE THE PROGRAM</a></b> \
\

First of all, create a lua script ( example: `main.lua`).
then open it and write down the basic function: 
```lua
function Update(dt)
end
```
To test if everythink works try to write a hello world:
```lua
function Update(dt)
    print("Hello world")
end
```
and run as 
```bash
./engine run main.lua
```
You must get the 
```bash
Hello world
```
massage on the terminal for every frame the game is run

# Engine API

The engine expose functions to use. To use them you have to use the format 
```lua
Engine.function_name(param...)
```
Here is the list of the functions

1. `Engine.create_entity(compoments_table)`: That create a entity with the compoments you want in a table format. The current components are:\
* **position = {x,y}**: For example `position={x=100,y=100}`
* **velocity = {vx,vy}**: for example `velocity = {vx=100,vy=100}`
* **mesh = { type, size = { x,y},color}**: for example `mesh = {type = "rec",size = { x = 50, y = 50 },color = "green"}` \
For more inforamtion see: <a href= "#COMPONENTS">Components</a> \
This return a id for the corresponding entinty.
**IF YOU WANT TO REUSE THE ENTITY YOU HAVE TO SAVE THE ID**

2. **Engine.set_position(id,{x,y})**: for example `Engine.set_postion(player,{x=100,y=0})`
3. **Engine.set_velocity(id,{vx,vy})** : for example `Engine.set_velocity(player,{vx=100,vy=0})`
4. **Engine.IsActionPressed(ActionName)**: for example `Engine.IsActionPressed("MoveRight)` where ActionName is a string, that is defined in the `keys.json`.                                     This func return a *bool* value
5. **Engine.window_init(width,height,title)**: you can define the window as you want ( you can change it from the .nik file)

# COMPONENTS