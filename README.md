# Klocki Voxel Engine

Engine designed around lock-free, low overhead multithreading.  
Supports translucent blocks, ambient occlusion, infinite world in all axis, collision system, mesh optimization and lazy chunk updating.  
GPU data format optimized to 4 bytes per triangle (inspired by [stb_voxel_render.h](https://github.com/nothings/stb/blob/master/stb_voxel_render.h)).  
Short showcase video: https://youtu.be/km2zESxC4ls

Code is a little bit messy as it is still in development and most of the codebase is in experimentation phase.  
You can see more complaining in [todo.txt](code/todo.txt)


The code should be easy to build. Launch a build script from a command line with MSVC compiler configured.
- debug_build.bat compiles the codebase into platform and application logic layers. Application layer is compiled to .dll and can be reloaded dynamically.
- release_build.bat compiles the whole codebase into single .exe file.


Controls:  
WASD - movement  
Space - jump  

Left mouse - destroy block  
Right mouse - place block  
Mouse scroll - change block in hand  
Second additional mouse button - unlock mouse from the window  

F1 - reset player speed  
F2 - increase player speed  

F3 - debug info  
F3 + 1 - player speed info  
F3 + 2 - show collision visualization  
F3 + 3 - show collision bounding box  
F3 + 4 - show chunk boundaries  
F3 + 5 - highlight translucent block sorting  

F4 - render wireframe  
F5 - fix camera position (useful for testing frustum culling)  
F8 - reset player position  
F9 - toggle player forward movement  
F11 - toggle player flying  

F6 - move test entity in negative direction  
F7 - move test entity in positive direction  
F10 - change axis along which test entity moves  
