#include <setjmp.h>
#include <stdbool.h>
#include <string.h>

#include "parser.h"
#define SUPPORT_IMAGE_EXPORT
#include "raylib.h"
#include "source_code_pro_font.h"
#include "utils.h"

#define DEFAULT_SCREEN_WIDTH 800
#define DEFAULT_SCREEN_HEIGHT 600
#define FILE_PATH_CAP 2048

int main(void) {
	SetTraceLogLevel(LOG_WARNING);
	InitWindow(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, "simplexpm");
	SetWindowState(FLAG_WINDOW_RESIZABLE);
	SetTargetFPS(30);

	Font font = LoadSourceCodeProFont();

	char xpm_file_path[FILE_PATH_CAP] = {0};
	Image image;
	Texture2D texture;
	bool have_texture = false, startup = true;
	while (!WindowShouldClose()) {
		bool isFileDropped = IsFileDropped();
		if (isFileDropped) {
			FilePathList file_paths = LoadDroppedFiles();
			strncpy(xpm_file_path, file_paths.paths[0], FILE_PATH_CAP);
			UnloadDroppedFiles(file_paths);
		}
		if ((IsKeyPressed(KEY_R) && !startup) || isFileDropped) {
			if ((have_texture = parse_xpm_file(&image, xpm_file_path))) {
				UnloadTexture(texture);
				texture = LoadTextureFromImage(image);
			}
			startup = false;
		}
		if (IsKeyPressed(KEY_S) && have_texture) {
			char png_file_path[FILE_PATH_CAP] = {0};
			strncpy(png_file_path, xpm_file_path,
			        strlen(xpm_file_path) - strlen(".xpm"));
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
			Vector2 position = CLITERAL(Vector2){
				(screen_width - (texture.width * scale)) / 2,
			    (screen_height - (texture.height * scale)) / 2
			};
			DrawTextureEx(texture, position, 0, scale, WHITE);
		} else {
			const char *message = startup ? "Drag and drop an XPM file here"
			                              : "Unable to parse XPM file\n"
			                                "(see console for detail)";
			Vector2 message_dimensions = MeasureTextEx(font, message, FONT_SIZE, 0),
			        message_placement = {
			        	.x = (screen_width - message_dimensions.x) / 2,
			        	.y = (screen_height - message_dimensions.y) / 2,
			        };
			DrawTextEx(font, message, message_placement, FONT_SIZE, 0, BLACK);
		}
		EndDrawing();
	}

	UnloadTexture(texture);
	CloseWindow();

	return 0;
}
