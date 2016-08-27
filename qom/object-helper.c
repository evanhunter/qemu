/*
 * QEMU Object Model helpers.
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

#include "config-host.h"

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qom/object.h"
#include "qom/object_interfaces.h"
#include "qemu/cutils.h"
#include "qapi/visitor.h"
#include "qapi-visit.h"
#include "qapi/string-input-visitor.h"
#include "qapi/string-output-visitor.h"
#include "qapi/qmp/qerror.h"
#include "trace.h"

/* TODO: replace QObject with a simpler visitor to avoid a dependency
 * of the QOM core on QObject?  */
#include "qom/qom-qobject.h"
#include "qapi/qmp/qobject.h"
#include "qapi/qmp/qbool.h"
#include "qapi/qmp/qint.h"
#include "qapi/qmp/qstring.h"
#include "qom/object_helper.h"


#define CHECKED_OBJECT_PROPERTY_SET(type, object, value, property_name, property_description, errp) \
		{ \
		    Error *err = NULL; \
		    assert(object); \
		    object_property_set_##type(object, value, property_name, &err); \
		    if (err) { \
		    	error_prepend(&err, "Setting " #type " property %s (%s) for %s failed: %s.", property_name, \
		    			(property_description)? property_description : "", \
		                object_get_typename(object), error_get_pretty(err)); \
				error_propagate(errp, err); /* Fatal abort */ \
				return; \
		    } \
		}

#define CHECKED_OBJECT_PROPERTY_POINTER_SET(type, object, value, property_name, property_description, errp) \
		{ \
			if (value == NULL) { \
				return; \
			} \
			CHECKED_OBJECT_PROPERTY_SET(type, object, value, property_name, property_description, errp) \
		}

void object_property_set_int_descriptive( Object *obj, int64_t     value, const char *prop_name, const char *prop_desc, Error **errp) { CHECKED_OBJECT_PROPERTY_SET(        int,  obj, value, prop_name, prop_desc, errp); }
void object_property_set_bool_descriptive(Object *obj, bool        value, const char *prop_name, const char *prop_desc, Error **errp) { CHECKED_OBJECT_PROPERTY_SET(        bool, obj, value, prop_name, prop_desc, errp); }
void object_property_set_str_descriptive( Object *obj, const char *value, const char *prop_name, const char *prop_desc, Error **errp) { CHECKED_OBJECT_PROPERTY_POINTER_SET(str,  obj, value, prop_name, prop_desc, errp); }




Object *checked_object_new(Object *parent, const char* node_name,
        const char *type_name, Error **errp)
{
    if (object_class_by_name(type_name) == NULL) {
    	error_setg(errp, "Creating object of type %s failed; no such type.",
                type_name);
    	return NULL;
    }

    Object *obj = object_new(type_name);
    if (!obj) {
    	error_setg(errp, "Creating object of type %s failed.", type_name);
        return NULL;
    }

    object_property_add_child(parent, node_name, obj, errp);
    return obj;
}


void object_property_add_child_descriptive(Object *obj, const char *node_name,
        Object *child, Error **errp)
{
    Error *err = NULL;
    object_property_add_child(obj, node_name, child, &err);
    if (err) {
    	error_prepend(&err, "Adding child %s for %s failed: %s.", node_name,
                object_get_typename(obj), error_get_pretty(err));
    	error_propagate(errp, err);
        return;
    }
}







/* ------------------------------------------------------------------------- */

static void property_get_str_simple(Object *obj, Visitor *v, const char *name,
        void *opaque, Error **errp)
{
    char *value = *(char **) opaque;
    visit_type_str(v, name, &value, errp);
}

static void property_set_str_simple(Object *obj, Visitor *v, const char *name,
        void *opaque, Error **errp)
{
    Error *local_err = NULL;
    char *value;
    visit_type_str(v, name, &value, &local_err);
    if (!local_err) {
        *((char **) opaque) = value;
    }
    error_propagate(errp, local_err);
}

