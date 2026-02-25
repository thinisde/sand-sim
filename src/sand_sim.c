#include "sand_sim.h"

#include "config.h"
#include "raylib.h"
#include "sand_shader.h"
#include <stddef.h>

static void clearRenderTexture(RenderTexture2D target) {
  BeginTextureMode(target);
  ClearBackground(BLACK);
  EndTextureMode();
}

static void paintBrush(RenderTexture2D target, Vector2 mouse, float groundY,
                       Color color) {
  const float radius = 7.0f;
  if (mouse.x < 0.0f || mouse.x >= (float)W || mouse.y < 0.0f) {
    return;
  }

  if (mouse.y > groundY - 2.0f) {
    mouse.y = groundY - 2.0f;
  }

  BeginTextureMode(target);
  DrawCircleV(mouse, radius, color);
  EndTextureMode();
}

static void runSandPass(RenderTexture2D dst, RenderTexture2D src, Shader shader,
                        int locPassType, int locDiagDx, int passType,
                        int diagDx) {
  const Rectangle srcRect = {0.0f, 0.0f, (float)W, -(float)H};

  SetShaderValue(shader, locPassType, &passType, SHADER_UNIFORM_INT);
  SetShaderValue(shader, locDiagDx, &diagDx, SHADER_UNIFORM_INT);

  BeginTextureMode(dst);
  BeginShaderMode(shader);
  DrawTextureRec(src.texture, srcRect, (Vector2){0.0f, 0.0f}, WHITE);
  EndShaderMode();
  EndTextureMode();
}

static void stepSand(RenderTexture2D **front, RenderTexture2D **back,
                     Shader shader, int locPassType, int locDiagDx,
                     bool frameParity) {
  RenderTexture2D *src = *front;
  RenderTexture2D *dst = *back;

  runSandPass(*dst, *src, shader, locPassType, locDiagDx, 0, 0);
  {
    RenderTexture2D *tmp = src;
    src = dst;
    dst = tmp;
  }

  const int firstDx = frameParity ? 1 : -1;
  runSandPass(*dst, *src, shader, locPassType, locDiagDx, 1, firstDx);
  {
    RenderTexture2D *tmp = src;
    src = dst;
    dst = tmp;
  }

  runSandPass(*dst, *src, shader, locPassType, locDiagDx, 1, -firstDx);
  {
    RenderTexture2D *tmp = src;
    src = dst;
    dst = tmp;
  }

  *front = src;
  *back = dst;
}

int sand_sim_run(void) {
  InitWindow(W, H, "Sand Simulation");
  SetTargetFPS(120);

  const float groundY = H - 80.0f;
  const float dt = 1.0f / 120.0f;
  bool frameParity = false;

  RenderTexture2D simA = LoadRenderTexture(W, H);
  RenderTexture2D simB = LoadRenderTexture(W, H);
  SetTextureFilter(simA.texture, TEXTURE_FILTER_POINT);
  SetTextureFilter(simB.texture, TEXTURE_FILTER_POINT);
  clearRenderTexture(simA);
  clearRenderTexture(simB);

  Shader sandUpdate =
      LoadShaderFromMemory(NULL, sand_update_fragment_shader());
  const int locGroundY = GetShaderLocation(sandUpdate, "uGroundY");
  const int locPassType = GetShaderLocation(sandUpdate, "uPassType");
  const int locDiagDx = GetShaderLocation(sandUpdate, "uDiagDx");
  SetShaderValue(sandUpdate, locGroundY, &groundY, SHADER_UNIFORM_FLOAT);

  RenderTexture2D *front = &simA;
  RenderTexture2D *back = &simB;

  float accumulator = 0.0f;
  while (!WindowShouldClose()) {
    float frame = GetFrameTime();
    if (frame > 0.05f) {
      frame = 0.05f;
    }
    accumulator += frame;

    const bool spawning = IsMouseButtonDown(MOUSE_LEFT_BUTTON);
    const bool erasing = IsMouseButtonDown(MOUSE_RIGHT_BUTTON);
    const bool resetRequested = IsKeyPressed(KEY_R);
    Vector2 mouse = GetMousePosition();

    if (resetRequested) {
      clearRenderTexture(simA);
      clearRenderTexture(simB);
      front = &simA;
      back = &simB;
      accumulator = 0.0f;
    }

    while (accumulator >= dt) {
      if (spawning) {
        paintBrush(*front, mouse, groundY, WHITE);
      }
      if (erasing) {
        paintBrush(*front, mouse, groundY, BLACK);
      }

      stepSand(&front, &back, sandUpdate, locPassType, locDiagDx, frameParity);

      accumulator -= dt;
      frameParity = !frameParity;
    }

    BeginDrawing();
    ClearBackground((Color){18, 18, 26, 255});

    DrawTextureRec(front->texture, (Rectangle){0, 0, (float)W, -(float)H},
                   (Vector2){0, 0}, (Color){214, 181, 96, 255});
    DrawLine(0, (int)groundY, W, (int)groundY, LIGHTGRAY);
    DrawText("LMB: add sand  RMB: remove sand  R: reset", 20, 20, 20,
             RAYWHITE);

    EndDrawing();
  }

  UnloadShader(sandUpdate);
  UnloadRenderTexture(simA);
  UnloadRenderTexture(simB);
  CloseWindow();
  return 0;
}
