#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>
#include <math.h>
#include <cairo.h>

#define POS_INIT_X     200.0
#define POS_INIT_Y     200.0
#define VELOC_INIT_X     3.0
#define VELOC_INIT_Y     2.0
GtkWidget *frame1;            //frame da simulação


typedef struct
{
  GtkWidget *window;
  GtkWidget *table1, *table2; //tabela que organiza tudo e outra para centrar um butão        
  GtkWidget *vbox2;           //caixa com 
  GtkWidget *frame2;          //frame abaixo (controlos e butões)
  GtkWidget *frame3;          //frame onde estão os controlos

  GtkWidget *movebox;
  GtkWidget *scalebox;        //caixa com as escalas
  GtkWidget *scale[4];        //escalas onde o utilizador mexe
  GtkWidget *adj[4];          //adjustments para as escalas
  
  gboolean move;
  double v[2];
  double pos[2];

  GtkWidget *toggle;          //butão que muda lente côncava para convexa
  gint caso;

}ProgInfo;


static void change_state(GtkWidget* w, ProgInfo *state)                    //Muda a legenda consoante a 
{                                                                          //lente é convergente ou divergente

  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
    {
      g_print("Toggle button activated\n");
      gtk_frame_set_label(GTK_FRAME(frame1),"Lente Concava");
      state->caso=1;
    }
  else
    {
      g_print("Toggle button not activated\n");
      gtk_frame_set_label(GTK_FRAME(frame1),"Lente Convexa");
      state->caso=2;
    }
}


gboolean
desenha_concava (GtkWidget       *widget ,
                 GdkEventExpose  *event  ,
                 ProgInfo *info)
{
  ProgInfo  *pinfo            ;
  cairo_t   *cr               ;
  gint       width, height    ;
  double     pos1, pos2       ;

  pinfo  = info;
 

  gtk_window_get_size (GTK_WINDOW(pinfo->window), &width, &height);
 
  pos1 = gtk_adjustment_get_value (pinfo->adj[0]);
   if (pos1 > pinfo->movebox->allocation.width)
    {
      pos1 = pinfo->movebox->allocation.width;
      gtk_adjustment_set_value (pinfo->adj[0], pos1);
    }
  gtk_adjustment_set_upper (pinfo->adj[0], pinfo->movebox->allocation.width);
  
  /*
  rgb1 = gtk_adjustment_get_value (pinfo->adj[0]);
  rgb2 = gtk_adjustment_get_value (pinfo->adj[1]);
  rgb3 = gtk_adjustment_get_value (pinfo->adj[2]);
  */
 
  cr = gdk_cairo_create (pinfo->window->window);

  cairo_set_source_rgb (cr, 1., 1., 1.);
  cairo_set_line_width (cr, 2);
  cairo_move_to (cr, 15, 3. * pinfo->movebox->allocation.height / 4.);
  cairo_line_to (cr, pinfo->movebox->allocation.width, 3. * pinfo->movebox->allocation.height / 4.);
  cairo_stroke (cr);

  cairo_set_source_rgb (cr, 1., 1., 1.);
  
  cairo_set_line_width (cr, 5);                          //desenha a lente
  
  pos2 = 3. * pinfo->movebox->allocation.height / 4.;

 
  if(pinfo->caso==2)   
    {                                                                         //lente convexa/convergente
      cairo_curve_to(cr, pos1, pos2 + 80, pos1 + 20, pos2, pos1, pos2 -80);
      cairo_curve_to(cr, pos1, pos2 - 80, pos1 - 20, pos2, pos1, pos2 +80);
    }
  else
    {                                                                         //lente concava/divergente

      cairo_curve_to(cr, pos1 - 20, pos2 + 80, pos1-5, pos2, pos1 -20, pos2- 80);
      cairo_line_to (cr, pos1 + 20, pos2 - 80);
      cairo_curve_to(cr, pos1 + 20, pos2 - 80, pos1+5, pos2, pos1 +20, pos2+ 80);
      cairo_line_to (cr, pos1 - 20, pos2 + 80);
    }

  cairo_fill(cr);
  cairo_stroke (cr);

  cairo_destroy (cr);
  pinfo->move = 0;

  return FALSE;
}

gboolean 
change_scale (GtkWidget  *w    ,
              gpointer    data )
{
  ProgInfo  *pinfo ;

  pinfo = (ProgInfo *) data;
  gtk_widget_queue_draw (pinfo->window);

  return TRUE;
}

gboolean
on_configure_event (GtkWidget       *widget ,
                    GdkEventExpose  *event  ,
                    gpointer         data   )
{
  change_scale (widget, data);
  return FALSE;
}


