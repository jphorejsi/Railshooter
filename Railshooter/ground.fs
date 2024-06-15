#version 330 core
in float zCoord; // Input from vertex shader
out vec4 FragColor;

void main()
{
    float dim = 0.9; // Default dimming factor

    // Define distance thresholds and corresponding dimming factors
    if (zCoord < -16.0) {
        dim = 0.1;
    } else if (zCoord < -12.0) {
        dim = 0.2;
    } else if (zCoord < -8.0) {
        dim = 0.3;
    } else if (zCoord < -4.0) {
        dim = 0.4;
    } else if (zCoord < 0.0) {
        dim = 0.5;
    } else if (zCoord < 4.0) {
        dim = 0.6;
    } else if (zCoord < 8.0) {
        dim = 0.7;
    } else if (zCoord < 12.0) {
        dim = 0.8;
    } else if (zCoord < 16.0) {
        dim = 0.9;
    }


    // Set the fragment color based on the dimming factor
    FragColor = vec4(0.306f * dim, 0.309f * dim, 0.325f * dim, 1.0f);
}