#include <stdlib.h>
#include <gtk/gtk.h>
#include <string.h>
#include <math.h>
#include <cairo.h>

//note-se que o raio não é desenhado de forma precisa as coisas apenas funcionam proporcionalmente
#define N_AR               1
#define VELOC_INIT_X     3.0
#define VELOC_INIT_Y     2.0

typedef struct
{
  GdkColor window        ;
  GdkColor scale         ;
  GdkColor zero          ;
  GdkColor frame1        ;
  GdkColor frame         ;
  GdkColor frame3        ;
  GdkColor frame_comandos;
  GdkColor label_comandos;
  GdkColor cor_seta      ;

  GdkColor bt_toggle     ;
  GdkColor black         ;
} ProgCores;

typedef struct
{
  GtkWidget *window           ;
  GtkWidget *vbox             ;
  GtkWidget *table1           ;           //tabela que organiza tudo
  GtkWidget *table2           ;           //tabela para centrar um butão
  GtkWidget *table3           ;           //tabela com os controlos
  GtkWidget *hbox             ;           //caixa com 
  GtkWidget *hbox2            ;
  GtkWidget *hbox3            ;
  GtkWidget *frame1           ;
  GtkWidget *frame2           ;           //frame abaixo (controlos e butões)
  //GtkWidget *frame3           ;           //frame onde estão os controlos
  GtkWidget *label1           ;
  GtkWidget *label2           ;
  GtkWidget *label_frame[3]   ;
  GtkWidget *frame_comandos[4];  
  GtkWidget *label_comandos[4]; 
  ProgCores cor               ;


  GtkWidget *scale[4]    ;          //escalas onde o utilizador mexe 
  GtkAdjustment *adj[4]  ;          //adjustments para as escalas  
  GtkWidget *scale_box[4];          //caixas para as escalas 

  GtkWidget *hbox_color       ;           //caixa com o tool item que chama a roda das corzinhas
  GtkWidget *hbox_legenda     ;           //caixa com legenda para a cor escolhida
  GtkWidget *label_legenda    ;           //legenda para a cor escolhida
  GtkWidget *toolbar          ;           //toolbar das coreszinhas
  GtkToolItem *font           ;           //butão onde se clica para chamar uma nova janela
  
  gboolean move  ;
  double   df    ;

  GtkWidget *toggle;          //butão que muda lente côncava para convexa
  gint caso        ;

}ProgInfo;

void 
color_create (ProgInfo  *pinfo)
{
  gdk_color_parse ("#000000", &pinfo->cor.window);

  gdk_color_parse ("#C1CDC1", &pinfo->cor.zero    );

  gdk_color_parse ("cyan",    &pinfo->cor.frame   );
  gdk_color_parse ("cyan",    &pinfo->cor.scale   );
  gdk_color_parse ("#FF0000", &pinfo->cor.frame_comandos);
  gdk_color_parse ("#00CDCD", &pinfo->cor.label_comandos);

  gdk_color_parse ("#000000", &pinfo->cor.bt_toggle);

  gdk_color_parse ("#000000", &pinfo->cor.black);
}

void 
scale_set_color (ProgInfo  *pinfo) //GtkWidget  *w  ,   GdkColor   *c1 ,   GdkColor   *c0 )
{
  int i1 ;

  for (i1 = 0 ; i1 < 4 ; ++i1)
    {
      gtk_widget_modify_bg (pinfo->scale[i1], GTK_STATE_NORMAL, &pinfo->cor.scale);
      gtk_widget_modify_bg (pinfo->scale[i1], GTK_STATE_ACTIVE, &pinfo->cor.scale);
      gtk_widget_modify_bg (pinfo->scale[i1], GTK_STATE_ACTIVE, &pinfo->cor.zero); 
      gtk_widget_modify_bg (pinfo->scale[i1], GTK_STATE_SELECTED, &pinfo->cor.scale);
      gtk_widget_modify_bg (pinfo->scale[i1], GTK_STATE_PRELIGHT, &pinfo->cor.scale);
      gtk_widget_modify_fg (pinfo->scale[i1], GTK_STATE_NORMAL, &pinfo->cor.scale);
    }
}