int main( int argc, char *argv[])
{
  ProgInfo *pinfo;

  pinfo  = (ProgInfo *) calloc (1, sizeof (ProgInfo));
  pinfo->pos[0] = POS_INIT_X;
  pinfo->pos[1] = POS_INIT_Y;
  pinfo->v[0] = VELOC_INIT_X;
  pinfo->v[1] = VELOC_INIT_Y;

  gtk_init(&argc, &argv);

  pinfo->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_position(GTK_WINDOW(pinfo->window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(pinfo->window), 600, 400);
  gtk_window_set_title(GTK_WINDOW(pinfo->window), "Lentes delgadas");

  gtk_container_set_border_width(GTK_CONTAINER(pinfo->window), 10);

  //--------------------------------
  //    Tabela Organizadora
  //--------------------------------

  pinfo->table1 = gtk_table_new(3, 3, TRUE);
  gtk_table_set_row_spacings(GTK_TABLE(pinfo->table1), 10);
  gtk_table_set_col_spacings(GTK_TABLE(pinfo->table1), 10);
  gtk_container_add(GTK_CONTAINER(pinfo->window), pinfo->table1);

  pinfo->table2 = gtk_table_new(6, 1, TRUE);
  gtk_table_set_row_spacings(GTK_TABLE(pinfo->table2), 0);
  gtk_table_set_col_spacings(GTK_TABLE(pinfo->table2), 0);

  frame1 = gtk_frame_new("Lente Concava");
  gtk_frame_set_shadow_type(GTK_FRAME(frame1), GTK_SHADOW_IN);
  pinfo->frame2 = gtk_frame_new("Shadow Out");
  gtk_frame_set_shadow_type(GTK_FRAME(pinfo->frame2), GTK_SHADOW_OUT);
  pinfo->frame3 = gtk_frame_new(" ");
  gtk_frame_set_shadow_type(GTK_FRAME(pinfo->frame3), GTK_SHADOW_NONE);
 
  gtk_table_attach_defaults(GTK_TABLE(pinfo->table1), frame1, 0, 3, 0, 2);
  gtk_table_attach_defaults(GTK_TABLE(pinfo->table1), pinfo->frame2, 0, 3, 2, 3);
  gtk_table_attach_defaults(GTK_TABLE(pinfo->table1), pinfo->frame3, 0, 2, 2, 3);
  gtk_table_attach_defaults(GTK_TABLE(pinfo->table1), pinfo->table2, 2, 3, 2, 3);

  pinfo->movebox = gtk_hbox_new (FALSE,0);
  gtk_container_add(GTK_CONTAINER (frame1),pinfo->movebox);

  pinfo->scalebox = gtk_hbox_new (FALSE,0);
  gtk_container_add(GTK_CONTAINER (pinfo->frame2),pinfo->scalebox);

  //------------------------------
  //    Caixa com controlos
  //------------------------------

  gtk_box_set_spacing(GTK_BOX(pinfo->scalebox),20);

  pinfo->adj[0] = (GtkAdjustment *) gtk_adjustment_new (200, 0.0, 400, 0.001, 1.0, 0.0); 
  pinfo->scale[0] = gtk_vscale_new (GTK_ADJUSTMENT (pinfo->adj[0]));
  gtk_range_set_inverted (GTK_RANGE (pinfo->scale[0]), TRUE);
  g_signal_connect (pinfo->adj[0], "value-changed", G_CALLBACK (change_scale), pinfo);
  gtk_scale_set_digits (GTK_SCALE (pinfo->scale[0]), 2);
  gtk_box_pack_start (GTK_BOX (pinfo->scalebox), pinfo->scale[0], FALSE, TRUE, 5);

  pinfo->adj[3] = (GtkAdjustment *) gtk_adjustment_new (.3, 0.0, 1.0, 0.001, 1.0, 0.0); 
  pinfo->scale[3] = gtk_vscale_new (GTK_ADJUSTMENT (pinfo->adj[3]));
  gtk_range_set_inverted (GTK_RANGE (pinfo->scale[3]), TRUE);
  g_signal_connect (pinfo->adj[3], "value-changed", G_CALLBACK (change_scale), pinfo);
  gtk_scale_set_digits (GTK_SCALE (pinfo->scale[3]), 2);
  gtk_box_pack_start (GTK_BOX (pinfo->scalebox), pinfo->scale[3], FALSE, TRUE, 5);

  //------------------------------
  //    Desenha a lente
  //------------------------------
  
  gtk_widget_add_events (pinfo->window, GDK_BUTTON_PRESS_MASK);
  g_signal_connect (pinfo->window, "expose-event", G_CALLBACK (desenha_concava), pinfo);
  g_signal_connect (pinfo->window, "configure-event", G_CALLBACK (on_configure_event), pinfo);
  g_signal_connect (pinfo->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  
  gtk_widget_set_app_paintable (pinfo->window, TRUE);

  //--------------------------------
  //    botão que muda o modo
  //--------------------------------

  pinfo->vbox2 = gtk_vbox_new(0,0);
  pinfo->toggle = gtk_toggle_button_new_with_label("Muda Modo");
  gtk_widget_set_usize(pinfo->toggle,100,24);
  gtk_box_pack_start(GTK_BOX(pinfo->vbox2), pinfo->toggle, 0,0,0);
  g_signal_connect(pinfo->toggle, "toggled", G_CALLBACK(change_state), (gpointer)pinfo);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pinfo->toggle), TRUE);



  gtk_table_attach_defaults(GTK_TABLE(pinfo->table2), pinfo->vbox2, 0, 1, 3, 4);
  
  g_signal_connect_swapped(G_OBJECT(pinfo->window), "destroy",
        G_CALLBACK(gtk_main_quit), G_OBJECT(pinfo->window));

  gtk_widget_show_all(pinfo->window);

  gtk_main();

  return 0;
}
