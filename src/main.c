#include <setjmp.h>
#include <stdbool.h>
#include <string.h>

#include "parser.h"
#define SUPPORT_IMAGE_EXPORT
#include "raylib.h"
#include "utils.h"

#define DEFAULT_SCREEN_WIDTH 800
#define DEFAULT_SCREEN_HEIGHT 600
#define FILE_PATH_CAP 2048
#define FONT_SIZE 48

// TODO: Improve error messages

static Image image;
static Texture2D texture;
static bool have_texture;

static void create_texture_from_xpm_file(const char *xpm_file_path) {
	if (!parse_xpm_file(&image, xpm_file_path))
		have_texture = false;
	else {
		UnloadTexture(texture);
		texture = LoadTextureFromImage(image);
		have_texture = true;
	}
}

int main(int argc, char **argv) {
	char xpm_file_path[FILE_PATH_CAP] = {0};

	if (argc >= 2) {
		strncpy(xpm_file_path, argv[1], FILE_PATH_CAP);
		create_texture_from_xpm_file(xpm_file_path);
	}

	SetTraceLogLevel(LOG_WARNING);
	InitWindow(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, "simplexpm");
	SetWindowState(FLAG_WINDOW_RESIZABLE);

	Font font = LoadFontEx("./fonts/Comic_Sans_MS.ttf", FONT_SIZE, NULL, 0);

	while (!WindowShouldClose()) {
		if (IsFileDropped()) {
			FilePathList file_paths = LoadDroppedFiles();
			strncpy(xpm_file_path, file_paths.paths[0], FILE_PATH_CAP);
			UnloadDroppedFiles(file_paths);
			create_texture_from_xpm_file(xpm_file_path);
		}
		if (IsKeyDown(KEY_R) && have_texture)
			create_texture_from_xpm_file(xpm_file_path);
		if (IsKeyDown(KEY_S) && have_texture) {
			char png_file_path[FILE_PATH_CAP] = {0};
			strncpy(png_file_path, xpm_file_path, strlen(xpm_file_path) - strlen(".xpm"));
			strncat(png_file_path, ".png", strlen(".png"));
			ExportImage(image, png_file_path);
		}

		BeginDrawing();
		ClearBackground(CLITERAL(Color){0xEC, 0xEC, 0xEC, 0xFF});

		int screen_width = GetScreenWidth();
		int screen_height = GetScreenHeight();
		if (have_texture) {
			float scale = screen_width * texture.height > screen_height * texture.width
			            ? (float)screen_height / texture.height
			            : (float)screen_width / texture.width;
			Vector2 position = CLITERAL(Vector2){(screen_width - (texture.width * scale)) / 2,
			                                      (screen_height - (texture.height * scale)) / 2};
			DrawTextureEx(texture, position, 0, scale, WHITE);
		} else {
			Vector2 message_dimensions = MeasureTextEx(font, error_message, FONT_SIZE, 0),
			        message_placement = {
						.x = (screen_width - message_dimensions.x) / 2,
						.y = (screen_height - message_dimensions.y) / 2,
			        };
			DrawTextEx(font, error_message, message_placement, FONT_SIZE, 0, BLACK);
		}
		EndDrawing();
	}

	UnloadTexture(texture);
	CloseWindow();

	return 0;
}
