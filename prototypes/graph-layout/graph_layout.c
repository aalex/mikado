/* Automatic layouting of directed acyclic graphs
 *
 *    aluminum is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    aluminium is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with oxide; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * 2003, 2004 © Øyvind Kolås <pippin@freedesktop.org>
 *
 */

#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "graph_layout.h"

static gint id2no (struct GraphLayout *gl,
                   glong               id);

static gint no2id (struct GraphLayout *gl,
                   gint no);

static void remove_connection (struct GraphLayout *gl,
                               gint                connection_no);

static gint
lines_intersect (gdouble x0, gdouble y0,
		 gdouble x1, gdouble y1,
		 gdouble x2, gdouble y2,
                 gdouble x3, gdouble y3);

static void toposort (struct GraphLayout *gl);

static void center_graph (struct GraphLayout *gl);

typedef struct rank_line
{
  gint nodes;
  struct GraphLayoutNode **node;
} rank_line;

typedef struct GraphLayoutNode
{
  gint id;
  gdouble x;
  gdouble y;
  gdouble width;
  gdouble height;
  gint inpads;
  gint outpads;

  gint rank;
  gint done;
} GraphLayoutNode;

typedef struct GraphLayoutConnection
{
  gint from_node_id;
  gint from_pad;
  gint to_node_id;
  gint to_pad;

  gint from_node_no;
  gint to_node_no;
}
GraphLayoutConnection;

typedef struct GraphLayout
{
  GraphLayoutNode **node;
  glong alloc_nodes;
  glong nodes;

  GraphLayoutConnection **connection;
  glong alloc_connections;
  glong connections;

  glong max_nodeid;

  /* get/settables */

  gdouble nodesep;
  gdouble ranksep;
}
GraphLayout;

static inline gint
node_rank (GraphLayout * gl, gint node_no, gint startrank)
{
  gint i;
  gint maxrank = startrank;
  for (i = 0; gl->connection[i]; i++)
    {
      GraphLayoutConnection *connection = gl->connection[i];
      if (connection->to_node_no == node_no)
	{
	  gint rank = node_rank (gl, connection->from_node_no, startrank + 1);
	  if (rank > maxrank)
	    maxrank = rank;
	}
    }
  return maxrank;
}

static void
init_data (struct GraphLayout *gl)
{
  gint node_no;

  for (node_no = 0; gl->node[node_no]; node_no++)
    {
      GraphLayoutNode *node = gl->node[node_no];
      node->x = -1;
      node->y = -1;
      node->done = 0;
    }
}


static gdouble
hor_coord (struct GraphLayout *gl, gdouble ver_coord)
{
  gint node_no;
  gdouble max = 0;

  for (node_no = 0; node_no < gl->nodes; node_no++)
    {
      if (gl->node[node_no]->y == ver_coord && gl->node[node_no]->x > max)
	{
	  max = gl->node[node_no]->x;
	}
    }
  return max + gl->nodesep;
}


static void
place_providers (struct GraphLayout *gl, gint node_no)
{
  gint inpad;

  for (inpad = 0; inpad < gl->node[node_no]->inpads; inpad++)
    {
      int conn_no;
      for (conn_no = 0; conn_no < gl->connections; conn_no++)
	{
	  GraphLayoutConnection *connection = gl->connection[conn_no];
	  if (connection->to_node_no == node_no &&
	      connection->to_pad == inpad)
	    {

	      if (!gl->node[connection->from_node_no]->done)
		{
		  gl->node[connection->from_node_no]->y =
		    gl->node[node_no]->y - gl->ranksep;
		  gl->node[connection->from_node_no]->x =
		    hor_coord (gl, gl->node[connection->from_node_no]->y);
		  if (gl->node[connection->from_node_no]->x <
		      gl->node[node_no]->x)
		    {
		      gl->node[connection->from_node_no]->x =
			gl->node[node_no]->x;
		    }


		  gl->node[connection->from_node_no]->done = 1;
		  place_providers (gl, connection->from_node_no);
		}
	    }
	}
    }
}

static gint
is_sink (struct GraphLayout *gl, gint node_no)
{
  gint conn_no;
  GraphLayoutNode *node = gl->node[node_no];

  if (!node->outpads)
    return 1;
  for (conn_no = 0; conn_no < gl->connections; conn_no++)
    {
      GraphLayoutConnection *connection = gl->connection[conn_no];
      if (connection->from_node_no == node_no)
	return 0;
    }
  return 1;
}

