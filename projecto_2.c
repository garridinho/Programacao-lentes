#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>
#include <math.h>
#include <cairo.h>

//note-se que o raio não é desenhado de forma precisa as coisas apenas funcionam proporcionalmente
#define N_AR               1
#define VELOC_INIT_X     3.0
#define VELOC_INIT_Y     2.0
GtkWidget *frame1;            //frame da simulação

typedef struct
{
  GdkColor window       ;
  GdkColor scale[4]     ;
  GdkColor zero         ;

  GdkColor frame_lente   ;
  GdkColor frame_comandos;

  GdkColor label_lente   ;
  GdkColor label_comandos;

  GdkColor bt_mudamodo  ;

  GdkColor black    ;
} ProgCores ;


typedef struct
{
  GtkWidget *window           ;
  GtkWidget *table1           ;           //tabela que organiza tudo
  GtkWidget *table2           ;           //tabela para centrar um butão
  GtkWidget *table3           ;           //tabela com os controlos
  GtkWidget *hbox             ;           //caixa com 
  GtkWidget *hbox2            ;
  GtkWidget *frame2           ;           //frame abaixo (controlos e butões)
  GtkWidget *frame3           ;           //frame onde estão os controlos
  GtkWidget *frame_comandos[5]; 
  ProgCores cor    ;

  GtkWidget *movebox     ;
  GtkWidget *scalebox    ;          //caixa com as escalas
  GtkWidget *scale[5]    ;          //escalas onde o utilizador mexe
  GtkWidget *adj[5]      ;          //adjustments para as escalas
  GtkWidget *scale_box[5];          //caixas para as escalas
  
  gboolean move  ;
  //  double   v[2]  ;
  //  double   pos[2];
  double   df    ;

  GtkWidget *toggle;          //butão que muda lente côncava para convexa
  gint caso        ;

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
  gint       w, h             ;           //width and height of the window
  double     df               ;           //distância focal
  double     i                ;           //indice de refracção
  double     r                ;           //raio de curvatura
  double     corr_r           ;           //vareavel que está relacionada com o valor do raio
  double     pos1, pos2       ;           //posições do objecto
  double     size_obj         ;           //tamanho do objecto
  double     t                ;           //varia o tamanho da lente e das coisa em geral
                                          //de acordo coom o tamanho da janela
  pinfo  = info;
  

  gtk_window_get_size (GTK_WINDOW(pinfo->window), &w, &h);
  t = h/20;                     //tipo as unidades para desenha tudo direitinho

  i        =  gtk_adjustment_get_value (pinfo->adj[4]);
  r        =  gtk_adjustment_get_value (pinfo->adj[3]);
  pos1     = (gtk_adjustment_get_value (pinfo->adj[2])*.45)+30;
  pos2     =  h/3.;
  size_obj =  gtk_adjustment_get_value (pinfo->adj[1]);

  //calcula a distância focal através da "Lensmaker's equation"
  corr_r=r*.1;
  i=i/N_AR;
  df     = (4*t*corr_r)/(i-1);
  df=.5*df;


  if (pos1 > pinfo->movebox->allocation.width)
    {
      pos1 = pinfo->movebox->allocation.width;
      gtk_adjustment_set_value (pinfo->adj[0], pos1);
    }
  gtk_adjustment_set_upper (pinfo->adj[0], pinfo->movebox->allocation.width);
    
    
  cr = gdk_cairo_create (pinfo->window->window);
  
  /*Linha horizontal Principal*/
  cairo_set_source_rgb (cr, 1., 1., 1.);           
  cairo_set_line_width (cr, 2);
  cairo_move_to (cr, 15, pos2);
  cairo_line_to (cr, w-15,pos2);
  cairo_stroke (cr);

  /*focos*/
  cairo_set_source_rgb (cr, 1., 1., 1.);                              
  cairo_set_line_width (cr, 2);
  cairo_move_to(cr,w/2-df,pos2+t*.5);
  cairo_line_to(cr,w/2-df,pos2-t*.5);
  cairo_stroke(cr);
  cairo_move_to(cr,w/2+df,pos2+t*.5);
  cairo_line_to(cr,w/2+df,pos2-t*.5);
  cairo_stroke(cr);
  

  /*objecto*/


  cairo_set_source_rgb (cr, 0.0, 0.8, 1.0);
  cairo_set_line_width (cr, 5.);                          

  if(pinfo->caso==2)   
    {                                                                         //lente convexa/convergente
      cairo_curve_to(cr, w/2, pos2 + 4*t, w/2 + t*corr_r, pos2, w/2, pos2 -4*t);
      cairo_curve_to(cr, w/2, pos2 - 4*t, w/2 - t*corr_r, pos2, w/2, pos2 +4*t);
    }
  else
    {                                                                         //lente concava/divergente
      cairo_curve_to(cr, w/2 - t, pos2 + 4*t, w/2-(t*.25*(1-corr_r)), pos2, w/2 -t, pos2- 4*t);
      cairo_line_to (cr, w/2 + t, pos2 - 4*t);
      cairo_curve_to(cr, w/2 + t, pos2 - 4*t, w/2+(t*.25*(1-corr_r)), pos2, w/2 +t, pos2+ 4*t);
      cairo_line_to (cr, w/2 - t, pos2 + 4*t);
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
  gint i1;
  ProgInfo *pinfo;
  GtkWidget *label_frame,*label_comandos,*label;

  pinfo  = (ProgInfo *) calloc (1, sizeof (ProgInfo));

  gtk_init(&argc, &argv);

  pinfo->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_position(GTK_WINDOW(pinfo->window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(pinfo->window), 600, 400);
  gtk_window_set_title(GTK_WINDOW(pinfo->window), "Lentes delgadas");

  gtk_container_set_border_width(GTK_CONTAINER(pinfo->window), 10);

  //--------------------------------
  //    Tabelas Organizadoras
  //--------------------------------

  pinfo->table1 = gtk_table_new(3, 3, TRUE);
  gtk_table_set_row_spacings(GTK_TABLE(pinfo->table1), 10);
  gtk_table_set_col_spacings(GTK_TABLE(pinfo->table1), 10);
  gtk_container_add(GTK_CONTAINER(pinfo->window), pinfo->table1);

  pinfo->table2 = gtk_table_new(3, 1, TRUE);
  gtk_table_set_row_spacings(GTK_TABLE(pinfo->table2), 0);
  gtk_table_set_col_spacings(GTK_TABLE(pinfo->table2), 0);

  pinfo->table3 = gtk_table_new(4, 1, TRUE);
  gtk_table_set_row_spacings(GTK_TABLE(pinfo->table3), 0);
  gtk_table_set_col_spacings(GTK_TABLE(pinfo->table3), 0);

  frame1 = gtk_frame_new("Lente Concava");
  gtk_frame_set_shadow_type(GTK_FRAME(frame1), GTK_SHADOW_IN);
  pinfo->frame2 = gtk_frame_new("Comandos");
  gtk_frame_set_shadow_type(GTK_FRAME(pinfo->frame2), GTK_SHADOW_OUT);

  pinfo->frame_comandos[4] = gtk_frame_new("Índice de refracção");
  gtk_frame_set_label_align (pinfo->frame_comandos[4],1.0,0.0);
  gtk_frame_set_shadow_type(GTK_FRAME(pinfo->frame_comandos[4]), GTK_SHADOW_IN);
  pinfo->frame_comandos[3] = gtk_frame_new("Raio de curvatura");
  gtk_frame_set_label_align (pinfo->frame_comandos[3],1.0,0.0);
  gtk_frame_set_shadow_type(GTK_FRAME(pinfo->frame_comandos[3]), GTK_SHADOW_IN);
  pinfo->frame_comandos[2] = gtk_frame_new("Distância Objecto-Lente");
  gtk_frame_set_label_align (pinfo->frame_comandos[2],1.0,0.0);
  gtk_frame_set_shadow_type(GTK_FRAME(pinfo->frame_comandos[2]), GTK_SHADOW_IN);
  pinfo->frame_comandos[1] = gtk_frame_new("Tamanho Objecto");
  gtk_frame_set_label_align (pinfo->frame_comandos[1],1.0,0.0);
  gtk_frame_set_shadow_type(GTK_FRAME(pinfo->frame_comandos[1]), GTK_SHADOW_IN);

  gtk_table_attach_defaults(GTK_TABLE(pinfo->table1), frame1, 0, 3, 0, 2);
  gtk_table_attach_defaults(GTK_TABLE(pinfo->table1), pinfo->frame2, 0, 2, 2, 3);
  gtk_table_attach_defaults(GTK_TABLE(pinfo->table1), pinfo->frame3, 0, 2, 2, 3);
  gtk_table_attach_defaults(GTK_TABLE(pinfo->table1), pinfo->table2, 2, 3, 2, 3);

  gtk_container_add(GTK_CONTAINER(pinfo->frame2), pinfo->table3);
 
  gtk_table_attach_defaults(GTK_TABLE(pinfo->table3), pinfo->frame_comandos[4], 0, 1, 0, 1);
  gtk_table_attach_defaults(GTK_TABLE(pinfo->table3), pinfo->frame_comandos[3], 0, 1, 1, 2);
  gtk_table_attach_defaults(GTK_TABLE(pinfo->table3), pinfo->frame_comandos[2], 0, 1, 2, 3);
  gtk_table_attach_defaults(GTK_TABLE(pinfo->table3), pinfo->frame_comandos[1], 0, 1, 3, 4);

  pinfo->movebox = gtk_hbox_new (FALSE,0);
  gtk_container_add(GTK_CONTAINER (frame1),pinfo->movebox);

  pinfo->scalebox = gtk_hbox_new (FALSE,0);
  gtk_container_add(GTK_CONTAINER (pinfo->frame2),pinfo->scalebox);

  /*o resto dos attachments estão junto dos widgets respectivos*/

  //------------------------------
  //    Tabela com os controlos
  //------------------------------

  gtk_box_set_spacing(GTK_BOX(pinfo->scalebox),10);

  //Indice de refracção//
  pinfo->scale_box[4] = gtk_hbox_new(TRUE,0);
  pinfo->adj[4] = (GtkAdjustment *) gtk_adjustment_new (3., 1., 5., 0.001, 1.0, 0); 
  pinfo->scale[4] = gtk_hscale_new (GTK_ADJUSTMENT (pinfo->adj[4]));
  g_signal_connect (pinfo->adj[4], "value-changed", G_CALLBACK (change_scale), pinfo);
  gtk_scale_set_digits (GTK_SCALE (pinfo->scale[4]), 2);
  gtk_box_pack_start (GTK_BOX (pinfo->scale_box[4]), pinfo->scale[4], FALSE, TRUE, 5);
  gtk_container_add(GTK_CONTAINER (pinfo->frame_comandos[4]),pinfo->scale_box[4]);

  //raio de curvatura
  pinfo->scale_box[3] = gtk_hbox_new(TRUE,0);
  pinfo->adj[3] = (GtkAdjustment *) gtk_adjustment_new (3., 1., 10., 0.001, 1.0, 0); 
  pinfo->scale[3] = gtk_hscale_new (GTK_ADJUSTMENT (pinfo->adj[3]));
  g_signal_connect (pinfo->adj[3], "value-changed", G_CALLBACK (change_scale), pinfo);
  gtk_scale_set_digits (GTK_SCALE (pinfo->scale[3]), 2);
  gtk_box_pack_start (GTK_BOX (pinfo->scale_box[3]), pinfo->scale[3], FALSE, TRUE, 5);
  gtk_container_add(GTK_CONTAINER (pinfo->frame_comandos[3]),pinfo->scale_box[3]);

  //distância objecto-lente
  pinfo->scale_box[2] = gtk_hbox_new(TRUE,0);
  pinfo->adj[2] = (GtkAdjustment *) gtk_adjustment_new (.3, 0.0, 1.0, 0.001, 1.0, 0); 
  pinfo->scale[2] = gtk_hscale_new (GTK_ADJUSTMENT (pinfo->adj[2]));
  g_signal_connect (pinfo->adj[2], "value-changed", G_CALLBACK (change_scale), pinfo);
  gtk_scale_set_digits (GTK_SCALE (pinfo->scale[2]), 2);
  gtk_box_pack_start (GTK_BOX (pinfo->scale_box[2]), pinfo->scale[2], FALSE, TRUE, 5);
  gtk_container_add(GTK_CONTAINER (pinfo->frame_comandos[2]),pinfo->scale_box[2]);

  //distância tamanho do objecto
  pinfo->scale_box[1] = gtk_hbox_new(TRUE,0);
  pinfo->adj[1] = (GtkAdjustment *) gtk_adjustment_new (.3, 0.0, 1.0, 0.001, 1.0, 0); 
  pinfo->scale[1] = gtk_hscale_new (GTK_ADJUSTMENT (pinfo->adj[1]));
  g_signal_connect (pinfo->adj[1], "value-changed", G_CALLBACK (change_scale), pinfo);
  gtk_scale_set_digits (GTK_SCALE (pinfo->scale[1]), 2);
  gtk_box_pack_start (GTK_BOX (pinfo->scale_box[1]), pinfo->scale[1], FALSE, TRUE, 5);
  gtk_container_add(GTK_CONTAINER (pinfo->frame_comandos[1]),pinfo->scale_box[1]);

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

  pinfo->hbox = gtk_hbox_new(TRUE,0);
  pinfo->toggle = gtk_toggle_button_new_with_label("Muda Modo");
  gtk_widget_set_usize(pinfo->toggle,100,24);
  gtk_box_pack_start(GTK_BOX(pinfo->hbox), pinfo->toggle, FALSE,FALSE,0);
  g_signal_connect(pinfo->toggle, "toggled", G_CALLBACK(change_state), (gpointer)pinfo);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(pinfo->toggle), TRUE);
  gtk_table_attach_defaults(GTK_TABLE(pinfo->table2), pinfo->hbox, 0, 1, 1, 2);
  
  
  //--------------------------------
  //    Corzinhas
  //--------------------------------
  /*
  scale_set_color (pinfo); 
  gtk_widget_modify_bg (pinfo->framelente, GTK_STATE_NORMAL, &pinfo->cor.frame_lente);
  gtk_widget_modify_bg (pinfo->framelente, GTK_STATE_NORMAL, &pinfo->cor.frame_comandos);

  label_lente = gtk_frame_get_label_widget (GTK_FRAME (pinfo->frame_lente));
  gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &pinfo->cor.label_lente);
  label_comandos = gtk_frame_get_label_widget (GTK_FRAME (pinfo->frame_comandos));
  gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &pinfo->cor.label_comandos)
  gtk_widget_modify_bg (pinfo->window, 0, &pinfo->cor.window);
  
  label = gtk_bin_get_child (GTK_BIN(pinfo->bt_exit));
  for (i1 = GTK_STATE_NORMAL ; i1 <= GTK_STATE_PRELIGHT ; ++i1)
    {
      gtk_widget_modify_bg (pinfo->bt_mudamodo, i1, &pinfo->cor.bt_hide);
      gtk_widget_modify_fg (label, i1, &pinfo->cor.black);
    }

  */
  g_signal_connect_swapped(G_OBJECT(pinfo->window), "destroy",
			   G_CALLBACK(gtk_main_quit), G_OBJECT(pinfo->window));

  gtk_widget_show_all(pinfo->window);

  gtk_main();

  return 0;
}
