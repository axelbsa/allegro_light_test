#include <cstdio>
#include "allegro5/allegro5.h"
#include "allegro5/allegro_image.h"
#include "allegro5/allegro_primitives.h"

#include <allegro5/allegro_native_dialog.h>

/* The ALLEGRO_CFG_* defines are actually internal to Allegro so don't use them
 * in your own programs.
 */
#ifdef ALLEGRO_CFG_D3D
   #include "allegro5/allegro_direct3d.h"
#endif
#ifdef ALLEGRO_CFG_OPENGL
   #include "allegro5/allegro_opengl.h"
#endif

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

static int display_flags = 0;

static void parse_args(int argc, char **argv)
{
   int i;

   for (i = 1; i < argc; i++) {
      if (0 == strcmp(argv[i], "--opengl")) {
         display_flags = ALLEGRO_OPENGL;
         continue;
      }
#ifdef ALLEGRO_CFG_D3D
      if (0 == strcmp(argv[i], "--d3d")) {
         display_flags = ALLEGRO_DIRECT3D;
         continue;
      }
#endif
      abort_example("Unrecognised argument: %s\n", argv[i]);
   }
}

static void choose_shader_source(ALLEGRO_SHADER *shader,
   char const **vsource, char const **psource)
{
   ALLEGRO_SHADER_PLATFORM platform = al_get_shader_platform(shader);
   if (platform == ALLEGRO_SHADER_HLSL) {
      *vsource = "data/ex_shader_vertex.hlsl";
      *psource = "data/ex_shader_pixel.hlsl";
   }
   else if (platform == ALLEGRO_SHADER_GLSL) {
      *vsource = "data/ex_shader_vertex.glsl";
      *psource = "data/ex_shader_pixel.glsl";
   }
   else {
      /* Shouldn't happen. */
      *vsource = NULL;
      *psource = NULL;
   }
}

int main(int argc, char **argv)
{
   ALLEGRO_DISPLAY *display;
   ALLEGRO_BITMAP *bmp;
   ALLEGRO_SHADER *shader;
   const char *vsource;
   const char *psource;

   parse_args(argc, argv);

   if (!al_init()) {
      abort_example("Could not init Allegro.\n");
   }
   al_install_keyboard();
   al_init_image_addon();

   al_set_new_display_flags(ALLEGRO_PROGRAMMABLE_PIPELINE | ALLEGRO_OPENGL);

   display = al_create_display(640, 480);
   if (!display) {
      abort_example("Could not create display.\n");
   }

   bmp = al_load_bitmap("data/mysha.pcx");
   if (!bmp) {
      abort_example("Could not load bitmap.\n");
   }

   shader = al_create_shader(ALLEGRO_SHADER_AUTO);
   if (!shader) {
      abort_example("Could not create shader.\n");
   }

   choose_shader_source(shader, &vsource, &psource);
   if (!vsource|| !psource) {
      abort_example("Could not load source files.\n");
   }

   if (!al_attach_shader_source_file(shader, ALLEGRO_VERTEX_SHADER, vsource)) {
      abort_example("al_attach_shader_source_file failed: %s\n",
         al_get_shader_log(shader));
   }
   if (!al_attach_shader_source_file(shader, ALLEGRO_PIXEL_SHADER, psource)) {
      abort_example("al_attach_shader_source_file failed: %s\n",
         al_get_shader_log(shader));
   }

   if (!al_build_shader(shader)) {
      abort_example("al_build_shader failed: %s\n", al_get_shader_log(shader));
   }

   al_use_shader(shader);

   float tints[12] = {
      4.0, 0.0, 1.0,
      0.0, 4.0, 1.0,
      1.0, 0.0, 4.0,
      4.0, 4.0, 1.0
   };

   while (1) {
      ALLEGRO_KEYBOARD_STATE s;
      al_get_keyboard_state(&s);
      if (al_key_down(&s, ALLEGRO_KEY_ESCAPE))
         break;

      al_clear_to_color(al_map_rgb(140, 40, 40));

      al_set_shader_float_vector("tint", 3, &tints[0], 1);
      al_draw_bitmap(bmp, 0, 0, 0);

      al_set_shader_float_vector("tint", 3, &tints[3], 1);
      al_draw_bitmap(bmp, 320, 0, 0);

      al_set_shader_float_vector("tint", 3, &tints[6], 1);
      al_draw_bitmap(bmp, 0, 240, 0);

      /* Draw the last one transformed */
      ALLEGRO_TRANSFORM trans, backup;
      al_copy_transform(&backup, al_get_current_transform());
      al_identity_transform(&trans);
      al_translate_transform(&trans, 320, 240);
      al_set_shader_float_vector("tint", 3, &tints[9], 1);
      al_use_transform(&trans);
      al_draw_bitmap(bmp, 0, 0, 0);
      al_use_transform(&backup);

      al_flip_display();

      al_rest(0.01);
   }

   al_use_shader(NULL);
   al_destroy_shader(shader);

   return 0;
}
