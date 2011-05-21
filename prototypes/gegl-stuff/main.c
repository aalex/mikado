#include <gegl.h>

void list_ops()
{
    gchar **operations;
    guint   n_operations;
    gint i;
    operations = gegl_list_operations (&n_operations);
    g_print ("Available operations:\n");
    for (i=0; i < n_operations; i++)
    {
        g_print ("\t%s\n", operations[i]);
    }
    g_free (operations);
}

int main(int argc, char **argv)
{
    gegl_init (&argc, &argv);

    list_ops();

    gegl_exit ();
    return 0;
}

