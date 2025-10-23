# installation


## First launch:

```shell
mkdir build
cd build
cmake ..
make
./launch-sandbox.sh
```


## Notes

Dockable interface

### File (in the top left)
Let you:
- load scenes
- create new scene (probably bugged)
- save project
- import files (will create .gres resource file to be used by the game engine)

### Controls

- Hold right click to move camera
- zqsd to move around
- a and e to go up or down


### Scene panel

- Scene panel show what nodes are in the scene
- Right click in a node to add new one or delete existing one
- Currently no drag and drop to reorganize hierarchy

### Node edit panel

Display info about selected node in the scene

### App panel

for debug tools


## TODO

- EditorLayer uses framebuffer textureID for ImGui. It shouldent happen as the Renderer is in theory the only one making opengl calls
- There might need to be a post processing object
- Modify front end to store view matrix and give it automaticaly to programs when using it