/*
 * Cortex-M devices emulation helper.
 *
 * Copyright (c) 2015 Liviu Ionescu.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CORTEXM_HELPER_H_
#define CORTEXM_HELPER_H_

#include "qemu/osdep.h"

#include "hw/qdev-properties.h"
#include "qom/object_helper.h"
#include "qapi/error.h"
#include "qemu/log.h"

/* ------------------------------------------------------------------------- */

#define TYPE_CORTEXM_PREFIX     "cortexm:"

/* ------------------------------------------------------------------------- */

#define DEFINE_PROP_CONST_STRING(_n, _s, _f) \
    DEFINE_PROP(_n, _s, _f, qdev_prop_string, const char*)

#define DEFINE_PROP_NON_VOID_PTR(_n, _s, _f, _t) \
    DEFINE_PROP(_n, _s, _f, qdev_prop_ptr, _t)

#define DEFINE_PROP_INT32_TYPE(_n, _s, _f, _d, _t) \
    DEFINE_PROP_DEFAULT(_n, _s, _f, _d, qdev_prop_int32, _t)

/* ------------------------------------------------------------------------- */

typedef uint64_t peripheral_register_t;

/* ------------------------------------------------------------------------- */

void cm_board_greeting(MachineState *machine);
void *cm_board_init_image(const char *file_name, const char *caption);
const char *cm_board_get_name(MachineState *machine);
const char *cm_board_get_desc(MachineState *machine);

bool cm_mcu_help_func(const char *mcu_device);
bool cm_board_help_func(const char *name);

/* ------------------------------------------------------------------------- */

void cortexm_bitband_init(Object *parent, const char *node_name,
        uint32_t address);

void *cm_cpu_arm_create(Object *parent, const char *cpu_model);

Object *cm_object_new_mcu(MachineState *machine, const char *board_device_name);

void cm_device_reset(DeviceState *dev);

bool cm_device_parent_realize(DeviceState *dev, Error **errp,
        const char *type_name);
bool cm_device_by_name_realize(DeviceState *dev, Error **errp,
        const char *type_name);

Object *cm_container_get_peripheral(void);


static inline Object *cm_object_get_parent             (Object *obj)                                                     __attribute__ ((deprecated));
static inline bool    cm_object_is_instance_of_typename(Object *obj, const char *type_name)                              __attribute__ ((deprecated));
static inline Object *cm_object_get_child_by_name      (Object *obj, const char *name)                                   __attribute__ ((deprecated));
static inline void    cm_object_realize                (Object *obj)                                                     __attribute__ ((deprecated));
static inline Object *cm_object_new                    (Object *parent, const char* node_name, const char *type_name)    __attribute__ ((deprecated));
static inline void    cm_object_property_add_child     (Object *obj, const char *node_name, Object *child)               __attribute__ ((deprecated));
static inline void    cm_object_property_set_int       (Object *obj, int64_t     value, const char *name)                __attribute__ ((deprecated));
static inline void    cm_object_property_set_bool      (Object *obj, bool        value, const char *name)                __attribute__ ((deprecated));
static inline void    cm_object_property_set_str       (Object *obj, const char *value, const char *name)                __attribute__ ((deprecated));
static inline void    cm_object_property_add_str       (Object *obj, const char *name, char          **v)                __attribute__ ((deprecated));
static inline void    cm_object_property_add_const_str (Object *obj, const char *name, const char    **v)                __attribute__ ((deprecated));
static inline void    cm_object_property_add_bool      (Object *obj, const char *name, const bool     *v)                __attribute__ ((deprecated));
static inline void    cm_object_property_add_uint64    (Object *obj, const char *name, const uint64_t *v)                __attribute__ ((deprecated));
static inline void    cm_object_property_add_uint32    (Object *obj, const char *name, const uint32_t *v)                __attribute__ ((deprecated));
static inline void    cm_object_property_add_uint16    (Object *obj, const char *name, const uint16_t *v)                __attribute__ ((deprecated));
static inline void    cm_object_property_add_uint8     (Object *obj, const char *name, const uint8_t  *v)                __attribute__ ((deprecated));
static inline void    cm_object_property_add_int16     (Object *obj, const char *name, const int16_t  *v)                __attribute__ ((deprecated));
static inline void    cm_device_parent_reset           (DeviceState *dev, const char *type_name)                         __attribute__ ((deprecated));
static inline void    cm_device_by_name_reset          (DeviceState *dev, const char *type_name)                         __attribute__ ((deprecated));
static inline Object *cm_object_get_machine            (void)                                                            __attribute__ ((deprecated));

