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

#include "hw/cortexm/cortexm-helper.h"
#include "hw/cortexm/cortexm-mcu.h"

#include "qemu/help_option.h"
#include "qapi/error.h"

#include "hw/boards.h"
#include "qom/object.h"
#include "cpu-qom.h"
#include "qemu/error-report.h"
#include "qapi/visitor.h"

#include "sysemu/sysemu.h"
#if defined(CONFIG_SDL)
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#endif

#if defined(CONFIG_VERBOSE)
#include "verbosity.h"
#endif

/* ------------------------------------------------------------------------- */

/**
 * When verbose, display a line to identify the board (name, description).
 *
 * Does not really depend on Cortex-M, but I could not find a better place.
 */
void cm_board_greeting(MachineState *machine)
{
#if defined(CONFIG_VERBOSE)
    if (verbosity_level >= VERBOSITY_COMMON) {
        MachineClass *mc = MACHINE_GET_CLASS(machine);
        printf("Board: '%s' (%s).\n", mc->name, mc->desc);
    }
#endif
}

/* ------------------------------------------------------------------------- */

#if defined(CONFIG_SDL)
static QEMUTimer *timer;

/**
 * SDL event loop, called every 10 ms by the timer.
 */
static void sdl_event_loop(void *p)
{
    SDL_Event event;

    while (SDL_PollEvent(&event)) {
        //If the user has Xed out the window
        if (event.type == SDL_QUIT) {
            //Quit the program
            fprintf(stderr, "Quit.\n");
            exit(1);
        }
    }

    timer_mod(timer, qemu_clock_get_ms(QEMU_CLOCK_REALTIME) + 10);
}
#endif /* defined(CONFIG_SDL) */

/**
 * Initialise SDL and display the board image.
 */
void *cm_board_init_image(const char *file_name, const char *caption)
{
    void *board_surface = NULL;
#if defined(CONFIG_SDL)
    if (display_type != DT_NOGRAPHIC) {
        // Start SDL, if needed.
        if (SDL_WasInit(SDL_INIT_VIDEO) == 0) {
            SDL_Init(SDL_INIT_VIDEO);
        }

        const char *fullname = qemu_find_file(QEMU_FILE_TYPE_IMAGES, file_name);
        if (fullname == NULL) {
            error_printf("Image file '%s' not found.\n", file_name);
            exit(1);
        }

        int res = IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
        if ((res & (IMG_INIT_JPG | IMG_INIT_PNG))
                != (IMG_INIT_JPG | IMG_INIT_PNG)) {
            error_printf("IMG_init failed (%s).\n", IMG_GetError());
            exit(1);
        }
        /* A better SDL_LoadBMP(). */
        SDL_Surface* board_bitmap = IMG_Load(fullname);
        if (board_bitmap == NULL) {
            error_printf("Cannot load image file '%s' (%s).\n", fullname,
                    IMG_GetError());
            exit(1);
        }

#if defined(CONFIG_VERBOSE)
        if (verbosity_level >= VERBOSITY_DETAILED) {
            printf("Board picture: '%s'.\n", fullname);
        }
#endif

        SDL_WM_SetCaption(caption, NULL);
        SDL_Surface* screen = SDL_SetVideoMode(board_bitmap->w, board_bitmap->h,
                32, SDL_DOUBLEBUF);

        SDL_Surface* board = SDL_DisplayFormat(board_bitmap);
        SDL_FreeSurface(board_bitmap);

        /* Apply image to screen */
        SDL_BlitSurface(board, NULL, screen, NULL);
        /* Update screen */
        SDL_Flip(screen);
        board_surface = screen;

        /* The event loop will be processed from time to time. */
        timer = timer_new_ms(QEMU_CLOCK_REALTIME, sdl_event_loop, &timer);
        timer_mod(timer, qemu_clock_get_ms(QEMU_CLOCK_REALTIME));
    }

#endif
    return board_surface;
}

static void cm_mcu_help_foreach(gpointer data, gpointer user_data)
{
    ObjectClass *oc = data;
    const char *typename;

    typename = object_class_get_name(oc);
    printf("  %s\n", typename);
}

static gint object_class_cmp(gconstpointer a, gconstpointer b)
{
    return strcmp(object_class_get_name(OBJECT_CLASS(a)),
            object_class_get_name(OBJECT_CLASS(b)));
}

