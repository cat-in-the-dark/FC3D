#include <stdio.h>
#include <raylib.h>

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif


static Camera camera = { 0 };
static Model model;
static Vector3 modelPos = { 0.0f, 0.0f, 0.0f };

void Update() {
  UpdateCamera(&camera, CAMERA_ORBITAL);

    BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

              DrawModel(model, modelPos, 1.0f, WHITE);
              DrawGrid(10, 1.0f);     // Draw a grid

            EndMode3D();
            
            DrawFPS(10, 10);

    EndDrawing();
}

int main(void) {
  const int screenWidth = 640;
  const int screenHeight = 480;

  InitWindow(screenWidth, screenHeight, "raylib [3d]");

  // Define the camera to look into our 3d world
  camera.position = (Vector3){ 5.0f, 5.0f, 5.0f }; // Camera position
  camera.target = (Vector3){ 0.0f, 1.0f, 0.0f };      // Camera looking at point
  camera.up = (Vector3){ 0.0f, 1.6f, 0.0f };          // Camera up vector (rotation towards target)
  camera.fovy = 45.0f;                                // Camera field-of-view Y
  camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

  model = LoadModel("assets/Car2.obj");         // Load OBJ model
  Texture2D texture = LoadTexture("assets/car2.png"); // Load model texture
  model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;            // Set model diffuse texture

  SetTargetFPS(60); 

  #if defined(PLATFORM_WEB)
  emscripten_set_main_loop(Update, 0, 1);
  #else
  while (!WindowShouldClose()) {
    Update();
  }
  #endif

  UnloadModel(model);         // Unload model
  UnloadTexture(texture);     // Unload texture

  CloseWindow();              // Close window and OpenGL context

  return 0;
}