static void
initial_order (struct GraphLayout *gl)
{
  gint node_no;

  init_data (gl);
  for (node_no = 0; gl->node[node_no]; node_no++)
    {
      GraphLayoutNode *node = gl->node[node_no];

      if (is_sink (gl, node_no))
	{
	  node->done = 1;
	  node->y = 0;
	  node->x = hor_coord (gl, 0);
	  place_providers (gl, node_no);
	}
    }
}

void
graph_layout_relayout (struct GraphLayout *gl)
{
  toposort (gl);
  initial_order (gl);
  center_graph (gl);
  return;
}

/* Instantiate a new graph layout engine
 */
struct GraphLayout *
graph_layout_new (void)
{
  GraphLayout *gl = calloc (1, sizeof (GraphLayout));

  gl->nodes = 0;
  gl->connections = 0;
  gl->max_nodeid = 0;
  gl->alloc_nodes = 8;
  gl->node = malloc (sizeof (struct GraphLayoutNode *) * gl->alloc_nodes);
  gl->node[0] = NULL;
  gl->alloc_connections = 8;
  gl->connection =
    malloc (sizeof (struct GraphLayoutConnection *) * gl->alloc_nodes);
  gl->connection[0] = NULL;

  return gl;
}

/* Remove all node data within graph
 */
void
graph_layout_clear (struct GraphLayout *gl)
{
  while (graph_layout_node_count (gl))
    graph_layout_node_free (gl, no2id (gl, 0));
  gl->max_nodeid = 0;
  gl->nodes = 0;

  while (graph_layout_connection_count (gl))
    remove_connection (gl, 0);
  gl->connections = 0;
}

/* Dispose of a graph layout engine
 */
void
graph_layout_free (struct GraphLayout *gl)
{
  graph_layout_clear (gl);
  free (gl->node);
  free (gl->connection);
  free (gl);
}

/* Set the minimum nodeseperation used in the layout
 */
void
graph_layout_nodesep_set (struct GraphLayout *gl, gdouble nodesep)
{
  gl->nodesep = nodesep;
}

/* Set the nodeseperation used in the layout
 */
void
graph_layout_ranksep_set (struct GraphLayout *gl, gdouble ranksep)
{
  gl->ranksep = ranksep;
}

/* Get the minimum nodeseperation used in the layout
 */
gdouble
graph_layout_nodesep_get (struct GraphLayout *gl)
{
  return gl->nodesep;
}

/* Get the nodeseperation used in the layout
 */
gdouble
graph_layout_ranksep_get (struct GraphLayout * gl)
{
  return gl->ranksep;
}

/* Add a new node to the layout, an integer handle used for
 * referencing the node within the instance is returned.
 */
gint
graph_layout_node_new (struct GraphLayout * gl)
{
  if (gl->alloc_nodes < gl->nodes + 2)
    {				/* need to allocate new space,
				   the sentinel padding node takes space as well */
      GraphLayoutNode **newlist =
	malloc (sizeof (struct GraphLayoutNode *) * gl->alloc_nodes * 2);
      if (!newlist)
	{
	  fprintf (stderr, "graph_layout_new() mem error\n");
	  return -1;
	}
      memcpy (newlist, gl->node,
	      sizeof (struct GraphLayoutNode *) * gl->alloc_nodes);
      gl->alloc_nodes *= 2;
      free (gl->node);
      gl->node = newlist;
    }
  {				/* do real insertion */
    GraphLayoutNode *node = calloc (1, sizeof (GraphLayoutNode));
    if (!node)
      return -1;

    node->id = ++gl->max_nodeid;
    node->x = 0.0;
    node->y = 0.0;
    node->width = 32;
    node->height = 32;
    node->inpads = 0;
    node->outpads = 0;

    gl->node[gl->nodes++] = node;
    gl->node[gl->nodes] = NULL;
    return node->id;
  }
  return -1;
}

/* Remove a node from the layout, this will also remove connections
 * that either are going into, or going out from the node
 */
