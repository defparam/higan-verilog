#if defined(Hiro_TableView)

namespace hiro {

static auto TableView_activate(GtkTreeView*, GtkTreePath*, GtkTreeViewColumn*, pTableView* p) -> void { return p->_doActivate(); }
static auto TableView_buttonEvent(GtkTreeView* treeView, GdkEventButton* event, pTableView* p) -> signed { return p->_doEvent(event); }
static auto TableView_change(GtkTreeSelection*, pTableView* p) -> void { return p->_doChange(); }
static auto TableView_edit(GtkCellRendererText* renderer, const char* path, const char* text, pTableView* p) -> void { return p->_doEdit(renderer, path, text); }
static auto TableView_headerActivate(GtkTreeViewColumn* column, pTableView* p) -> void { return p->_doHeaderActivate(column); }
static auto TableView_mouseMoveEvent(GtkWidget*, GdkEvent*, pTableView* p) -> signed { return p->_doMouseMove(); }
static auto TableView_popup(GtkTreeView*, pTableView* p) -> void { return p->_doContext(); }

static auto TableView_dataFunc(GtkTreeViewColumn* column, GtkCellRenderer* renderer, GtkTreeModel* model, GtkTreeIter* iter, pTableView* p) -> void { return p->_doDataFunc(column, renderer, iter); }
static auto TableView_toggle(GtkCellRendererToggle* toggle, const char* path, pTableView* p) -> void { return p->_doToggle(toggle, path); }

//gtk_tree_view_set_rules_hint(gtkTreeView, true);

auto pTableView::construct() -> void {
  gtkWidget = gtk_scrolled_window_new(0, 0);
  gtkScrolledWindow = GTK_SCROLLED_WINDOW(gtkWidget);
  gtk_scrolled_window_set_policy(gtkScrolledWindow, GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(gtkScrolledWindow, GTK_SHADOW_ETCHED_IN);

  gtkWidgetChild = gtk_tree_view_new();
  gtkTreeView = GTK_TREE_VIEW(gtkWidgetChild);
  gtkTreeSelection = gtk_tree_view_get_selection(gtkTreeView);
  gtk_container_add(GTK_CONTAINER(gtkWidget), gtkWidgetChild);
  gtk_tree_view_set_rubber_banding(gtkTreeView, true);

  gtk_widget_show(gtkWidgetChild);

  setBackgroundColor(state().backgroundColor);
  setBatchable(state().batchable);
  setBordered(state().bordered);
  setFont(self().font(true));
  setForegroundColor(state().foregroundColor);

  g_signal_connect(G_OBJECT(gtkTreeView), "button-press-event", G_CALLBACK(TableView_buttonEvent), (gpointer)this);
  g_signal_connect(G_OBJECT(gtkTreeView), "button-release-event", G_CALLBACK(TableView_buttonEvent), (gpointer)this);
  g_signal_connect(G_OBJECT(gtkTreeView), "motion-notify-event", G_CALLBACK(TableView_mouseMoveEvent), (gpointer)this);
  g_signal_connect(G_OBJECT(gtkTreeView), "popup-menu", G_CALLBACK(TableView_popup), (gpointer)this);
  g_signal_connect(G_OBJECT(gtkTreeView), "row-activated", G_CALLBACK(TableView_activate), (gpointer)this);
  g_signal_connect(G_OBJECT(gtkTreeSelection), "changed", G_CALLBACK(TableView_change), (gpointer)this);

  pWidget::construct();
}

auto pTableView::destruct() -> void {
  gtk_widget_destroy(gtkWidgetChild);
  gtk_widget_destroy(gtkWidget);
}

auto pTableView::append(sTableViewHeader header) -> void {
}

auto pTableView::append(sTableViewItem item) -> void {
}

auto pTableView::focused() const -> bool {
  return GTK_WIDGET_HAS_FOCUS(gtkTreeView);
}

auto pTableView::remove(sTableViewHeader header) -> void {
}

auto pTableView::remove(sTableViewItem item) -> void {
}

auto pTableView::resizeColumns() -> void {
  lock();

  if(auto& header = state().header) {
    vector<signed> widths;
    signed minimumWidth = 0;
    signed expandable = 0;
    for(auto column : range(header->columnCount())) {
      signed width = _width(column);
      widths.append(width);
      minimumWidth += width;
      if(header->column(column).expandable()) expandable++;
    }

    signed maximumWidth = self().geometry().width() - 6;
    if(auto scrollBar = gtk_scrolled_window_get_vscrollbar(gtkScrolledWindow)) {
      if(gtk_widget_get_visible(scrollBar)) maximumWidth -= scrollBar->allocation.width;
    }

    signed expandWidth = 0;
    if(expandable && maximumWidth > minimumWidth) {
      expandWidth = (maximumWidth - minimumWidth) / expandable;
    }

    for(auto column : range(header->columnCount())) {
      if(auto self = header->state.columns[column]->self()) {
        signed width = widths[column];
        if(self->state().expandable) width += expandWidth;
        gtk_tree_view_column_set_fixed_width(self->gtkColumn, width);
      }
    }
  }

  unlock();
}

auto pTableView::setAlignment(Alignment alignment) -> void {
}

auto pTableView::setBackgroundColor(Color color) -> void {
  GdkColor gdkColor = CreateColor(color);
  gtk_widget_modify_base(gtkWidgetChild, GTK_STATE_NORMAL, color ? &gdkColor : nullptr);
}

auto pTableView::setBatchable(bool batchable) -> void {
  gtk_tree_selection_set_mode(gtkTreeSelection, batchable ? GTK_SELECTION_MULTIPLE : GTK_SELECTION_SINGLE);
}

auto pTableView::setBordered(bool bordered) -> void {
  gtk_tree_view_set_grid_lines(gtkTreeView, bordered ? GTK_TREE_VIEW_GRID_LINES_BOTH : GTK_TREE_VIEW_GRID_LINES_NONE);
}

auto pTableView::setFocused() -> void {
  //gtk_widget_grab_focus() will select the first item if nothing is currently selected
  //this behavior is undesirable. detect selection state first, and restore if required
  lock();
  bool selected = gtk_tree_selection_get_selected(gtkTreeSelection, nullptr, nullptr);
  gtk_widget_grab_focus(gtkWidgetChild);
  if(!selected) gtk_tree_selection_unselect_all(gtkTreeSelection);
  unlock();
}

auto pTableView::setFont(const Font& font) -> void {
  if(auto& header = state().header) {
    if(auto self = header->self()) self->_setState();
  }
}

auto pTableView::setForegroundColor(Color color) -> void {
  GdkColor gdkColor = CreateColor(color);
  gtk_widget_modify_text(gtkWidgetChild, GTK_STATE_NORMAL, color ? &gdkColor : nullptr);
}

auto pTableView::setGeometry(Geometry geometry) -> void {
  pWidget::setGeometry(geometry);
  if(auto& header = state().header) {
    for(auto& column : header->state.columns) {
      if(column->state.expandable) return resizeColumns();
    }
  }
}

auto pTableView::_cellWidth(unsigned _row, unsigned _column) -> unsigned {
  unsigned width = 8;
  if(auto item = self().item(_row)) {
    if(auto cell = item->cell(_column)) {
      if(cell->state.checkable) {
        width += 24;
      }
      if(auto& icon = cell->state.icon) {
        width += icon.width() + 2;
      }
      if(auto& text = cell->state.text) {
        width += pFont::size(cell->font(true), text).width();
      }
    }
  }
  return width;
}

auto pTableView::_columnWidth(unsigned _column) -> unsigned {
  unsigned width = 8;
  if(auto& header = state().header) {
    if(auto column = header->column(_column)) {
      if(auto& icon = column->state.icon) {
        width += icon.width() + 2;
      }
      if(auto& text = column->state.text) {
        width += pFont::size(column->font(true), text).width();
      }
    }
  }
  return width;
}

auto pTableView::_createModel() -> void {
  gtk_tree_view_set_model(gtkTreeView, nullptr);
  gtkListStore = nullptr;
  gtkTreeModel = nullptr;

  vector<GType> types;
  if(auto& header = state().header) {
    for(auto column : header->state.columns) {
      if(auto self = column->self()) {
        if(!self->gtkColumn) continue;  //may not have been created yet; or recently destroyed
        types.append(G_TYPE_BOOLEAN);
        types.append(GDK_TYPE_PIXBUF);
        types.append(G_TYPE_STRING);
      }
    }
  }
  if(!types) return;  //no columns available

  gtkListStore = gtk_list_store_newv(types.size(), types.data());
  gtkTreeModel = GTK_TREE_MODEL(gtkListStore);
  gtk_tree_view_set_model(gtkTreeView, gtkTreeModel);
}

auto pTableView::_doActivate() -> void {
  if(!locked()) self().doActivate();
}

auto pTableView::_doChange() -> void {
  if(!locked()) _updateSelected();
}

auto pTableView::_doContext() -> void {
  if(!locked()) self().doContext();
}

auto pTableView::_doDataFunc(GtkTreeViewColumn* gtkColumn, GtkCellRenderer* renderer, GtkTreeIter* iter) -> void {
  auto path = gtk_tree_model_get_string_from_iter(gtkTreeModel, iter);
  auto row = toNatural(path);
  g_free(path);

  if(auto& header = state().header) {
    for(auto& column : header->state.columns) {
      if(auto p = column->self()) {
        if(renderer != GTK_CELL_RENDERER(p->gtkCellToggle)
        && renderer != GTK_CELL_RENDERER(p->gtkCellIcon)
        && renderer != GTK_CELL_RENDERER(p->gtkCellText)
        ) continue;
        if(auto item = self().item(row)) {
          if(auto cell = item->cell(column->offset())) {
            if(renderer == GTK_CELL_RENDERER(p->gtkCellToggle)) {
              gtk_cell_renderer_set_visible(renderer, cell->state.checkable);
            } else if(renderer == GTK_CELL_RENDERER(p->gtkCellText)) {
              auto alignment = cell->alignment(true);
              if(!alignment) alignment = {0.0, 0.5};
            //note: below line will center column header text; but causes strange glitches
            //(specifically, windows fail to respond to the close button ... some kind of heap corruption inside GTK+)
            //gtk_tree_view_column_set_alignment(gtkColumn, alignment.horizontal());
              gtk_cell_renderer_set_alignment(renderer, alignment.horizontal(), alignment.vertical());
              auto pangoAlignment = PANGO_ALIGN_CENTER;
              if(alignment.horizontal() < 0.333) pangoAlignment = PANGO_ALIGN_LEFT;
              if(alignment.horizontal() > 0.666) pangoAlignment = PANGO_ALIGN_RIGHT;
              g_object_set(G_OBJECT(renderer), "alignment", pangoAlignment, nullptr);
              auto font = pFont::create(cell->font(true));
              g_object_set(G_OBJECT(renderer), "font-desc", font, nullptr);
              pango_font_description_free(font);
              if(auto color = cell->foregroundColor(true)) {
                auto gdkColor = CreateColor(color);
                g_object_set(G_OBJECT(renderer), "foreground-gdk", &gdkColor, nullptr);
              } else {
                g_object_set(G_OBJECT(renderer), "foreground-set", false, nullptr);
              }
            }
            if(auto color = cell->backgroundColor(true)) {
              auto gdkColor = CreateColor(color);
              g_object_set(G_OBJECT(renderer), "cell-background-gdk", &gdkColor, nullptr);
            } else {
              g_object_set(G_OBJECT(renderer), "cell-background-set", false, nullptr);
            }
          }
        }
      }
    }
  }
}

auto pTableView::_doEdit(GtkCellRendererText* gtkCellRendererText, const char* path, const char* text) -> void {
  if(auto& header = state().header) {
    for(auto& column : header->state.columns) {
      if(auto delegate = column->self()) {
        if(gtkCellRendererText == GTK_CELL_RENDERER_TEXT(delegate->gtkCellText)) {
          auto row = toNatural(path);
          if(auto item = self().item(row)) {
            if(auto cell = item->cell(column->offset())) {
              if(string{text} != cell->state.text) {
                cell->setText(text);
                if(!locked()) self().doEdit(cell);
              }
              return;
            }
          }
        }
      }
    }
  }
}

auto pTableView::_doEvent(GdkEventButton* event) -> signed {
  GtkTreePath* path = nullptr;
  gtk_tree_view_get_path_at_pos(gtkTreeView, event->x, event->y, &path, nullptr, nullptr, nullptr);

  if(event->type == GDK_BUTTON_PRESS) {
    //when clicking in empty space below the last table view item; GTK+ does not deselect all items;
    //below code enables this functionality, to match behavior with all other UI toolkits (and because it's very convenient to have)
    if(path == nullptr && gtk_tree_selection_count_selected_rows(gtkTreeSelection) > 0) {
      for(auto& item : state().items) item->setSelected(false);
      self().doChange();
      return true;
    }
  }

  if(event->type == GDK_BUTTON_PRESS && event->button == 3) {
    //this check prevents the loss of selection on other items if the item under the mouse cursor is currently selected
    if(path && gtk_tree_selection_path_is_selected(gtkTreeSelection, path)) return true;
  }

  if(event->type == GDK_BUTTON_RELEASE && event->button == 3) {
    //handle action during right-click release; as button-press-event is sent prior to selection update
    //without this, the callback handler would see the previous selection state instead
    self().doContext();
    return false;
  }

  return false;
}

auto pTableView::_doHeaderActivate(GtkTreeViewColumn* gtkTreeViewColumn) -> void {
  if(auto& header = state().header) {
    for(auto& column : header->state.columns) {
      if(auto delegate = column->self()) {
        if(gtkTreeViewColumn == delegate->gtkColumn) {
          if(!locked()) self().doSort(column);
          return;
        }
      }
    }
  }
}

//GtkTreeView::cursor-changed and GtkTreeSelection::changed do not send signals for changes during rubber-banding selection
//so here we capture motion-notify-event, and if the selections have changed, invoke TableView::onChange
auto pTableView::_doMouseMove() -> signed {
  if(gtk_tree_view_is_rubber_banding_active(gtkTreeView)) {
    if(!locked()) _updateSelected();
  }
  return false;
}

auto pTableView::_doToggle(GtkCellRendererToggle* gtkCellRendererToggle, const char* path) -> void {
  if(auto& header = state().header) {
    for(auto& column : header->state.columns) {
      if(auto delegate = column->self()) {
        if(gtkCellRendererToggle == GTK_CELL_RENDERER_TOGGLE(delegate->gtkCellToggle)) {
          auto row = toNatural(path);
          if(auto item = self().item(row)) {
            if(auto cell = item->cell(column->offset())) {
              cell->setChecked(!cell->checked());
              if(!locked()) self().doToggle(cell);
              return;
            }
          }
        }
      }
    }
  }
}

//compare currently selected items to previously selected items
//if different, invoke the onChange callback unless locked, and cache current selection
//this prevents firing an onChange event when the actual selection has not changed
//this is particularly important for the motion-notify-event binding
auto pTableView::_updateSelected() -> void {
  vector<unsigned> selected;

  GList* list = gtk_tree_selection_get_selected_rows(gtkTreeSelection, &gtkTreeModel);
  GList* p = list;

  while(p) {
    GtkTreeIter iter;
    if(gtk_tree_model_get_iter(gtkTreeModel, &iter, (GtkTreePath*)p->data)) {
      char* pathname = gtk_tree_model_get_string_from_iter(gtkTreeModel, &iter);
      unsigned selection = toNatural(pathname);
      g_free(pathname);
      selected.append(selection);
    }
    p = p->next;
  }

  g_list_foreach(list, (GFunc)gtk_tree_path_free, nullptr);
  g_list_free(list);

  bool identical = selected.size() == currentSelection.size();
  if(identical) {
    for(auto n : range(selected)) {
      if(selected[n] != currentSelection[n]) {
        identical = false;
        break;
      }
    }
  }
  if(identical) return;

  currentSelection = selected;
  for(auto& item : state().items) item->state.selected = false;
  for(auto& position : currentSelection) {
    if(position >= self().itemCount()) continue;
    self().item(position)->state.selected = true;
  }

  if(!locked()) self().doChange();
}

auto pTableView::_width(unsigned column) -> unsigned {
  if(auto& header = state().header) {
    if(auto width = header->column(column).width()) return width;
    unsigned width = 1;
    if(!header->column(column).visible()) return width;
    if(header->visible()) width = max(width, _columnWidth(column));
    for(auto row : range(state().items)) {
      width = max(width, _cellWidth(row, column));
    }
    return width;
  }
  return 1;
}

}

#endif