static inline ObjectProperty *
cm_object_property_add(Object *obj, const char *name, const char *type,
        ObjectPropertyAccessor *get, ObjectPropertyAccessor *set,
        ObjectPropertyRelease *release, void *opaque)                    __attribute__ ((deprecated));

static inline Object *cm_object_get_parent             (Object *obj)                                                  { return object_get_parent(obj); }
static inline bool    cm_object_is_instance_of_typename(Object *obj, const char *type_name)                           { return object_is_instance_of_typename(obj, type_name); }
static inline Object *cm_object_get_child_by_name      (Object *obj, const char *name)                                { return object_get_child_by_name(obj, name); }
static inline void    cm_object_realize                (Object *obj)                                                  { object_realize(obj); }
static inline Object *cm_object_new                    (Object *parent, const char* node_name, const char *type_name) { return checked_object_new(parent, node_name, type_name, &error_fatal); }
static inline void    cm_object_property_add_child     (Object *obj, const char *node_name, Object *child)            { object_property_add_child_descriptive(obj, node_name, child, &error_fatal); }
static inline void    cm_object_property_set_int       (Object *obj, int64_t     value, const char *name)             { object_property_set_int_descriptive(  obj, value, name, NULL, &error_fatal); }
static inline void    cm_object_property_set_bool      (Object *obj, bool        value, const char *name)             { object_property_set_bool_descriptive( obj, value, name, NULL, &error_fatal); }
static inline void    cm_object_property_set_str       (Object *obj, const char *value, const char *name)             { object_property_set_str_descriptive(  obj, value, name, NULL, &error_fatal); }
static inline void    cm_object_property_add_str       (Object *obj, const char *name, char          **v)             { object_property_add_str_simple(      obj, name, v, &error_fatal); }
static inline void    cm_object_property_add_const_str (Object *obj, const char *name, const char    **v)             { object_property_add_const_str_simple(obj, name, v, &error_fatal); }
static inline void    cm_object_property_add_bool      (Object *obj, const char *name, const bool     *v)             { object_property_add_bool_simple(     obj, name, v, &error_fatal); }
static inline void    cm_object_property_add_uint64    (Object *obj, const char *name, const uint64_t *v)             { object_property_add_uint64_simple(   obj, name, v, &error_fatal); }
static inline void    cm_object_property_add_uint32    (Object *obj, const char *name, const uint32_t *v)             { object_property_add_uint32_simple(   obj, name, v, &error_fatal); }
static inline void    cm_object_property_add_uint16    (Object *obj, const char *name, const uint16_t *v)             { object_property_add_uint16_simple(   obj, name, v, &error_fatal); }
static inline void    cm_object_property_add_uint8     (Object *obj, const char *name, const uint8_t  *v)             { object_property_add_uint8_simple(    obj, name, v, &error_fatal); }
static inline void    cm_object_property_add_int16     (Object *obj, const char *name, const int16_t  *v)             { object_property_add_int16_simple(    obj, name, v, &error_fatal); }
static inline void    cm_device_parent_reset           (DeviceState *dev, const char *type_name)                      { device_parent_reset( dev, type_name); }
static inline void    cm_device_by_name_reset          (DeviceState *dev, const char *type_name)                      { device_by_name_reset(dev, type_name); }
static inline Object *cm_object_get_machine            (void)                                                         { return qdev_get_machine(); }

static inline ObjectProperty *
cm_object_property_add(Object *obj, const char *name, const char *type,
        ObjectPropertyAccessor *get, ObjectPropertyAccessor *set,
        ObjectPropertyRelease *release, void *opaque)
{
	return  object_property_add(obj, name, type, get, set, release, opaque, &error_fatal);
}



/* ------------------------------------------------------------------------- */

#endif /* CORTEXM_HELPER_H_ */