void select_font(GtkWidget *widget, ProgInfo *info)
{
  ProgInfo *pinfo;
  GtkResponseType result;
  GtkColorSelection *colorsel;
  pinfo=info;

  GtkWidget *dialog = gtk_color_selection_dialog_new("Cor do Objecto");
  result = gtk_dialog_run(GTK_DIALOG(dialog));

  if (result == GTK_RESPONSE_OK)
  {
    colorsel = GTK_COLOR_SELECTION(GTK_COLOR_SELECTION_DIALOG(dialog)->colorsel);
    gtk_color_selection_get_current_color(colorsel,&pinfo->cor.cor_seta);
    gtk_widget_modify_fg(GTK_WIDGET(pinfo->label_legenda),GTK_STATE_NORMAL,&pinfo->cor.cor_seta);
  } 

  gtk_widget_destroy(dialog);
}

static void change_state(GtkWidget* w, ProgInfo *pinfo)                    //Muda a legenda consoante a 
{                                                                          //lente é convergente ou divergente

  if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(w)))
    {
      g_print("Toggle button activated\n");
      gtk_frame_set_label(GTK_FRAME(pinfo->frame1),"Lente Concava");
      gtk_widget_modify_bg (pinfo->frame1,GTK_STATE_NORMAL, &pinfo->cor.frame)        ;
      pinfo->label1 = gtk_frame_get_label_widget (GTK_FRAME (pinfo->frame1))          ;
      gtk_widget_modify_fg (pinfo->label1,GTK_STATE_NORMAL,&pinfo->cor.label_comandos);
      pinfo->caso=1;
    }
  else
    {
      g_print("Toggle button not activated\n");
      gtk_frame_set_label(GTK_FRAME(pinfo->frame1),"Lente Convexa");
      gtk_widget_modify_bg (pinfo->frame1,GTK_STATE_NORMAL, &pinfo->cor.frame)        ;
      pinfo->label1 = gtk_frame_get_label_widget (GTK_FRAME (pinfo->frame1))          ;
      gtk_widget_modify_fg (pinfo->label1,GTK_STATE_NORMAL,&pinfo->cor.label_comandos);
      pinfo->caso=2;
    }
}