void object_property_add_str_simple(Object *obj, const char *name, char **v, Error **errp)
{
    Error *local_err = NULL;
    object_property_add(obj, name, "string", property_get_str_simple,
            property_set_str_simple,
            NULL, (void *) v, &local_err);
    if (local_err) {
    	error_prepend(&local_err, "Adding property %s for %s failed: %s.", name,
                object_get_typename(obj), error_get_pretty(local_err));
    	error_propagate(errp, local_err);
    }
}

void object_property_add_const_str_simple(Object *obj, const char *name,
        const char **v, Error **errp)
{
    Error *local_err = NULL;
    object_property_add(obj, name, "string", property_get_str_simple,
            property_set_str_simple,
            NULL, (void *) v, &local_err);
    if (local_err) {
    	error_prepend(&local_err, "Adding property %s for %s failed: %s.", name,
                object_get_typename(obj), error_get_pretty(local_err));
    	error_propagate(errp, local_err);
    }
}

static void property_get_bool_simple(Object *obj, Visitor *v, const char *name,
        void *opaque, Error **errp)
{
    bool value = *(bool *) opaque;
    visit_type_bool(v, name, &value, errp);
}

static void property_set_bool_simple(Object *obj, Visitor *v, const char *name,
        void *opaque, Error **errp)
{
    Error *local_err = NULL;
    bool value;

    visit_type_bool(v, name, &value, &local_err);
    if (!local_err) {
        *((bool *) opaque) = value;
    }
    error_propagate(errp, local_err);
}

void object_property_add_bool_simple(Object *obj, const char *name, const bool *v, Error **errp)
{
    Error *local_err = NULL;
    object_property_add(obj, name, "bool", property_get_bool_simple,
            property_set_bool_simple,
            NULL, (void *) v, &local_err);
    if (local_err) {
    	error_prepend(&local_err, "Adding property %s for %s failed: %s.", name,
                object_get_typename(obj), error_get_pretty(local_err));
        error_propagate(errp, local_err);
    }
}
static void property_get_uint64_ptr_simple(Object *obj, Visitor *v,
        const char *name, void *opaque, Error **errp)
{
    uint64_t value = *(uint64_t *) opaque;
    visit_type_uint64(v, name, &value, errp);
}

static void property_set_uint64_ptr_simple(Object *obj, struct Visitor *v,
        const char *name, void *opaque, Error **errp)
{
    Error *local_err = NULL;
    uint64_t value;
    visit_type_uint64(v, name, &value, &local_err);
    if (!local_err) {
        *((uint64_t *) opaque) = value;
    }
    error_propagate(errp, local_err);
}

void object_property_add_uint64_simple(Object *obj, const char *name,
        const uint64_t *v, Error **errp)
{
    Error *local_err = NULL;
    object_property_add(obj, name, "uint64", property_get_uint64_ptr_simple,
            property_set_uint64_ptr_simple, NULL, (void *) v, &local_err);
    if (local_err) {
    	error_prepend(&local_err, "Adding property %s for %s failed: %s.", name,
                object_get_typename(obj), error_get_pretty(local_err));
        error_propagate(errp, local_err);
    }
}

static void property_get_uint32_ptr_simple(Object *obj, Visitor *v,
        const char *name, void *opaque, Error **errp)
{
    uint32_t value = *(uint32_t *) opaque;
    visit_type_uint32(v, name, &value, errp);
}

static void property_set_uint32_ptr_simple(Object *obj, struct Visitor *v,
        const char *name, void *opaque, Error **errp)
{
    Error *local_err = NULL;
    uint32_t value;
    visit_type_uint32(v, name, &value, &local_err);
    if (!local_err) {
        *((uint32_t *) opaque) = value;
    }
    error_propagate(errp, local_err);
}