void
graph_layout_node_free (struct GraphLayout *gl, gint node)
{
  gint node_no = id2no (gl, node);
  gint connection_no = 0;

  /* remove connections referencing this node */
  for (connection_no = 0; connection_no < gl->connections; connection_no++)
    {
      GraphLayoutConnection *connection = gl->connection[connection_no];
      if (connection->from_node_id == node || connection->to_node_id == node)
	{
	  remove_connection (gl, connection_no);
	}
    }

  free (gl->node[node_no]);
  gl->node[node_no] = gl->node[gl->nodes - 1];
  gl->node[gl->nodes - 1] = NULL;
  gl->nodes--;

  /* FIXME: add code to reallocate the storage down as well? */
}

/* Assign an connection, one pad can only be bound to one other node
 * (assigning a source_node of 0, will remove existing connections, dynamic graphs,. for
 *  relayouting is not a behavior to be trusted)
 */
void
graph_layout_connection_set (struct GraphLayout *gl,
			     gint source_node,
			     gint source_pad, gint dest_node, gint dest_pad)
{
  gint connection_no = 0;

  while (gl->connection[connection_no])
    {
      GraphLayoutConnection *connection = gl->connection[connection_no];
      if (connection->to_node_id == dest_node
	  && connection->to_pad == dest_pad)
	{
	  if (source_node == 0)
	    {			/* remove connection */
	      remove_connection (gl, connection_no);
	      return;
	    }
	  else
	    {
	      connection->from_node_id = source_node;
	      connection->from_pad = source_pad;
	      return;
	    }
	}
      connection_no++;
    }

  if (source_node == 0)		/* trying to remove non existant connection */
    return;

  if (gl->alloc_connections < gl->connections + 2)
    {				/* need to allocate new space,
				   the sentinel padding connection takes space as well */
      GraphLayoutConnection **newlist =
	malloc (sizeof (struct GraphLayoutConnection *) *
		gl->alloc_connections * 2);
      if (!newlist)
	{
	  fprintf (stderr, "graph_layout connection mem error\n");
	  return;
	}
      memcpy (newlist, gl->connection,
	      sizeof (struct GraphLayoutConnection *) *
	      gl->alloc_connections);
      gl->alloc_connections *= 2;
      free (gl->connection);
      gl->connection = newlist;
    }
  {				/* do real insertion */
    GraphLayoutConnection *connection =
      calloc (1, sizeof (GraphLayoutConnection));
    if (!connection)
      fprintf (stderr, "err!\n");

    connection->from_node_id = source_node;
    connection->from_pad = source_pad;
    connection->to_node_id = dest_node;
    connection->to_pad = dest_pad;

    gl->connection[gl->connections++] = connection;
    gl->connection[gl->connections] = NULL;
  }
}

/* get the position of the center of a node using it's handle
 */
void
graph_layout_node_getpos (struct GraphLayout *gl,
			  gint node, gdouble * x, gdouble * y)
{
  gint node_no = id2no (gl, node);
  graph_layout_node_no_getpos (gl, node_no, x, y);
}

/* get the position of the center of a node by it's internal
 * number,. (this number is unstable between layouts, but is
 * useful when drawing to pull out graph data)
 */
void
graph_layout_node_no_getpos (struct GraphLayout *gl,
			     gint node_no, gdouble * x, gdouble * y)
{
  if (node_no == -1)
    {
      return;
    }
  if (x)
    *x = gl->node[node_no]->x;
  if (y)
    *y = gl->node[node_no]->y;
}

/* query if a node is at given position
 * return: node handle
 *        -1 not a node
 */
gint
graph_layout_node_resolve (struct GraphLayout *gl, gdouble x, gdouble y)
{
  gint no = 0;
  while (gl->node[no])
    {
      GraphLayoutNode *node = gl->node[no];
      if (x >= node->x - node->width / 2 &&
	  y >= node->y - node->height / 2 &&
	  x <= node->x + node->width / 2 && y <= node->y + node->height / 2)
	return node->id;
      no++;
    }
  return -1;
}

/* query which node handles are involved in an connection
 * within the graph layout
 */
void
graph_layout_connection_get_nodes (struct GraphLayout *gl,
				   gint connection_no,
				   gint * from_node, gint * to_node)
{
  if (connection_no > gl->connections)
    return;
  if (from_node)
    *from_node = gl->connection[connection_no]->from_node_id;
  if (to_node)
    *to_node = gl->connection[connection_no]->to_node_id;
}

/* query which node handles are involved in an connection
 * within the graph layout
 */
