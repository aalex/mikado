/**
 * Prototype of an oriented graph in Clutter.
 */
#include <clutter/clutter.h>
#include <glib.h>
#include "graph_layout.h"

static const gint WIN_W = 640;
static const gint WIN_H = 480;

typedef struct stuff_
{
    ClutterActor *group;
    struct GraphLayout *layout;
} Stuff;

static void key_event_cb(ClutterActor *actor, ClutterKeyEvent *event, gpointer data)
{
    Stuff *self = (Stuff *) data;
    (void) self; // unused
    switch (event->keyval)
    {
        case CLUTTER_Escape:
            clutter_main_quit();
            break;
        case CLUTTER_space:
            break;
        default:
            break;
    }
}

void stuff_init(Stuff *self)
{
    self->layout = graph_layout_new();
    //graph_layout_nodesep_set(self->layout, 2.0);
    //graph_layout_ranksep_set(self->layout, 2.0);
    int first = graph_layout_node_new(self->layout);
    int second = graph_layout_node_new(self->layout);
    graph_layout_node_outpads_set(self->layout, first, 1);
    graph_layout_node_inpads_set(self->layout, second, 1);
    graph_layout_connection_set(self->layout, first, 0, second, 0);
    graph_layout_relayout(self->layout);
}

void stuff_cleanup(Stuff *self)
{
    graph_layout_free(self->layout);
}

int main(int argc, char *argv[])
{
    clutter_init(&argc, &argv);

    ClutterActor *stage = NULL;
    ClutterColor black = { 0x00, 0x00, 0x00, 0xff };
    stage = clutter_stage_get_default();
    clutter_stage_set_title(CLUTTER_STAGE(stage), "This is just a prototype");
    clutter_stage_set_color(CLUTTER_STAGE(stage), &black);
    clutter_actor_set_size(stage, WIN_W, WIN_H);

    ClutterActor *rect = clutter_rectangle_new();
    clutter_container_add_actor(CLUTTER_CONTAINER(stage), rect);
    clutter_actor_set_size(rect, 5.0, 5.0);

    Stuff *self = g_new0(Stuff, 1);
    self->group = clutter_group_new();
    clutter_container_add_actor(CLUTTER_CONTAINER(stage), self->group);
    
    g_signal_connect(stage, "key-press-event", G_CALLBACK(key_event_cb), self);
    clutter_actor_show(stage);

    clutter_main();

    stuff_cleanup(self);
    g_free(self);
    return 0;
}