gboolean
desenha_concava (GtkWidget       *widget ,
                 GdkEventExpose  *event  ,
                 ProgInfo *info)
{
  ProgInfo  *pinfo            ;

  /*será melhor por estas vareáveis todas em ProgInfo?*/
  cairo_t   *cr               ;
  gint       w, h             ;           //width and height of the window
  double     df               ;           //distância focal
  double     i                ;           //indice de refracção <-> relacionado com a transparência ^^'
  double     alpha            ;           //transparência
  double     r                ;           //raio de curvatura
  double     corr_r           ;           //vareavel que está relacionada com o valor do raio
  double     pos1, pos2       ;           //posições do objecto
  double     size_obj         ;           //tamanho do objecto
  double     t                ;           //varia o tamanho da lente e das coisa em geral
                                          //de acordo coom o tamanho da janela
  pinfo  = info;
  

  gtk_window_get_size (GTK_WINDOW(pinfo->window), &w, &h);
  t = h/20;                     //t = as unidades para desenhar tudo em escala com a janela

  i        =  gtk_adjustment_get_value (pinfo->adj[3]);
  alpha    =  1.3*(i-1);
  r        =  gtk_adjustment_get_value (pinfo->adj[2]);
  corr_r   =  r * .1;
  pos1     =  w/2-gtk_adjustment_get_value (pinfo->adj[1]);
  pos2     =  h/3.;
  size_obj =  gtk_adjustment_get_value (pinfo->adj[0]);

  //calcula a distância focal através da "Lensmaker's equation"
  i      = i / N_AR;
  df     = (4*t*corr_r) / (i-1);
  df     = .5*df;
 
  //---inicio dos desenhinhos---//
  cr = gdk_cairo_create (pinfo->window->window);
 

  /*Linha horizontal Principal*/
  cairo_set_source_rgb (cr, 1., 1., 1.);           
  cairo_set_line_width (cr, 2);
  cairo_move_to (cr, 15,  pos2);
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
  
  //cairo_set_source_rgb (cr, 1.0, 0.8, 1.0);
  gdk_cairo_set_source_color(cr,&pinfo->cor.cor_seta);
  cairo_set_line_width (cr, 1.); 

  /*objecto*/
  cairo_rectangle(cr, pos1 - size_obj * .1 * t, pos2 - size_obj * 1.1 * t, size_obj * .2 * t,size_obj * 1.1 * t);
  cairo_fill     (cr);
  cairo_stroke   (cr);
  cairo_move_to  (cr, pos1    , pos2 - 2 * size_obj * t);
  cairo_line_to  (cr, pos1 - size_obj * .5 * t, pos2 - 2 * size_obj * t + size_obj * t) ;
  cairo_line_to  (cr, pos1 + size_obj * .5 * t, pos2 - 2 * size_obj * t + size_obj * t) ;
  cairo_line_to  (cr, pos1    , pos2 - 2 * size_obj * t);
  cairo_fill     (cr);
  cairo_stroke   (cr);

  /*raios*/
  cairo_set_source_rgb (cr, 1.0, 1.0, 0);
  cairo_move_to(cr, pos1 ,pos2 - 2 * size_obj * t);
  cairo_line_to(cr, w/2, pos2 - 2 * size_obj * t);
  cairo_stroke (cr);

  cairo_set_source_rgba (cr, 0.0, 0.8, 1.0,alpha); //red,green,blue,alpha(=leia-se transparência)
  cairo_set_line_width (cr, 5.);                          

  if(pinfo->caso==2)   
    {                                                                         //lente convexa/convergente
      cairo_curve_to(cr, w/2, pos2 + 4*t, w/2 + 2*t*corr_r, pos2, w/2, pos2 -4*t);
      cairo_curve_to(cr, w/2, pos2 - 4*t, w/2 - 2*t*corr_r, pos2, w/2, pos2 +4*t);
    }
  else
    {                                                                         //lente concava/divergente
      cairo_curve_to(cr, w/2 - t, pos2 + 4*t, w/2-(t*.8*(1-corr_r)), pos2, w/2 -t, pos2- 4*t);
      cairo_line_to (cr, w/2 + t, pos2 - 4*t);
      cairo_curve_to(cr, w/2 + t, pos2 - 4*t, w/2+(t*.8*(1-corr_r)), pos2, w/2 +t, pos2+ 4*t);
      cairo_line_to (cr, w/2 - t, pos2 + 4*t);
    }
  cairo_fill(cr);
  cairo_stroke (cr);
  //-----fim dos desenhinhos-----//
  


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
  int i1;
  ProgInfo *pinfo;



  pinfo  = (ProgInfo *) calloc (1, sizeof (ProgInfo));

  gtk_init       (&argc, &argv);
  color_create   (pinfo)       ;
 

  pinfo->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

  gtk_window_set_position(GTK_WINDOW(pinfo->window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(pinfo->window), 600, 400);
  gtk_window_set_title(GTK_WINDOW(pinfo->window), "Lentes delgadas");

  gtk_container_set_border_width(GTK_CONTAINER(pinfo->window), 10);

  pinfo->vbox = gtk_vbox_new(TRUE,0);
  gtk_container_add(GTK_CONTAINER(pinfo->window),pinfo->vbox);

  //--------------------------------
  //    Tabelas Organizadoras
  //--------------------------------

  pinfo->table1 = gtk_table_new(3, 3, TRUE);
  gtk_table_set_row_spacings(GTK_TABLE(pinfo->table1), 10);
  gtk_table_set_col_spacings(GTK_TABLE(pinfo->table1), 10);
  gtk_container_add(GTK_CONTAINER(pinfo->vbox), pinfo->table1);

  pinfo->table2 = gtk_table_new(3, 1, TRUE);
  gtk_table_set_row_spacings(GTK_TABLE(pinfo->table2), 0);
  gtk_table_set_col_spacings(GTK_TABLE(pinfo->table2), 0);

  pinfo->table3 = gtk_table_new(2, 2, TRUE);
  gtk_table_set_row_spacings(GTK_TABLE(pinfo->table3), 0);
  gtk_table_set_col_spacings(GTK_TABLE(pinfo->table3), 0);

  pinfo->frame1 = gtk_frame_new("Lente Concava");
  gtk_frame_set_shadow_type(GTK_FRAME(pinfo->frame1), GTK_SHADOW_IN);
  pinfo->frame2 = gtk_frame_new("Comandos");
  gtk_frame_set_shadow_type(GTK_FRAME(pinfo->frame2), GTK_SHADOW_OUT);

  pinfo->frame_comandos[3] = gtk_frame_new("Índice de refracção");
  gtk_frame_set_label_align (GTK_FRAME(pinfo->frame_comandos[3]),1.0,0.0);
  gtk_frame_set_shadow_type(GTK_FRAME(pinfo->frame_comandos[3]), GTK_SHADOW_IN);
  pinfo->frame_comandos[2] = gtk_frame_new("Raio de curvatura");
  gtk_frame_set_label_align (GTK_FRAME(pinfo->frame_comandos[2]),1.0,0.0);
  gtk_frame_set_shadow_type(GTK_FRAME(pinfo->frame_comandos[2]), GTK_SHADOW_IN);
  pinfo->frame_comandos[1] = gtk_frame_new("Distância Objecto-Lente");
  gtk_frame_set_label_align (GTK_FRAME(pinfo->frame_comandos[1]),1.0,0.0);
  gtk_frame_set_shadow_type(GTK_FRAME(pinfo->frame_comandos[1]), GTK_SHADOW_IN);
  pinfo->frame_comandos[0] = gtk_frame_new("Tamanho Objecto");
  gtk_frame_set_label_align (GTK_FRAME(pinfo->frame_comandos[0]),1.0,0.0);
  gtk_frame_set_shadow_type(GTK_FRAME(pinfo->frame_comandos[0]), GTK_SHADOW_IN);

  gtk_table_attach_defaults(GTK_TABLE(pinfo->table1), pinfo->frame1, 0, 3, 0, 2);
  gtk_table_attach_defaults(GTK_TABLE(pinfo->table1), pinfo->frame2, 0, 2, 2, 3);
  //gtk_table_attach_defaults(GTK_TABLE(pinfo->table1), pinfo->frame3, 0, 2, 2, 3);

  pinfo->hbox3 = gtk_hbox_new(TRUE,0);
  gtk_table_attach_defaults(GTK_TABLE(pinfo->table1), pinfo->hbox3, 2, 3, 2, 3);
  gtk_box_pack_start (GTK_BOX (pinfo->hbox3), pinfo->table2, FALSE, TRUE, 5);
 
  gtk_container_add(GTK_CONTAINER(pinfo->frame2), pinfo->table3);
  
 
  gtk_table_attach_defaults(GTK_TABLE(pinfo->table3), pinfo->frame_comandos[3], 0, 1, 0, 1);
  gtk_table_attach_defaults(GTK_TABLE(pinfo->table3), pinfo->frame_comandos[2], 0, 1, 1, 2);
  gtk_table_attach_defaults(GTK_TABLE(pinfo->table3), pinfo->frame_comandos[1], 1, 2, 0, 1);
  gtk_table_attach_defaults(GTK_TABLE(pinfo->table3), pinfo->frame_comandos[0], 1, 2, 1, 2);


  /*o resto dos attachments estão junto dos widgets respectivos*/

  //------------------------------
  //    Tabela com os controlos
  //------------------------------
  
  //Indice de refracção//
  pinfo->scale_box[3] = gtk_hbox_new(TRUE,0);
  pinfo->adj[3] = (GtkAdjustment *) gtk_adjustment_new (1.3, 1.2, 1.5, 0.001, 1.0, 0); 
  pinfo->scale[3] = gtk_hscale_new (GTK_ADJUSTMENT (pinfo->adj[3]));
  g_signal_connect (pinfo->adj[3], "value-changed", G_CALLBACK (change_scale), pinfo);
  gtk_scale_set_digits (GTK_SCALE (pinfo->scale[3]), 2);
  gtk_box_pack_start (GTK_BOX (pinfo->scale_box[3]), pinfo->scale[3], FALSE, TRUE, 5);
  gtk_container_add(GTK_CONTAINER (pinfo->frame_comandos[3]),pinfo->scale_box[3]);

  //raio de curvatura
  pinfo->scale_box[2] = gtk_hbox_new(TRUE,0);
  pinfo->adj[2] = (GtkAdjustment *) gtk_adjustment_new (3., 1., 8.5, 0.001, 1.0, 0); 
  pinfo->scale[2] = gtk_hscale_new (GTK_ADJUSTMENT (pinfo->adj[2]));
  g_signal_connect (pinfo->adj[2], "value-changed", G_CALLBACK (change_scale), pinfo);
  gtk_scale_set_digits (GTK_SCALE (pinfo->scale[2]), 2);
  gtk_box_pack_start (GTK_BOX (pinfo->scale_box[2]), pinfo->scale[2], FALSE, TRUE, 5);
  gtk_container_add(GTK_CONTAINER (pinfo->frame_comandos[2]),pinfo->scale_box[2]);

  //distância objecto-lente
  pinfo->scale_box[1] = gtk_hbox_new(TRUE,0);
  pinfo->adj[1] = (GtkAdjustment *) gtk_adjustment_new (100, 0.0, 500, 0.001, 1.0, 0); 
  pinfo->scale[1] = gtk_hscale_new (GTK_ADJUSTMENT (pinfo->adj[1]));
  g_signal_connect (pinfo->adj[1], "value-changed", G_CALLBACK (change_scale), pinfo);
  gtk_scale_set_digits (GTK_SCALE (pinfo->scale[1]), 2);
  gtk_box_pack_start (GTK_BOX (pinfo->scale_box[1]), pinfo->scale[1], FALSE, TRUE, 5);
  gtk_container_add(GTK_CONTAINER (pinfo->frame_comandos[1]),pinfo->scale_box[1]);

  //tamanho do objecto
  pinfo->scale_box[0] = gtk_hbox_new(TRUE,0);
  pinfo->adj[0] = (GtkAdjustment *) gtk_adjustment_new (.3, 0.0, 2.0, 0.001, 1.0, 0); 
  pinfo->scale[0] = gtk_hscale_new (GTK_ADJUSTMENT (pinfo->adj[0]));
  g_signal_connect (pinfo->adj[0], "value-changed", G_CALLBACK (change_scale), pinfo);
  gtk_scale_set_digits (GTK_SCALE (pinfo->scale[0]), 2);
  gtk_box_pack_start (GTK_BOX (pinfo->scale_box[0]), pinfo->scale[0], FALSE, TRUE, 5);
  gtk_container_add(GTK_CONTAINER (pinfo->frame_comandos[0]),pinfo->scale_box[0]);

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
  
  scale_set_color(pinfo);
  
  gtk_widget_modify_bg (pinfo->frame2,GTK_STATE_NORMAL, &pinfo->cor.frame)        ;
  pinfo->label2 = gtk_frame_get_label_widget (GTK_FRAME (pinfo->frame2))          ;
  gtk_widget_modify_fg (pinfo->label2,GTK_STATE_NORMAL,&pinfo->cor.label_comandos);
  
  for(i1=0;i1<4;++i1)
    {
      gtk_widget_modify_bg (pinfo->frame_comandos[i1],GTK_STATE_NORMAL, &pinfo->cor.window)         ;
      pinfo->label_comandos[i1] = gtk_frame_get_label_widget (GTK_FRAME (pinfo->frame_comandos[i1]));
      gtk_widget_modify_fg (pinfo->label_comandos[i1],GTK_STATE_NORMAL,&pinfo->cor.label_comandos)  ;
    }

  gtk_widget_modify_bg (pinfo->window, 0, &pinfo->cor.window);

  //--------------------------------
  //    Roda das Corzinhas (é preciso por o botão num sítio mais bonito - e, se possível mudar-lhe a forma)
  //--------------------------------

  pinfo->hbox_color   = gtk_hbox_new(TRUE,0);
  pinfo->hbox_legenda = gtk_hbox_new(TRUE,0);

  pinfo->toolbar    = gtk_toolbar_new();
  gtk_toolbar_set_style(GTK_TOOLBAR(pinfo->toolbar), GTK_TOOLBAR_ICONS);
  gtk_container_set_border_width(GTK_CONTAINER(pinfo->toolbar), 2);

  pinfo->font = gtk_tool_button_new_from_stock(GTK_STOCK_SELECT_COLOR);
  gtk_toolbar_insert(GTK_TOOLBAR(pinfo->toolbar), pinfo->font, -1);

  gtk_box_pack_start(GTK_BOX(pinfo->hbox_color), pinfo->toolbar, FALSE, FALSE, 5);

  pinfo->label_legenda = gtk_label_new("Cor escolhida");
  gtk_label_set_justify(GTK_LABEL(pinfo->label_legenda), GTK_JUSTIFY_CENTER);
  gtk_box_pack_start(GTK_BOX(pinfo->hbox_legenda), pinfo->label_legenda, TRUE, FALSE, 5);

  gtk_table_attach_defaults(GTK_TABLE(pinfo->table2), pinfo->hbox_color  , 0, 1, 0, 1);
  gtk_table_attach_defaults(GTK_TABLE(pinfo->table2), pinfo->hbox_legenda, 0, 1, 2, 3);

  //g_signal_connect(G_OBJECT(pinfo->font), "clicked",G_CALLBACK(select_font), pinfo->label_legenda);
  g_signal_connect(G_OBJECT(pinfo->font), "clicked",G_CALLBACK(select_font), pinfo);
 
  g_signal_connect_swapped(G_OBJECT(pinfo->window), "destroy",
			   G_CALLBACK(gtk_main_quit), G_OBJECT(pinfo->window));

  gtk_widget_show_all(pinfo->window);

  gtk_main();

  return 0;
}
