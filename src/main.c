#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
#include <raylib.h>
#include <rlgl.h>
#include <stdio.h>

#if defined(PLATFORM_DESKTOP)
#define GLSL_VERSION 330
#else  // PLATFORM_RPI, PLATFORM_ANDROID, PLATFORM_WEB
#define GLSL_VERSION 100
#endif

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

static Camera camera = {0};
static Model models[128];

static lua_State* L;

static RenderTexture2D target;
static Shader bloom;
static Shader barycentricShader;
static int barycentricLoc;

void DrawLineModel(Model model, Vector3 position, float scale, Color color) {
  float x = 0;
  float y = 0;
  float z = 0;

  Mesh mesh = model.meshes[0];

  rlPushMatrix();
  rlTranslatef(position.x, position.y, position.z);
  rlScalef(scale, scale, scale);

  rlEnableBackfaceCulling();
  rlBegin(RL_LINES);
  rlColor4ub(color.r, color.g, color.b, color.a);

  // TODO: maybe we can load vertices for line in a buffer and drender directly from gpu mem?
  for (int i = 0; i < mesh.triangleCount * 3; i += 3) {
    // unsigned short index1 = mesh.indices[i];
    // unsigned short index2 = mesh.indices[i + 1];
    // unsigned short index3 = mesh.indices[i + 2];
    int index1 = i;
    int index2 = i + 1;
    int index3 = i + 2;

    // Draw lines for each edge of the triangle
    rlVertex3f(mesh.vertices[3 * index1], mesh.vertices[3 * index1 + 1], mesh.vertices[3 * index1 + 2]);
    rlVertex3f(mesh.vertices[3 * index2], mesh.vertices[3 * index2 + 1], mesh.vertices[3 * index2 + 2]);

    rlVertex3f(mesh.vertices[3 * index2], mesh.vertices[3 * index2 + 1], mesh.vertices[3 * index2 + 2]);
    rlVertex3f(mesh.vertices[3 * index3], mesh.vertices[3 * index3 + 1], mesh.vertices[3 * index3 + 2]);

    rlVertex3f(mesh.vertices[3 * index3], mesh.vertices[3 * index3 + 1], mesh.vertices[3 * index3 + 2]);
    rlVertex3f(mesh.vertices[3 * index1], mesh.vertices[3 * index1 + 1], mesh.vertices[3 * index1 + 2]);
  }

  rlDisableBackfaceCulling();
  rlEnd();
  rlPopMatrix();
}

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

    Vector3 pos = {x, y, z};
    Model model = models[model_id];  // TODO: check model_id out of bound

    DrawLineModel(model, pos, scale, RED);
    DrawModel(model, pos, scale * 0.99, BLACK);  // FIXME: the bigger scale the bigger scale offset. not good
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
  // Vector3 pos = {.x = 0, .y = 0, .z = 0};
  // DrawLineModel(models[1], pos, 1.0, RED);
  // DrawModel(models[1], pos, 0.99, BLACK);
}

void Update() {
  lua_Update();

  UpdateCamera(&camera, CAMERA_ORBITAL);

  BeginTextureMode(target);

  ClearBackground(BLACK);

  BeginMode3D(camera);

  // Draw();

  lua_Draw();

  EndMode3D();

  EndTextureMode();

  BeginDrawing();
  ClearBackground(BLACK);

  BeginShaderMode(bloom);
  DrawTextureRec(target.texture, (Rectangle){0, 0, (float) target.texture.width, (float) -target.texture.height},
                 (Vector2){0, 0}, WHITE);
  EndShaderMode();

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
    const char* msg = lua_tostring(L, -1);
    TraceLog(LOG_ERROR, "lua: %s", msg);
    return;
  }
  TraceLog(LOG_INFO, "Lua loaded\n");
}

