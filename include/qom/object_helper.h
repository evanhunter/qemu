/*
 * QEMU Object Model
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 *
 */

#ifndef QEMU_OBJECT_HELPER_H
#define QEMU_OBJECT_HELPER_H

#include <stdint.h>
#include <stddef.h>
#include "hw/qdev-properties.h"
#include "qemu/typedefs.h"
#include "qom/object.h"

void object_property_set_int_descriptive  (Object *obj, int64_t     value, const char *prop_name, const char *prop_desc, Error **errp);
void object_property_set_bool_descriptive (Object *obj, bool        value, const char *prop_name, const char *prop_desc, Error **errp);
void object_property_set_str_descriptive  (Object *obj, const char *value, const char *prop_name, const char *prop_desc, Error **errp);
void object_property_add_child_descriptive(Object *obj, const char *node_name, Object *child, Error **errp);

Object *checked_object_new(Object *parent, const char* node_name, const char *type_name, Error **errp);

void object_property_add_str_simple      (Object *obj, const char *name, char          **v, Error **errp);
void object_property_add_const_str_simple(Object *obj, const char *name, const char    **v, Error **errp);
void object_property_add_bool_simple     (Object *obj, const char *name, const bool     *v, Error **errp);
void object_property_add_uint64_simple   (Object *obj, const char *name, const uint64_t *v, Error **errp);
void object_property_add_uint32_simple   (Object *obj, const char *name, const uint32_t *v, Error **errp);
void object_property_add_uint16_simple   (Object *obj, const char *name, const uint16_t *v, Error **errp);
void object_property_add_uint8_simple    (Object *obj, const char *name, const uint8_t  *v, Error **errp);
void object_property_add_int16_simple    (Object *obj, const char *name, const int16_t  *v, Error **errp);


#endif /* QEMU_OBJECT_HELPER_H */
