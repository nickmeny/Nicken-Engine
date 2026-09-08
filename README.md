# DESCRIPTION
A Small 2D game engine that was made with raylib and C. The gameplay must be writen in luaJIT.
Right now, the project is in his first steps. I have already implement the ECS Rendering and Movment system. The collision system is underdev.

# RELEASES
Current Release: Nicken version 0.4
New thinks:
    * Components visual editor

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

1. `Engine.create_entity(compoments_table)`: That create a entity with the compoments you want in a table format. The current components are: 
* **position = {x,y}**: For example `position={x=100,y=100}`
* **velocity = {vx,vy}**: for example `velocity = {vx=100,vy=100}`
* **mesh = { type, size = { x,y},color}**: for example `mesh = {type = "rec",size = { x = 50, y = 50 },color = "green"}` \
For more inforamtion see: <a href= "#COMPONENTS">Components</a> \
This return a id for the corresponding entinty.
**IF YOU WANT TO REUSE THE ENTITY YOU HAVE TO SAVE THE ID**

2. **Engine.set_position(id,x,y)**: for example `Engine.set_postion(player,x=100,y=0)`
3. **Engine.set_velocity(id,vx,vy)** : for example `Engine.set_velocity(player,{vx=100,vy=0})`
4. **Engine.IsActionPressed(ActionName)**: for example `Engine.IsActionPressed("MoveRight)` where ActionName is a string, that is defined in the `keys.json`.                                     This func return a *bool* value
5. **Engine.window_init(width,height,title)**: you can define the window as you want ( you can change it from the .nik file)

# COMPONENTS

The Entities have components. It is like a puzzle. You add what components you want to each entity.
1. **position = {x,y}** : Is the position of each entity. It has two integer values: x and y. 
2. **velocity = {vx,vy}** : Is the velocity of each entity.It has two integer values: vx and vy. You put it when you want the engine to update the position for you ( it moves the enity as pixels as the velocity in th ecoresponding axis)
3. **mesh = {type,size{x,y},color}** Is for the engine meshes. The "params" it takes are: 
    * **type**: the type of mesh. there are 2 types right now: `"rec"` and `'circle'`
    * **size**: the size of the mesh. It takes 2 values x and y. **NOTE** If you selecet the type to be circle the x is the radius and the y must be 0.
    * **color**: the color of the mesh. The current colors are: "red","green","blue","black","gray","yellow","white" or any rgb color ( you have to defined it with tables using rgba format, for example: `color={255,0,0,255}`)

# Editor
Compile the program and run it as
```bash
./engine editor
```
It will open a window that you can edit thinks ( UNDER DEVELPMENT)
# VS CODE AUTO COMPLETE
Here is a tutorial to how use the vs code with the engine.
First of all download the extesion <a href = "https://marketplace.visualstudio.com/items?itemName=sumneko.lua">https://marketplace.visualstudio.com/items?itemName=sumneko.lua</a>. Is a lua language server.
In the repo, there is a `engine_api.lua` file. Put it on your work dir if you dont have already. Then, you will get the auto comple.