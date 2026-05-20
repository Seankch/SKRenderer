## Introduction
SkRenderer is a hybrid ray tracing renderer, built using the Vulkan API. It serves as a base sandbox for future graphics implementations.

## Core features
- Engine architecture designed using Dependency Injection.
- Loading and rendering of models, sub-meshes and textures.
- Dynamic rendering and bindless textures.
- ImGUI integration for editor controls.
- Material system and shader reflection using SPIRV-Reflect.
- Lighting system supporting multiple directional and point lights.
- BLAS, TLAS creation and update, ray traced shadows and reflections.

## Engine architecture
The architecture is designed using dependency injection. Each system's required depedencies are injected through the constructor, allowing the system’s dependencies to be easily visible, improving code clarity, and maintainability.
The graphics system is designed based on Render Hardware Interface (RHI), abstracting the rendering API in its own set of classes. This allows new rendering APIs to be added easily without changing too much of the engine.

## Future plans and improvements
Future plans include implementing DX12, pipeline ray tracing, and exploring ray tracing effects and techniques.

## Screenshots