void
graph_layout_connection_get_nodes_pads (struct GraphLayout *gl,
					gint connection_no,
					gint * from_node,
					gint * from_pad,
					gint * to_node, gint * to_pad)
{
  if (connection_no > gl->connections)
    return;
  if (from_node)
    *from_node = gl->connection[connection_no]->from_node_id;
  if (to_node)
    *to_node = gl->connection[connection_no]->to_node_id;
  if (from_pad)
    *from_pad = gl->connection[connection_no]->from_pad;
  if (to_pad)
    *to_pad = gl->connection[connection_no]->to_pad;
}

/* query the coordinates of connection end-points
 */
void
graph_layout_connection_get_coords (struct GraphLayout *gl,
				    gint connection_no,
				    gdouble * x0,
				    gdouble * y0, gdouble * x1, gdouble * y1)
{
  gint from_node;
  gint to_node;
  gint from_pad;
  gint to_pad;

  if (connection_no > gl->connections)
    return;

  graph_layout_connection_get_nodes_pads (gl, connection_no, &from_node,
					  &from_pad, &to_node, &to_pad);

  graph_layout_node_getpos (gl, from_node, x0, y0);
  graph_layout_node_getpos (gl, to_node, x1, y1);

  *y0 += graph_layout_node_height_get (gl, from_node) / 2;
  *y1 -= graph_layout_node_height_get (gl, to_node) / 2;

  {
    gdouble from_width = graph_layout_node_width_get (gl, from_node);
    gint from_pads = graph_layout_node_outpads_get (gl, from_node);

    gdouble to_width = graph_layout_node_width_get (gl, to_node);
    gint to_pads = graph_layout_node_inpads_get (gl, to_node);

    *x0 +=
      -from_width * 0.9 / 2 + (from_width * 0.9 / from_pads) * (from_pad +
								0.5);
    *x1 += -to_width * 0.9 / 2 + (to_width * 0.9 / to_pads) * (to_pad + 0.5);

  }

}

/* get number of nodes in graph
 */
gint
graph_layout_node_count (struct GraphLayout * gl)
{
  return gl->nodes;
}

/* get number of connections in graph
 */
gint
graph_layout_connection_count (struct GraphLayout * gl)
{
  return gl->connections;
}

void
graph_layout_node_width_set (struct GraphLayout *gl, gint node, gdouble width)
{
  gl->node[id2no (gl, node)]->width = width;
}

void
graph_layout_node_height_set (struct GraphLayout *gl,
			      gint node, gdouble height)
{
  gl->node[id2no (gl, node)]->height = height;
}

gdouble
graph_layout_node_width_get (struct GraphLayout *gl, gint node)
{
  assert (gl);
  assert (node >= 0);
  return gl->node[id2no (gl, node)]->width;
}

gdouble
graph_layout_node_height_get (struct GraphLayout * gl, gint node)
{
  assert (gl);
  assert (node >= 0);
  return gl->node[id2no (gl, node)]->height;
}

void
graph_layout_node_inpads_set (struct GraphLayout *gl, gint node, gint inpads)
{
  gl->node[id2no (gl, node)]->inpads = inpads;
}

gint
graph_layout_node_inpads_get (struct GraphLayout *gl, gint node)
{
  return gl->node[id2no (gl, node)]->inpads;
}

void
graph_layout_node_outpads_set (struct GraphLayout *gl,
			       gint node, gint outpads)
{
  gl->node[id2no (gl, node)]->outpads = outpads;
}

gint
graph_layout_node_outpads_get (struct GraphLayout *gl, gint node)
{
  return gl->node[id2no (gl, node)]->outpads;
}

static gint
id2no (struct GraphLayout *gl, glong id)
{
  gint no = 0;
  while (gl->node[no])
    {
      if (gl->node[no]->id == id)
	return no;
      no++;
    }
  return -1;
}

static gint
no2id (struct GraphLayout *gl, gint no)
{
  return gl->node[no]->id;
}

gint
graph_layout_node_no2id (struct GraphLayout * gl, gint no)
{
  return gl->node[no]->id;
}

static void
remove_connection (struct GraphLayout *gl, gint connection_no)
{
  free (gl->connection[connection_no]);
  gl->connection[connection_no] = gl->connection[gl->connections - 1];
  gl->connection[gl->connections - 1] = NULL;
  gl->connections--;
}


static inline gdouble
determinant (gdouble x0, gdouble y0, gdouble x1, gdouble y1)
{
  return x0 * y1 - y0 * x1;
}

