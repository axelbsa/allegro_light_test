#include <math.h>
#include <stdio.h>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <allegro5/allegro_opengl.h>
#include <allegro5/allegro_native_dialog.h>

#if defined _WIN32 || defined __CYGWIN__
#include <windows.h>
#else
#include <sys/time.h>
#endif

#define ALLEGRO_STATICLINK

#define MIN_INTERVAL (1.0 / 100.0)
#define MAX_LATENCY   0.5

#define WIDTH 1024
#define HEIGHT 768

ALLEGRO_BITMAP *background_1;
ALLEGRO_BITMAP *cob_background;
ALLEGRO_BITMAP *heli;

ALLEGRO_SHADER *shader;

ALLEGRO_TIMER *timer;
ALLEGRO_EVENT_QUEUE *queue;

double start_time;
double current_time;
double target_time;
double last_game_time = 0.0;

int map_x = 0;

int game_is_running = 1;

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
    } else {
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

    if(!al_init()) {
        fprintf(stderr, "failed to initialize allegro!\n");
        return -1;
    }

    //al_set_new_display_flags(ALLEGRO_NOFRAME);
    //al_set_new_display_flags(ALLEGRO_FULLSCREEN_WINDOW);
    al_set_new_display_flags(ALLEGRO_OPENGL | ALLEGRO_PROGRAMMABLE_PIPELINE);

    al_init_image_addon();
    al_install_keyboard();

    display = al_create_display(WIDTH, HEIGHT);
    if(!display) {
        fprintf(stderr, "failed to create display!\n");
        return -1;
    }

    al_clear_to_color(al_map_rgb(0,0,0));
    al_flip_display();

    //al_rest(10.0);
    //al_destroy_display(display);

    timer = al_create_timer(1.0 / 60);
    queue = al_create_event_queue();
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_timer_event_source(timer));
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
    background_1 = al_load_bitmap("assets/bg.png");
    cob_background = al_load_bitmap("assets/cob_back_big.png");
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
    al_wait_for_event(queue, &event);
    switch (event.type) {
        case ALLEGRO_EVENT_DISPLAY_CLOSE:
            game_is_running = 0;
            break;
        case ALLEGRO_EVENT_KEY_CHAR:
            if (event.keyboard.keycode == ALLEGRO_KEY_ESCAPE)
                game_is_running = 0;
            break;
    }
}

void draw_frame() 
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
    al_clear_to_color(al_map_rgb(0, 0, 0));

    al_set_shader_int("test", 1);
    al_set_shader_float_vector("lum", 3, &lumi[0], 1);
    al_draw_bitmap(cob_background, 0, 0, 0);

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
}

int main_loop()
{
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
            game_tick(target_time - last_game_time);
            last_game_time = target_time;
        }

        /* The default target bitmap is the backbuffer of the display.
           We'll draw the image to it and "flip" the backbuffer to make it
           visible. It's better to draw the bitmap inside this loop because
           certain events may cause the display to get "lost," such as minimizing
           and restoring the window. By drawing within the loop, we are 
           safeguarding against that. */
        draw_frame();
        al_flip_display();

    }
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
