#include <math.h>
#include <stdio.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_opengl.h>
#include <allegro5/allegro_native_dialog.h>

#if defined _WIN32 || defined __CYGWIN__
#include <windows.h>
#else
#include <sys/time.h>
#endif

#define ALLEGRO_STATICLINK

#define MIN_INTERVAL (1.0 / 60.0)
#define MAX_LATENCY   0.5

#define WIDTH 1024
#define HEIGHT 768

ALLEGRO_BITMAP *cob_background;
ALLEGRO_BITMAP *cob_background_nrm;
ALLEGRO_BITMAP *heli;

ALLEGRO_SHADER *shader;

ALLEGRO_FONT* font;

ALLEGRO_TIMER *timer;
ALLEGRO_TIMEOUT timeout;
ALLEGRO_EVENT_QUEUE *queue;

double start_time;
double current_time;
double target_time;
double last_game_time = 0.0;
double initial_time = 0.0f;

float mouse_x = 0.5;
float mouse_y = 0.5;

int map_x = 0;

int fps_counter = 0;
int fps = 0;

int game_is_running = 1;

float DEFAULT_LIGHT_Z = 0.075f;

// Light position based on mouse cursor
int LIGHT_POS[2] = { WIDTH / 2, HEIGHT / 2 };

//Light RGB and intensity (alpha)
const float LIGHT_COLOR[4] = { 1.0f, 0.8f, 0.6f, 1.0f };

//Ambient RGB and intensity (alpha)
const float AMBIENT_COLOR[4] = { 1.0f, 1.0f, 1.0f, 0.5f };

//Attenuation coefficients for light falloff
const float FALLOFF[3] = { 0.4f, 3.0f, 20.0f };

void abort_example(char const* format, ...)
{
	char str[1024];
	va_list args;
	ALLEGRO_DISPLAY* display;

	va_start(args, format);
	vsnprintf(str, sizeof str, format, args);
	va_end(args);

	if (al_init_native_dialog_addon()) {
		display = al_is_system_installed() ? al_get_current_display() : NULL;
		al_show_native_message_box(display, "Error", "Cannot run example", str, NULL, 0);
	}
	else {
		fprintf(stderr, "%s", str);
	}
	exit(1);
}

class Ship
{
public:
	static float x;
	static float y;

	static float direction;
	static float velocity;
};

float Ship::x = 0;
float Ship::y = 0;
float Ship::direction = 0;
float Ship::velocity = 0;

double get_time()
{
#ifndef WIN32
	struct timeval tv;

	gettimeofday(&tv, 0);

	return tv.tv_sec + tv.tv_usec * 1.0e-6;
#else
	return GetTickCount() / 1000.0;
#endif
}

int video_init()
{
	ALLEGRO_DISPLAY *display = NULL;

	const char* vertex_shader_file;
	const char* pixel_shader_file;

	if (!al_init()) {
		fprintf(stderr, "failed to initialize allegro!\n");
		return -1;
	}

	//al_set_new_display_flags(ALLEGRO_NOFRAME);
	//al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
	al_set_new_display_flags(ALLEGRO_OPENGL | ALLEGRO_PROGRAMMABLE_PIPELINE);

	display = al_create_display(WIDTH, HEIGHT);
	if (!display) {
		fprintf(stderr, "failed to create display!\n");
		return -1;
	}

	al_init_image_addon();
	al_install_keyboard();
	if (!al_install_mouse()) 
	{
		fprintf(stderr, "No mouse support\n");
		return -1;
	}

	al_init_font_addon(); // initialize the font addon
	font = al_create_builtin_font();

	timer = al_create_timer(1.0 / 60);
	queue = al_create_event_queue();
	al_register_event_source(queue, al_get_display_event_source(display));
	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_timer_event_source(timer));
	al_init_timeout(&timeout, 0.12);
	al_start_timer(timer);

	shader = al_create_shader(ALLEGRO_SHADER_GLSL);
	if (!shader) {
		fprintf(stderr, "NO SHADER FOR YOU\n");
		return -1;
	}
	vertex_shader_file = "shaders/ex_prim_shader_vertex.glsl";
	pixel_shader_file = "shaders/ex_prim_shader_pixel.glsl";

	if (!al_attach_shader_source_file(shader, ALLEGRO_VERTEX_SHADER, vertex_shader_file)) {
		abort_example("al_attach_shader_source_file for vertex shader failed: %s\n",
			al_get_shader_log(shader));
	}
	if (!al_attach_shader_source_file(shader, ALLEGRO_PIXEL_SHADER, pixel_shader_file)) {
		abort_example("al_attach_shader_source_file for pixel shader failed: %s\n",
			al_get_shader_log(shader));
	}

	if (!al_build_shader(shader)) {
		abort_example("al_build_shader for link failed: %s\n", al_get_shader_log(shader));
	}

	al_use_shader(shader);
	return 1;
}

int assets_init()
{
	cob_background = al_load_bitmap("assets/cob_back_big.png");
	cob_background_nrm = al_load_bitmap("assets/cob_back_big_NRM.png");
	heli = al_load_bitmap("assets/heli.png");
	return 1;
}

int init_start_values()
{
	return 1;
}

