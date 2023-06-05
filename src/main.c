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

void lua_Update(void) {
  lua_pushnumber(L, GetFrameTime());
  lua_setglobal(L, "dt");

  lua_pushnumber(L, GetTime());
  lua_setglobal(L, "time");

  CallFunc("Update");
}

void lua_Draw(void) {
  CallFunc("Draw");
}

void Draw(void) {
  Vector3 pos = {0,0,0};
  DrawModel(models[2], pos, 1, WHITE);

  models[2].meshes[0].vertices[1] += 0.01;
  UpdateMeshBuffer(
    models[2].meshes[0], 
    0,
    models[2].meshes[0].vertices,
    sizeof(float) * models[2].meshes[0].vertexCount * 3,
    0
  );
}

void Update() {
  lua_Update();

  UpdateCamera(&camera, CAMERA_ORBITAL);

    BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

              Draw();

              lua_Draw();
              
              DrawGrid(10, 1.0f);     // Draw a grid

            EndMode3D();
            
            DrawFPS(10, 10);

    EndDrawing();
}

void lua_Init(void) {
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

static Mesh CreateMesh() {
  Mesh m = {0};
  m.triangleCount = 1;
  m.vertexCount = 3;
  m.vertices = MemAlloc(m.vertexCount*3*sizeof(float));    // 3 vertices, 3 coordinates each (x, y, z)
  m.texcoords = MemAlloc(m.vertexCount*2*sizeof(float));   // 3 vertices, 2 coordinates each (x, y)
  m.normals = MemAlloc(m.vertexCount*3*sizeof(float));     // 3 vertices, 3 coordinates each (x, y, z)

  // Vertex at (0, 0, 0)
  m.vertices[0] = 0;
  m.vertices[1] = 0;
  m.vertices[2] = 0;
  m.normals[0] = 0;
  m.normals[1] = 1;
  m.normals[2] = 0;
  m.texcoords[0] = 0;
  m.texcoords[1] = 0;

  // Vertex at (1, 0, 2)
  m.vertices[3] = 1;
  m.vertices[4] = 0;
  m.vertices[5] = 2;
  m.normals[3] = 0;
  m.normals[4] = 1;
  m.normals[5] = 0;
  m.texcoords[2] = 0.5f;
  m.texcoords[3] = 1.0f;

  // Vertex at (2, 0, 0)
  m.vertices[6] = 2;
  m.vertices[7] = 0;
  m.vertices[8] = 0;
  m.normals[6] = 0;
  m.normals[7] = 1;
  m.normals[8] = 0;
  m.texcoords[4] = 1;
  m.texcoords[5] =0;

  UploadMesh(&m, true);

  return m;
}

int main(void) {
  lua_Init();

  const int screenWidth = 640;
  const int screenHeight = 480;

  InitWindow(screenWidth, screenHeight, "raylib [3d]");

  // Define the camera to look into our 3d world
  camera.position = (Vector3){ 5.0f, 5.0f, 5.0f }; // Camera position
  camera.target = (Vector3){ 0.0f, 1.0f, 0.0f };      // Camera looking at point
  camera.up = (Vector3){ 0.0f, 1.6f, 0.0f };          // Camera up vector (rotation towards target)
  camera.fovy = 45.0f;                                // Camera field-of-view Y
  camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

  Image checked = GenImageChecked(2, 2, 1, 1, RED, GREEN);
  Texture2D texture = LoadTextureFromImage(checked);
  UnloadImage(checked);

  models[0] = LoadModel("assets/Car2.obj");         // Load OBJ model
  models[1] = LoadModel("assets/Car3.obj");         // Load OBJ model
  Texture2D texture0 = LoadTexture("assets/car2.png"); // Load model texture
  Texture2D texture1 = LoadTexture("assets/car3.png"); // Load model texture
  models[0].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture0;            // Set model diffuse texture
  models[1].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture1;            // Set model diffuse texture

  Mesh mesh = CreateMesh();
  models[2] = LoadModelFromMesh(mesh);
  models[2].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture0; 

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