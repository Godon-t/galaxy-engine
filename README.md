# notes

- EditorLayer uses framebuffer textureID for ImGui. It shouldent happen as the Renderer is in theory the only one making opengl calls
- There might need to be a post processing object
- Modify front end to store view matrix and give it automaticaly to programs when using it