void game_tick(double delta_time)
{
	ALLEGRO_EVENT event;
	al_wait_for_event_until(queue, &event, &timeout);
	if (event.type == ALLEGRO_EVENT_MOUSE_AXES || event.type == ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY)
	{
		mouse_x = event.mouse.x;
		mouse_y = event.mouse.y;
		printf("Mouse position X=%d, Y=%d\n", mouse_x, mouse_y);
	}
	switch (event.type) {
		case ALLEGRO_EVENT_DISPLAY_CLOSE:
			game_is_running = 0;
		break;
		case ALLEGRO_EVENT_KEY_CHAR:
			if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
				game_is_running = 0;
			if (event.keyboard.keycode == ALLEGRO_KEY_W)
				mouse_y += 0.01;
			if (event.keyboard.keycode == ALLEGRO_KEY_S)
				mouse_y -= 0.01;
			if (event.keyboard.keycode == ALLEGRO_KEY_A)
				mouse_x -= 0.01;
			if (event.keyboard.keycode == ALLEGRO_KEY_D)
				mouse_x += 0.01;
		break;
	}
}

void draw_frame(double delta_time, int fps)
{
	
	float lumi[6] = {
		0.6, 0.2, 0.1,
		0.299, 0.587, 0.114,
	};

	float tints[12] = {
		4.0, 0.0, 1.0,
		0.0, 4.0, 1.0,
		1.0, 0.0, 4.0,
		4.0, 4.0, 1.0
	};
	const float resolution[2] = { (float)WIDTH, (float)HEIGHT };
	const float light_pos[3] = { (float)mouse_x, (float)sin(delta_time) + 0.5, 0.03f };

	al_clear_to_color(al_map_rgb(0, 0, 0));

	al_set_shader_sampler("cob_normal", cob_background_nrm, 1);
	al_set_shader_float_vector("Resolution", 2, resolution, 1);
	al_set_shader_float_vector("LightPos", 3, light_pos, 1);
	al_set_shader_float_vector("LightColor", 4, LIGHT_COLOR, 1);
	al_set_shader_float_vector("AmbientColor", 4, AMBIENT_COLOR, 1);
	al_set_shader_float_vector("Falloff", 3, FALLOFF, 1);
	al_draw_bitmap(cob_background, 0, 0, 0);

	al_draw_textf(font, al_map_rgb(255, 255, 255), 200, 40, ALLEGRO_ALIGN_CENTER, "FPS:%d", fps);
	//al_draw_text(NULL, font_color, 10, 10, 0, "SOME TEXT");
	//void al_draw_text(const ALLEGRO_FONT *font,
	//ALLEGRO_COLOR color, float x, float y, int flags,
	//char const *text)

	//al_set_shader_int("test", 1);
	//al_set_shader_float_vector("lum", 3, &lumi[0], 1);
	//al_set_shader_sampler("cob_normal", cob_background_nrm, 1);
	//al_draw_bitmap(cob_background, 0, 0, 0);

	//al_set_shader_int("test", 0);
	//al_draw_bitmap(heli, 200, 200, 0);
	//al_draw_bitmap_region(background_1, 350, 400, 768, 400, 0, 0, 0);

	//al_set_shader_int("test", 1);
	//al_set_shader_float_vector("lum", 3, &lumi[3], 1);
	//al_draw_bitmap(background_1, 0, 775, 0);
	//al_draw_bitmap_region(background_1, 350, 400, 768, 400, 0, 350, 0);

	/*
	//Draw the last one transformed
	ALLEGRO_TRANSFORM trans, backup;
	al_copy_transform(&backup, al_get_current_transform());
	al_identity_transform(&trans);
	al_translate_transform(&trans, 0, 0);
	al_set_shader_float_vector("tint", 3, &tints[2], 1);
	al_use_transform(&trans);
	al_draw_bitmap(background_1, 0, 0, 0);
	al_use_transform(&backup);
	*/
	fps_counter += 1;
}

int main_loop()
{
	initial_time = get_time();
	while (game_is_running)
	{
		current_time = get_time();
		target_time = current_time - start_time;

		/* If the computer's clock has been adjusted backwards,
		* compensate */
		if (target_time < last_game_time)
			start_time = current_time - last_game_time;

		/* If the game time lags too much, for example if the computer
		* has been suspended, avoid trying to catch up */
		if (target_time > last_game_time + MAX_LATENCY)
			last_game_time = target_time - MAX_LATENCY;

		/* If more than MIN_INTERVAL has passed since last update, run
		* game_tick() again. */
		if (target_time >= last_game_time + MIN_INTERVAL)
		{
			//game_tick(target_time - last_game_time);
			last_game_time = target_time;
		}

		if ( (get_time() - initial_time) > 1.0)
		{
			fps = fps_counter;
			fps_counter = 0;
			initial_time = get_time();
		}

		game_tick(target_time - last_game_time);
		draw_frame(target_time - last_game_time, fps);
		al_flip_display();

	}
	return 1;
}

void video_exit()
{
	al_use_shader(NULL);
	al_destroy_shader(shader);
	al_uninstall_system();
}

int main()
{
	video_init();
	assets_init();
	init_start_values();

	fprintf(stderr, "Simple mingw test\n");

	main_loop();
	video_exit();
	return 0;
}