#include "menu_header.h"

void menu_section_header_draw(GContext *ctx, const Layer *cell_layer, char* text)
{
  GRect bounds = layer_get_bounds(cell_layer);
  GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);

  #ifdef PBL_ROUND
    GTextAlignment alignment = GTextAlignmentCenter;
  #else
    GTextAlignment alignment = GTextAlignmentLeft;
  #endif

  graphics_draw_text(ctx, text, font, bounds, GTextOverflowModeTrailingEllipsis, alignment, NULL);
}