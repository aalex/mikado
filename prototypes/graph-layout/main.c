#include <glib.h>
#include "graph_layout.h"

int main(int argc, char *argv[])
{
    struct GraphLayout *gl = graph_layout_new();
    //graph_layout_nodesep_set(gl, 2.0);
    //graph_layout_ranksep_set(gl, 2.0);
    int first = graph_layout_node_new(gl);
    int second = graph_layout_node_new(gl);
    graph_layout_node_outpads_set(gl, first, 1);
    graph_layout_node_inpads_set(gl, second, 1);
    graph_layout_connection_set(gl, first, 0, second, 0);
    graph_layout_relayout(gl);
    graph_layout_free(gl);
    return 0;
}
