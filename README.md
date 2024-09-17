I created a project that emulates methods used in the game "Star Fox" for the SNES, using OpenGL and GLSL. The project successfully mimics the camera techniques from the game, where the camera's position does not directly map to the position of the model ship character. Instead, it trails behind on the x and y axis, always facing the -z axis. This trailing camera effect, combined with its ability to rotate and pitch with the model, creates a more immersive experience. Additionally, the game replicates the environment of "Star Fox" by incorporating several dimming levels as the ground plane increases in distance along the -z axis and features a textured background far away on the same axis. The project remains faithful to the original game while leveraging modern graphics techniques.
The game also includes a scoring and health system displayed on the screen. The score increases when the player's projectiles, modeled in Blender by me, make contact with and destroy meteors, triggering particle effects. Health is lost if the ship collides with a meteor. This scoring and health mechanic adds an extra layer of challenge and engagement for players. Overall, the project successfully captures the essence of "Star Fox" while introducing a modern graphics API