int calculateBarycentric(float** barycentric, int n_vertices) {
  // For every triangle consisting of 6 vertices (3 points of two coordinates)
  int n = n_vertices / 3;

  // we will have three points of three coordinates, respectively:
  // p_1 = (1, 0, 0), p_2 = (0, 1, 0), p_3 = (0, 0, 1).
  float coords[6] = {
      1, 0, 0, 1, 0, 0,
  };

  TraceLog(LOG_INFO, "calculateBarycentric n=%d vertices=%d", n, n_vertices);
  float* bc = MemAlloc(6 * n * sizeof(float));
  if (!bc) {
    TraceLog(LOG_ERROR, "failed to allocate barycentric coordinates");
    return -1;
  }

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < 6; j++) {
      bc[i * 6 + j] = coords[j];
    }
  }

  *barycentric = bc;
  return n * 6;
}

void applyWireframesMaterial(Model model) {
  float* barycentric;
  int n_barycentric = calculateBarycentric(&barycentric, model.meshes[0].vertexCount);

  rlEnableVertexArray(model.meshes[0].vaoId);
  rlLoadVertexBuffer(barycentric, n_barycentric * sizeof(float), false);
  rlEnableVertexAttribute(barycentricLoc);
  rlSetVertexAttribute(barycentricLoc, 2, RL_FLOAT, false, 0, (void*) 0);
  rlDisableVertexArray();

  model.materials[0].shader = barycentricShader;

  MemFree(barycentric);
}

typedef struct LineMesh {
  int vertexCount;  // Number of vertices stored in arrays
  int linesCount;   // Number of lines stored

  float* vertices;  // Vertex position (XYZ - 3 components per vertex) (shader-location = 0)
  int* lines;

  // TODO: animation data?

  // OpenGL identifiers
  unsigned int vaoId;  // OpenGL Vertex Array Object id
  unsigned int vboId;  // OpenGL Vertex Buffer Objects id (default vertex data)
} LineMesh;

typedef struct LineModel {
  Matrix transform;  // Local transform matrix

  LineMesh* meshes;

  // TODO: animation data?
} LineModel;

int main(void) {
  lua_Init();

  const int screenWidth = 640;
  const int screenHeight = 480;

  InitWindow(screenWidth, screenHeight, "raylib [3d]");

  // char *extensions = (char *)glGetString(GL_EXTENSIONS);

  // Define the camera to look into our 3d world
  camera.position = (Vector3){5.0f, 5.0f, 5.0f};  // Camera position
  camera.target = (Vector3){0.0f, 1.0f, 0.0f};    // Camera looking at point
  camera.up = (Vector3){0.0f, 1.6f, 0.0f};        // Camera up vector (rotation towards target)
  camera.fovy = 45.0f;                            // Camera field-of-view Y
  camera.projection = CAMERA_PERSPECTIVE;         // Camera projection type

  bloom = LoadShader(0, TextFormat("assets/shaders/glsl%i/bloom.fs", GLSL_VERSION));
  barycentricShader = LoadShader(TextFormat("assets/shaders/glsl%i/barycentric.vs", GLSL_VERSION),
                                 TextFormat("assets/shaders/glsl%i/barycentric.fs", GLSL_VERSION));
  barycentricLoc = GetShaderLocationAttrib(barycentricShader, "barycentric");

  target = LoadRenderTexture(screenWidth, screenHeight);
  models[0] = LoadModel("assets/Car2.obj");
  models[1] = LoadModel("assets/Car3.obj");
  models[2] = LoadModel("assets/castle.obj");
  // Texture2D texture0 = LoadTexture("assets/car2.png");
  // Texture2D texture1 = LoadTexture("assets/car3.png");
  // models[0].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture0;
  // models[1].materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture1;

  // applyWireframesMaterial(models[0]);
  // applyWireframesMaterial(models[1]);
  // applyWireframesMaterial(models[2]);

  SetTargetFPS(60);

#if defined(PLATFORM_WEB)
  emscripten_set_main_loop(Update, 0, 1);
#else
  while (!WindowShouldClose()) {
    Update();
  }
#endif

  lua_close(L);

  UnloadModel(models[0]);  // Unload model
  // UnloadModel(models[1]);         // Unload model
  // UnloadTexture(texture0);     // Unload texture
  // UnloadTexture(texture1);     // Unload texture
  UnloadRenderTexture(target);
  UnloadShader(bloom);
  UnloadShader(barycentricShader);

  CloseWindow();  // Close window and OpenGL context

  // MemFree(barycentric);

  return 0;
}