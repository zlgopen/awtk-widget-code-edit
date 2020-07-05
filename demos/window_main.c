#include "awtk.h"
#include "code_edit/code_edit.h"
#include "code_edit_register.h"

static ret_t on_quit(void* ctx, event_t* e) {
  tk_quit();

  return RET_OK;
}

static ret_t on_load(void* ctx, event_t* e) {
  code_edit_load(WIDGET(ctx), NULL);

  return RET_OK;
}

static ret_t on_save(void* ctx, event_t* e) {
  bool_t with_utf8_bom = widget_get_value(widget_lookup(widget_get_window(WIDGET(ctx)), "utf8_bom", TRUE));

  code_edit_save(WIDGET(ctx), NULL, with_utf8_bom);

  return RET_OK;
}

static ret_t on_redo(void* ctx, event_t* e) {
  code_edit_redo(WIDGET(ctx));

  return RET_OK;
}

static ret_t on_undo(void* ctx, event_t* e) {
  code_edit_undo(WIDGET(ctx));

  return RET_OK;
}

static ret_t on_code_changed(void* ctx, event_t* e) {
  widget_t* code_edit = WIDGET(ctx);
  widget_t* win = widget_get_window(WIDGET(ctx));

  widget_set_enable(widget_lookup(win, "redo", TRUE), code_edit_can_redo(code_edit)); 
  widget_set_enable(widget_lookup(win, "undo", TRUE), code_edit_can_undo(code_edit)); 
  
  return RET_OK;
}

static ret_t on_readonly_changed(void* ctx, event_t* e) {
  bool_t value = widget_get_value(WIDGET(e->target));

  code_edit_set_readonly(WIDGET(ctx), value);

  return RET_OK;
}

static ret_t on_theme_changed(void* ctx, event_t* e) {
  widget_t* code_edit = WIDGET(ctx);
  widget_t* theme_widget = WIDGET(e->target);
  const char* theme = combo_box_get_text(theme_widget);

  code_edit_set_code_theme(code_edit, theme);

  return RET_OK;
}

static ret_t on_show_line_number_changed(void* ctx, event_t* e) {
  bool_t value = widget_get_value(WIDGET(e->target));

  code_edit_set_show_line_number(WIDGET(ctx), value);

  return RET_OK;
}

/**
 * 初始化
 */
ret_t application_init(void) {
  code_edit_register();

  widget_t* win = window_open("main");
  widget_t* code_edit = widget_lookup(win, "code", TRUE);
  widget_child_on(win, "quit", EVT_CLICK, on_quit, code_edit); 
  widget_child_on(win, "load", EVT_CLICK, on_load, code_edit); 
  widget_child_on(win, "save", EVT_CLICK, on_save, code_edit); 
  widget_child_on(win, "redo", EVT_CLICK, on_redo, code_edit); 
  widget_child_on(win, "undo", EVT_CLICK, on_undo, code_edit); 
  widget_child_on(win, "code", EVT_VALUE_CHANGED, on_code_changed, code_edit); 
  widget_child_on(win, "readonly", EVT_VALUE_CHANGED, on_readonly_changed, code_edit); 
  widget_child_on(win, "theme", EVT_VALUE_CHANGED, on_theme_changed, code_edit); 
  widget_child_on(win, "show_line_number", EVT_VALUE_CHANGED, on_show_line_number_changed, code_edit); 

  return RET_OK;
}

/**
 * 退出
 */
ret_t application_exit(void) {
  log_debug("application_exit\n");
  return RET_OK;
}
