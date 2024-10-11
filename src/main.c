#include <setjmp.h>
#include <stdbool.h>
#include <string.h>

#include "parser.h"
#include "raylib.h"
#include "stb_c_lexer.h"
#include "utils.h"

#define FILE_PATH_CAP 2048

int main(int argc, char **argv) {
	char file_path[FILE_PATH_CAP] = {0};
	Texture2D texture = {0};
	bool have_texture = false;

	// Check if a file was given on the command line
	if (argc >= 2) {
		strncpy(file_path, argv[2], FILE_PATH_CAP);
		Image image = {0};
		if (!parse_xpm_file(&image, file_path))
			have_texture = false;
		else {
			UnloadTexture(texture);
			texture = LoadTextureFromImage(image);
			have_texture = true;
		}
	}

	int screenWidth = 800;
	int screenHeight = 600;
	SetTraceLogLevel(LOG_WARNING);
	InitWindow(screenWidth, screenHeight, "simplexpm");

	while (!WindowShouldClose()) {
		if (IsFileDropped()) {
			FilePathList file_paths = LoadDroppedFiles();
			strncpy(file_path, file_paths.paths[0], FILE_PATH_CAP);
			UnloadDroppedFiles(file_paths);
			Image image = {0};
			if (!parse_xpm_file(&image, file_path))
				have_texture = false;
			else {
				UnloadTexture(texture);
				texture = LoadTextureFromImage(image);
				have_texture = true;
			}
		}

		BeginDrawing();
		ClearBackground(RAYWHITE);
		if (have_texture)
            DrawTexture(texture, screenWidth / 2 - texture.width / 2, screenHeight / 2 - texture.height / 2, WHITE);
		else
			DrawText(error_message, GetScreenWidth() / 2, GetScreenHeight() / 2, 24, RED);
		EndDrawing();
	}

	UnloadTexture(texture);
	CloseWindow();

	return 0;
}
