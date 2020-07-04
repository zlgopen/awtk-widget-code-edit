#include "tkc/utils.h"
#include "code_edit/code_edit.h"

const char* s_code_edit_properties[] = {CODE_EDIT_PROP_LANG, CODE_EDIT_PROP_FILENAME,
                                        CODE_EDIT_PROP_SHOW_LINE_NUMBER, NULL};

TK_DECL_VTABLE(code_edit) = {.size = sizeof(code_edit_t),
                             .type = WIDGET_TYPE_CODE_EDIT,
                             .clone_properties = s_code_edit_properties,
                             .persistent_properties = s_code_edit_properties,
                             .parent = TK_PARENT_VTABLE(widget),
                             .create = code_edit_create,
                             .on_paint_self = code_edit_on_paint_self,
                             .set_prop = code_edit_set_prop,
                             .get_prop = code_edit_get_prop,
                             .on_event = code_edit_on_event,
                             .on_add_child = code_edit_on_add_child,
                             .on_destroy = code_edit_on_destroy};

widget_t* code_edit_create_internal(widget_t* parent, xy_t x, xy_t y, wh_t w, wh_t h) {
  widget_t* widget = widget_create(parent, TK_REF_VTABLE(code_edit), x, y, w, h);
  code_edit_t* code_edit = CODE_EDIT(widget);
  return_value_if_fail(code_edit != NULL, NULL);

  return widget;
}

widget_t* code_edit_cast(widget_t* widget) {
  return_value_if_fail(WIDGET_IS_INSTANCE_OF(widget, code_edit), NULL);

  return widget;
}
