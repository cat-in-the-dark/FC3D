#include <stdio.h>
#include <raylib.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#if defined(PLATFORM_DESKTOP)
    #define GLSL_VERSION            330
#else   // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif


static Camera camera = { 0 };
static Model models[128];

static lua_State* L;

void CallFunc(const char* name) {
  lua_getglobal(L, name);
  if (lua_isfunction(L, -1)) {
    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
      const char* msg = lua_tostring(L, -1);
      TraceLog(LOG_ERROR, "lua: %s", msg);
    }
  } else {
    TraceLog(LOG_ERROR, "lua: %s is not a function", name);
  }
}

int lua_DrawModel(lua_State* L) {
  int n = lua_gettop(L);
  if (n == 5) {
    int model_id = luaL_checknumber(L, 1) - 1;
    float x = luaL_checknumber(L, 2);
    float y = luaL_checknumber(L, 3);
    float z = luaL_checknumber(L, 4);
    float scale = luaL_checknumber(L, 5);

    Vector3 pos = {x,y,z};
    Model model = models[model_id]; // TODO: check model_id out of bound

    DrawModel(model, pos, scale, WHITE);
  } else {
    luaL_error(L, "invalid parameters, mdl(idx, x,y,z, scale)");
  }

  return 0;
}

void Update() {
  CallFunc("Update");

  lua_pushnumber(L, GetFrameTime());
  lua_setglobal(L, "dt");

  lua_pushnumber(L, GetTime());
  lua_setglobal(L, "time");

  UpdateCamera(&camera, CAMERA_ORBITAL);

    BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

              CallFunc("Draw");
              DrawGrid(10, 1.0f);     // Draw a grid

            EndMode3D();
            
            DrawFPS(10, 10);

    EndDrawing();
}

void InitLua() {
  L = luaL_newstate();
  luaL_openlibs(L);

  lua_pushnumber(L, 0);
  lua_setglobal(L, "dt");

  lua_pushnumber(L, 0);
  lua_setglobal(L, "time");

  lua_pushcfunction(L, lua_DrawModel);
  lua_setglobal(L, "mdl");

  int err = luaL_dofile(L, "assets/main.lua");
  if (err != LUA_OK) {
    const char * msg = lua_tostring(L, -1);
    TraceLog(LOG_ERROR, "lua: %s", msg);
    return;
  }
  TraceLog(LOG_INFO, "Lua loaded\n");
}

int main(void) {
  InitLua();

  const int screenWidth = 640;
  const int screenHeight = 480;

  InitWindow(screenWidth, screenHeight, "raylib [3d]");

  // Define the camera to look into our 3d world
  camera.position = (Vector3){ 5.0f, 5.0f, 5.0f }; // Camera position
  camera.target = (Vector3){ 0.0f, 1.0f, 0.0f };      // Camera looking at point
  camera.up = (Vector3){ 0.0f, 1.6f, 0.0f };          // Camera up vector (rotation towards target)
  camera.fovy = 45.0f;                                // Camera field-of-view Y
  camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

  models[0] = LoadModel("assets/Car2.obj");         // Load OBJ model
  models[1] = LoadModel("assets/Car3.obj");         // Load OBJ model
  Texture2D texture0 = LoadTexture("assets/car2.png"); // Load model texture
  Texture2D texture1 = LoadTexture("assets/car3.png"); // Load model texture
  models[0].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture0;            // Set model diffuse texture
  models[1].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture1;            // Set model diffuse texture

  SetTargetFPS(60); 

  #if defined(PLATFORM_WEB)
  emscripten_set_main_loop(Update, 0, 1);
  #else
  while (!WindowShouldClose()) {
    Update();
  }
  #endif

  lua_close(L);

  UnloadModel(models[0]);         // Unload model
  UnloadModel(models[1]);         // Unload model
  UnloadTexture(texture0);     // Unload texture
  UnloadTexture(texture1);     // Unload texture

  CloseWindow();              // Close window and OpenGL context

  return 0;
}