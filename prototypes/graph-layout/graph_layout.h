/* Automatic layouting of directed acyclc graphs for oxide
 *
 *    oxide is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    oxide is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with oxide; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright 2003, 2004 Øyvind Kolås <pippin@freedesktop.org>
 *
 */

#ifndef GRAPH_LAYOUT_H
#define GRAPH_LAYOUT_H

/* Instantiate a new graph layout engine
 */
struct GraphLayout *
graph_layout_new             (void);

/* Remove all node data within graph
 */
void
graph_layout_clear           (struct GraphLayout *gl);

/* Dispose of a graph layout engine
 */
void
graph_layout_free            (struct GraphLayout *gl);

/* Set the minimum nodeseperation used in the layout
 */
void
graph_layout_nodesep_set     (struct GraphLayout *gl,
                              double              nodesep);

/* Set the nodeseperation used in the layout
 */
void
graph_layout_ranksep_set     (struct GraphLayout *gl,
                              double              ranksep);

/* Get the minimum nodeseperation used in the layout
 */
double
graph_layout_nodesep_get     (struct GraphLayout *gl);

/* Get the nodeseperation used in the layout
 */
double
graph_layout_ranksep_get     (struct GraphLayout *gl);

/* Add a new node to the layout, an integer handle used for
 * referencing the node within the instance is returned.
 */
int
graph_layout_node_new        (struct GraphLayout *gl);

/* Remove a node from the layout, this will also remove connections
 * that either are going into, or going out from the node
 */
void
graph_layout_node_free       (struct GraphLayout *gl, int node);

void
graph_layout_node_width_set  (struct GraphLayout *gl,
                              int                 node,
                              double              width);

void
graph_layout_node_height_set (struct GraphLayout *gl,
                              int                 node,
                              double              height);

double
graph_layout_node_width_get  (struct GraphLayout *gl,
                              int                 node_no);

double
graph_layout_node_height_get (struct GraphLayout *gl,
                              int                 node);

/* get the position of the center of a node using it's handle
 */
void
graph_layout_node_getpos     (struct GraphLayout *gl,
                              int                 node,
                              double             *x,
                              double             *y);

/* get the position of the center of a node by it's internal
 * number,. (this number is unstable between layouts, but is
 * useful when drawing to pull out graph data)
 */
void
graph_layout_node_no_getpos   (struct GraphLayout *gl,
                               int                 node_no,
                               double             *x,
                               double             *y);

/* get number of nodes in graph
 */
int
graph_layout_node_count       (struct GraphLayout *gl);

/* Assign an connection, one pad can only be bound to one other node
 * assigning a source_node of 0, will remove existing connection
 */
void
graph_layout_connection_set   (struct GraphLayout *gl,
                               int                 source_node,
                               int                 source_pad,
                               int                 dest_node,
                               int                 dest_pad);

/* get number of connections in graph
 */
int
graph_layout_connection_count (struct GraphLayout *gl);

/* query if a node is at given position
 * return: node handle
 *         0 no node present
 *        -1 error
 */
int
graph_layout_node_resolve     (struct GraphLayout *gl,
                               double              x,
                               double              y);

/* query which node handles are involved in an connection
 * within the graph layout
 */
void
graph_layout_connection_get_nodes (struct GraphLayout *gl,
                                   int                 connection_no,
                                   int                *from_node,
                                   int                *to_node);

/* query the coordinates of connection end-points, is returned as the
 * center of the nodes involved in the connection.
 */
void
graph_layout_connection_get_coords (struct GraphLayout *gl,
                                    int                 connection_no,
                                    double             *x0,
                                    double             *y0,
                                    double             *x1,
                                    double             *y1);

void
graph_layout_relayout         (struct GraphLayout *gl);

int
graph_layout_node_no2id       (struct GraphLayout *gl,
                               int                 no);

void
graph_layout_node_inpads_set  (struct GraphLayout *gl,
                               int                 node,
                               int                 inpads);

int
graph_layout_node_inpads_get  (struct GraphLayout *gl,
                               int node);

void
graph_layout_node_outpads_set (struct GraphLayout *gl,
                               int                 node,
                               int                 outpads);

int
graph_layout_node_outpads_get (struct GraphLayout *gl,
                               int                 node);

#endif /* GRAPH_LAYOUT_H */