static inline gint
lines_intersect (gdouble x0, gdouble y0,
		 gdouble x1, gdouble y1,
		 gdouble x2, gdouble y2, gdouble x3, gdouble y3)
{
  gdouble dxl0 = x1 - x0;
  gdouble dyl0 = y1 - y0;

  gdouble det_a = determinant (x2 - x0, y2 - y0, dxl0, dyl0);
  gdouble det_b = determinant (x3 - x0, y3 - y0, dxl0, dyl0);

  if ((det_a < 0 && det_b > 0) || (det_a > 0 && det_b < 0))
    return 1;

  if ((det_a < 0.001 && det_a > -0.001) && (det_b < 0.001 && det_b > -0.001))
    return 1;

  if (((x3 - x2) < 0.001 && (x3 - x2) > -0.001)
      && ((y3 - y2) < 0.001 && (y3 - y2) > -0.001))
    return 1;

  if ((det_b < 0.001 && det_b > -0.001) && (dxl0 < 0.001 && dxl0 > -0.001))
    return 1;

  return 0;
}

/********* Topological sort of graph ************/

static void
add_connection_no (struct GraphLayout *gl)
{
  gint i;

  for (i = 0; i < gl->connections; i++)
    {
      gl->connection[i]->from_node_no =
	id2no (gl, gl->connection[i]->from_node_id);
      gl->connection[i]->to_node_no =
	id2no (gl, gl->connection[i]->to_node_id);
    }
}


static gint
inputs_done (struct GraphLayout *gl,
	     GraphLayoutNode * node, gint * connection_done)
{
  gint i;

  for (i = 0; i < gl->connections; i++)
    if (gl->connection[i]->to_node_id == node->id)
      if (!connection_done[i])
	return 0;
  return 1;
}

static void
mark_outputsdone (struct GraphLayout *gl,
		  GraphLayoutNode * node, int *connection_done)
{
  gint i;

  for (i = 0; i < gl->connections; i++)
    if (gl->connection[i]->from_node_id == node->id)
      connection_done[i] = 1;
}

static void
toposort (struct GraphLayout *gl)
{
  gint *connection_done = NULL;
  gint nodes_done = 0;
  gint prev_nodes_done = 0;

  if (!gl->nodes)
    return;

  connection_done = calloc (1, sizeof (int) * gl->connections);

  while (nodes_done < gl->nodes)
    {

      int node_no;
    eek:

      node_no = nodes_done;

      while (node_no < gl->nodes)
	{
	  prev_nodes_done = nodes_done;

	  if (inputs_done (gl, gl->node[node_no], connection_done))
	    {
	      GraphLayoutNode *temp;

	      mark_outputsdone (gl, gl->node[node_no], connection_done);
	      temp = gl->node[node_no];
	      gl->node[node_no] = gl->node[nodes_done];
	      gl->node[nodes_done] = temp;
	      nodes_done++;
	      goto eek;
	    }
	  node_no++;
	}
      if (nodes_done == prev_nodes_done)
	{
	  free (connection_done);
	  return;
	}
    }

  free (connection_done);

  add_connection_no (gl);
}

static void
center_graph (struct GraphLayout *gl)
{
  gint node_no;
#define BIG_DOUBLE 300000.0
  gdouble left = BIG_DOUBLE;
  gdouble top = BIG_DOUBLE;
  gdouble right = -BIG_DOUBLE;
  gdouble bottom = -BIG_DOUBLE;

  gdouble shift_x, shift_y;	/* amount of shifting to center layout */

  for (node_no = 0; gl->node[node_no]; node_no++)
    {
      GraphLayoutNode *node = gl->node[node_no];

      if (node->x - node->width / 2 < left)
	left = node->x - node->width / 2;
      if (node->y - node->height / 2 < top)
	top = node->y - node->height / 2;

      if (node->x + node->width / 2 > right)
	right = node->x + node->width / 2;
      if (node->y + node->height / 2 > bottom)
	bottom = node->y + node->height / 2;
    }

  shift_x = (left + right) / 2;
  shift_y = (top + bottom) / 2;

  for (node_no = 0; gl->node[node_no]; node_no++)
    {
      GraphLayoutNode *node = gl->node[node_no];

      node->x -= shift_x;
      node->y -= shift_y;
    }
};