void object_property_add_uint32_simple(Object *obj, const char *name,
        const uint32_t *v, Error **errp)
{
    Error *local_err = NULL;
    object_property_add(obj, name, "uint32", property_get_uint32_ptr_simple,
            property_set_uint32_ptr_simple, NULL, (void *) v, &local_err);
    if (local_err) {
    	error_prepend(&local_err, "Adding property %s for %s failed: %s.", name,
                object_get_typename(obj), error_get_pretty(local_err));
        error_propagate(errp, local_err);
    }
}

static void property_get_uint16_ptr_simple(Object *obj, Visitor *v,
        const char *name, void *opaque, Error **errp)
{
    uint16_t value = *(uint16_t *) opaque;
    visit_type_uint16(v, name, &value, errp);
}

static void property_set_uint16_ptr_simple(Object *obj, struct Visitor *v,
        const char *name, void *opaque, Error **errp)
{
    Error *local_err = NULL;
    uint16_t value;
    visit_type_uint16(v, name, &value, &local_err);
    if (!local_err) {
        *((uint16_t *) opaque) = value;
    }
    error_propagate(errp, local_err);
}

void object_property_add_uint16_simple(Object *obj, const char *name,
        const uint16_t *v, Error **errp)
{
    Error *local_err = NULL;
    object_property_add(obj, name, "uint16", property_get_uint16_ptr_simple,
            property_set_uint16_ptr_simple, NULL, (void *) v, &local_err);
    if (local_err) {
    	error_prepend(&local_err, "Adding property %s for %s failed: %s.", name,
                object_get_typename(obj), error_get_pretty(local_err));
    	error_propagate(errp, local_err);
    }
}

static void property_get_uint8_ptr_simple(Object *obj, Visitor *v, const char *name,
        void *opaque, Error **errp)
{
    uint8_t value = *(uint8_t *) opaque;
    visit_type_uint8(v, name, &value, errp);
}

static void property_set_uint8_ptr_simple(Object *obj, struct Visitor *v,
        const char *name, void *opaque, Error **errp)
{
    Error *local_err = NULL;
    uint8_t value;
    visit_type_uint8(v, name, &value, &local_err);
    if (!local_err) {
        *((uint8_t *) opaque) = value;
    }
    error_propagate(errp, local_err);
}

void object_property_add_uint8_simple(Object *obj, const char *name,
        const uint8_t *v, Error **errp)
{
    Error *local_err = NULL;
    object_property_add(obj, name, "uint8", property_get_uint8_ptr_simple,
            property_set_uint8_ptr_simple, NULL, (void *) v, &local_err);
    if (local_err) {
    	error_prepend(&local_err, "Adding property %s for %s failed: %s.", name,
                object_get_typename(obj), error_get_pretty(local_err));
    	error_propagate(errp, local_err);
    }
}

static void property_get_int16_ptr_simple(Object *obj, Visitor *v, const char *name,
        void *opaque, Error **errp)
{
    int16_t value = *(int16_t *) opaque;
    visit_type_int16(v, name, &value, errp);
}

static void property_set_int16_ptr_simple(Object *obj, struct Visitor *v,
        const char *name, void *opaque, Error **errp)
{
    Error *local_err = NULL;
    int16_t value;
    visit_type_int16(v, name, &value, &local_err);
    if (!local_err) {
        *((int16_t *) opaque) = value;
    }
    error_propagate(errp, local_err);
}

void object_property_add_int16_simple(Object *obj, const char *name,
        const int16_t *v, Error **errp)
{
    Error *local_err = NULL;
    object_property_add(obj, name, "int16", property_get_int16_ptr_simple,
            property_set_int16_ptr_simple, NULL, (void *) v, &local_err);
    if (local_err) {
    	error_prepend(&local_err, "Adding property %s for %s failed: %s.", name,
                object_get_typename(obj), error_get_pretty(local_err));
    	error_propagate(errp, local_err);
    }
}

/* ------------------------------------------------------------------------- */
