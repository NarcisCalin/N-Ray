# Introduction

## N-Ray is a path tracing renderer made for learning purposes by Narcis Calin

### About
I'm making this renderer to learn more about computer graphics. The entire project is made with C++ and GLSL and it is built on top of Raylib.
It has both path tracing and ray marching, meaning it is possible to render conventional scenes as well as fractals and some other cool effects.
The project started being developed for the CPU but it currently runs on the GPU as well, allowing massive performance boost.

### Libraries Used
For this project I have used: 
- Raylib for the base code and for multiple features like loading .hdr images and drawing stuff on screen
- Dear ImGui for the UI
- rlImGui to use ImGui with Raylib
- Embree for fast CPU BVH traversal
- GLM for math

### Features
- CPU and GPU rendering
- Physically based rendering path tracing
- Ray marching which also includes a PBR mode
- Camera parameters simulation
- Debug rays to see how rays bounce around the scene
- Dynamic depth of field by clicking on the scene

### How to build
Currently the best way to build N-Ray is by opening it through Visual Studio. You might need to get some dependencies, but most of them are inside the "external" folder
If you only want to use the ray marcher, then remove the models folder

### Gallery

<img width="1910" height="1051" alt="nrayFractal" src="https://github.com/user-attachments/assets/9c03b071-8e51-459f-a6de-0ed9f290b58a" />

<img width="1905" height="1048" alt="nrayGlass" src="https://github.com/user-attachments/assets/662178cd-6d0f-4956-8cb2-e925253451c8" />

<img width="1911" height="1052" alt="nrayFractal2" src="https://github.com/user-attachments/assets/5e81a221-69d4-4b8a-b6d5-e824591a59e8" />