bool cm_mcu_help_func(const char *mcu_device)
{

    if ((mcu_device == NULL) || !is_help_option(mcu_device)) {
        return false;
    }

    GSList *list;

    list = object_class_get_list(TYPE_CORTEXM_MCU, false);
    list = g_slist_sort(list, object_class_cmp);

    printf("\nSupported MCUs:\n");
    g_slist_foreach(list, cm_mcu_help_foreach, NULL);
    g_slist_free(list);

    return true;
}

bool cm_board_help_func(const char *name)
{
    if ((name == NULL) || !is_help_option(name)) {
        return false;
    }

    printf("\nSupported boards:\n");

    GSList *el, *machines = object_class_get_list(TYPE_MACHINE, false);

    machines = g_slist_sort(machines, object_class_cmp);
    for (el = machines; el; el = el->next) {
        MachineClass *mc = el->data;
        if (mc->alias) {
            printf("  %-20s %s (alias of %s)\n", mc->alias, mc->desc, mc->name);
        }
        printf("  %-20s %s%s\n", mc->name, mc->desc,
                mc->is_default ? " (default)" : "");
    }

    return true;
}

const char *cm_board_get_name(MachineState *machine)
{
    return object_class_get_name(OBJECT_CLASS(machine));
}

const char *cm_board_get_desc(MachineState *machine)
{
    return MACHINE_GET_CLASS(machine)->desc;
}

/* ------------------------------------------------------------------------- */

/**
 * A version of cpu_generic_init() that does only the object creation and
 * initialisation, without calling realize().
 */
static CPUState *cm_cpu_generic_create(const char *typename,
        const char *cpu_model)
{
    char *str, *name, *featurestr;
    CPUState *cpu;
    ObjectClass *oc;
    CPUClass *cc;
    Error *err = NULL;

    str = g_strdup(cpu_model);
    name = strtok(str, ",");

    oc = cpu_class_by_name(typename, name);
    if (oc == NULL) {
        g_free(str);
        return NULL;
    }

    cpu = CPU(object_new(object_class_get_name(oc)));
    cc = CPU_GET_CLASS(cpu);

    featurestr = strtok(NULL, ",");
    cc->parse_features(cpu, featurestr, &err);
    g_free(str);
    if (err != NULL) {
        error_report_err(err);
        object_unref(OBJECT(cpu));
        return NULL;
    }

    return cpu;
}

/**
 * A version of cpu_arm_init() that does only the object creation and
 * initialisation, without calling realize().
 */
void *cm_cpu_arm_create(Object *parent, const char *cpu_model)
{
    ARMCPU *cpu;
    cpu = (ARMCPU*) ARM_CPU(cm_cpu_generic_create(TYPE_ARM_CPU, cpu_model));
    if (!cpu) {
        error_report("Unable to find CPU definition %s", cpu_model);
        exit(1);
    }
    cm_object_property_add_child(parent, "core", OBJECT(cpu));

    return cpu;
}

/* ------------------------------------------------------------------------- */


/* ------------------------------------------------------------------------- */

Object *cm_object_new_mcu(MachineState *machine, const char *board_device_name)
{
    const char *device_name = board_device_name;
    if (machine && machine->mcu_device) {
        device_name = machine->mcu_device;
    }

    return cm_object_new(cm_object_get_machine(), "mcu", device_name);
}

#warning why not void object_realize(Object *obj)???
/**
 *  Call the parent realize() of a given type.
 */
bool cm_device_parent_realize(DeviceState *dev, Error **errp,
        const char *type_name)
{
    /* Identify parent class. */
    DeviceClass *klass = DEVICE_CLASS(
            object_class_get_parent(object_class_by_name(type_name)));

    if (klass->realize) {
        Error *local_err = NULL;
        klass->realize(dev, &local_err);
        if (local_err) {
            error_propagate(errp, local_err);
            return false;
        }
    }
    return true;
}

#warning why not void object_realize(Object *obj)???
/**
 *  Call the realize() of a given type.
 */
bool cm_device_by_name_realize(DeviceState *dev, Error **errp,
        const char *type_name)
{
    /* Identify class. */
    DeviceClass *klass = DEVICE_CLASS(object_class_by_name(type_name));

    if (klass->realize) {
        Error *local_err = NULL;
        klass->realize(dev, &local_err);
        if (local_err) {
            error_propagate(errp, local_err);
            return false;
        }
    }
    return true;
}



Object *cm_container_get_peripheral(void)
{
    return container_get(object_get_root(), "/peripheral");
}

/* ------------------------------------------------------------------------- */

