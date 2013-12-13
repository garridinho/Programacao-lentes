#include <gtk/gtk.h>
#include <string.h>
 
static void menu_response(GtkWidget* menu_item, gpointer data)
{
  if(strcmp(gtk_menu_item_get_label(GTK_MENU_ITEM(menu_item)), "New") == 0)       // equal
    {
      g_print("You pressed New\n");
    }
  if(strcmp(gtk_menu_item_get_label(GTK_MENU_ITEM(menu_item)), "Exit") == 0)      // equal
    {
      g_print("You pressed Exit\n");
    }
  if(strcmp(gtk_menu_item_get_label(GTK_MENU_ITEM(menu_item)), "About") == 0)     // equal
    {
      g_print("You pressed About\n");
    }
}

gboolean tarefa_butao(GtkWidget* button, gpointer data)
{
  g_print("You pressed a button\n");
}

int main(int argc, char* argv[])
{
  gtk_init(&argc, &argv);
  GtkWidget *window, *menu_bar, *menu_item, *file_menu, *help_menu, *vbox, *button;
  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(window, "delete-event", G_CALLBACK(gtk_main_quit), NULL);
  
  menu_bar = gtk_menu_bar_new();
  
  file_menu = gtk_menu_new();
  help_menu = gtk_menu_new();
  
  menu_item = gtk_menu_item_new_with_label("File");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), file_menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
 
  menu_item = gtk_menu_item_new_with_label("Help");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(menu_item), help_menu);
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), menu_item);
 
  menu_item = gtk_menu_item_new_with_label("New");
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);
  g_signal_connect(menu_item, "activate", G_CALLBACK(menu_response), NULL);
 
  menu_item = gtk_menu_item_new_with_label("Exit");
  gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), menu_item);
  g_signal_connect(menu_item, "activate", G_CALLBACK(menu_response), NULL);
 
  menu_item = gtk_menu_item_new_with_label("About");
  gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), menu_item);
  g_signal_connect(menu_item, "activate", G_CALLBACK(menu_response), NULL);
 
  vbox = gtk_vbox_new(0,0);
  button = gtk_button_new_with_label("This is a Button");
  gtk_box_pack_start(GTK_BOX(vbox), menu_bar,0,0,0);
  gtk_box_pack_start(GTK_BOX(vbox), button,0,0,0);
  g_signal_connect(button, "clicked", G_CALLBACK(tarefa_butao), NULL);
 
  gtk_container_add(GTK_CONTAINER(window), vbox);
 
  gtk_widget_show_all(window);
  gtk_main();
  return 0;
}
