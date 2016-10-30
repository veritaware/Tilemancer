/*

  Tilemancer
  Copyright (C) 2016  Lucca Pedersoli Junior

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

// #define USE_WX

#include "tilemancer/tilemancer.h"

#ifdef USE_WX

#include "wx.h"
#include <wx/glcanvas.h>

// x11 header included by the gtk canvas defines Bool and that interfers with
// other code and is also ugly
#ifdef Bool
#undef Bool
#endif

class TilemancerApp: public wxApp
{
 public:
  virtual bool OnInit();
};

class TilemancerView;

class TilemancerFrame: public wxFrame
{
 public:
  TilemancerFrame();
 private:
  void OnFileNew(wxCommandEvent &event);
  void OnFileOpen(wxCommandEvent &event);
  void OnFileSave(wxCommandEvent &event);
  void OnFileExit(wxCommandEvent &event);

  void OnEditUndo(wxCommandEvent &event);
  void OnEditRedo(wxCommandEvent &event);

  void OnPaletteNewColor(wxCommandEvent &event);
  void OnPaletteDuplicateColor(wxCommandEvent &event);
  void OnPaletteDeleteColor(wxCommandEvent &event);
  void OnPaletteLoad(wxCommandEvent &event);
  void OnPaletteSave(wxCommandEvent &event);

  void OnAboutAbout(wxCommandEvent &event);
 private:
  TilemancerView* view;
 wxDECLARE_EVENT_TABLE();
};

class TilemancerView : public wxGLCanvas {
 public:
  TilemancerView(wxWindow* parent);
  ~TilemancerView();

  void OnPaint(wxPaintEvent& event);
  void OnSize(wxSizeEvent& event);
  void OnEraseBackground(wxEraseEvent& event);

  void OnLeftDown(wxMouseEvent& e);
  void OnLeftUp(wxMouseEvent& e);
  void OnMiddleDown(wxMouseEvent& e);
  void OnMiddleUp(wxMouseEvent& e);
  void OnRightDown(wxMouseEvent& e);
  void OnRightUp(wxMouseEvent& e);
  void OnMotion(wxMouseEvent& e);
  void OnWheel(wxMouseEvent& e);

  void Invalidate();

 private:
  wxGLContext rc;
 DECLARE_EVENT_TABLE()
};

enum
{
  ID_DUMMY_FIRST = 0,
  ID_PAL_NEW_COLOR,
  ID_PAL_DUPLICATE_COLOR,
  ID_PAL_DELETE_COLOR,
  ID_PAL_LOAD,
  ID_PAL_SAVE
};

////////////////////////////////////////////////////////////////////////////////


#if wxUSE_GLCANVAS
// #pragma message("wxGL is included")
#else
#error "No wxGL"
#endif

BEGIN_EVENT_TABLE(TilemancerView, wxGLCanvas)
        EVT_SIZE(TilemancerView::OnSize)
        EVT_PAINT(TilemancerView::OnPaint)
        EVT_ERASE_BACKGROUND(TilemancerView::OnEraseBackground)

        EVT_LEFT_DOWN(TilemancerView::OnLeftDown)
        EVT_LEFT_UP(TilemancerView::OnLeftUp)
        EVT_MIDDLE_DOWN(TilemancerView::OnMiddleDown)
        EVT_MIDDLE_UP(TilemancerView::OnMiddleUp)
        EVT_RIGHT_DOWN(TilemancerView::OnRightDown)
        EVT_RIGHT_UP(TilemancerView::OnRightUp)
        EVT_MOTION(TilemancerView::OnMotion)
        EVT_MOUSEWHEEL(TilemancerView::OnWheel)
END_EVENT_TABLE()

int wx_gl_args[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};

TilemancerView::TilemancerView(wxWindow* parent)
    : wxGLCanvas(parent, wxID_ANY, wx_gl_args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE),
      rc(this) {
  Invalidate();
}

TilemancerView::~TilemancerView() {}

void TilemancerView::OnSize(wxSizeEvent& event) {
  if (!IsShownOnScreen()) return;
  SetCurrent(rc);

  const auto size = event.GetSize();
  glViewport(0, 0, size.x, size.y);
}

void TilemancerView::OnLeftDown(wxMouseEvent& e) {
}

void TilemancerView::OnLeftUp(wxMouseEvent& e) {}

void TilemancerView::OnMiddleDown(wxMouseEvent& e) {}

void TilemancerView::OnMiddleUp(wxMouseEvent& e) {}

void TilemancerView::OnRightDown(wxMouseEvent& e) {}

void TilemancerView::OnRightUp(wxMouseEvent& e) {}

void TilemancerView::OnMotion(wxMouseEvent& e) {
  Invalidate();
}

void TilemancerView::Invalidate() { Refresh(false); }

void TilemancerView::OnWheel(wxMouseEvent& e) {
  Invalidate();
}

void TilemancerView::OnPaint(wxPaintEvent& WXUNUSED(event)) {
  wxPaintDC dc(this);
  SetCurrent(rc);
}

void TilemancerView::OnEraseBackground(wxEraseEvent& WXUNUSED(event)) {
  // Do nothing, to avoid flashing.
}


////////////////////////////////////////////////////////////////////////////////


wxBEGIN_EVENT_TABLE(TilemancerFrame, wxFrame)
        EVT_MENU(wxID_NEW,   TilemancerFrame::OnFileNew)
        EVT_MENU(wxID_OPEN,   TilemancerFrame::OnFileOpen)
        EVT_MENU(wxID_SAVE,   TilemancerFrame::OnFileSave)
        EVT_MENU(wxID_EXIT,  TilemancerFrame::OnFileExit)

        EVT_MENU(wxID_UNDO,  TilemancerFrame::OnEditUndo)
        EVT_MENU(wxID_REDO,  TilemancerFrame::OnEditRedo)

        EVT_MENU(ID_PAL_NEW_COLOR,   TilemancerFrame::OnPaletteNewColor)
        EVT_MENU(ID_PAL_DUPLICATE_COLOR,   TilemancerFrame::OnPaletteDuplicateColor)
        EVT_MENU(ID_PAL_DELETE_COLOR,   TilemancerFrame::OnPaletteDeleteColor)
        EVT_MENU(ID_PAL_LOAD,   TilemancerFrame::OnPaletteLoad)
        EVT_MENU(ID_PAL_SAVE,   TilemancerFrame::OnPaletteSave)

        EVT_MENU(wxID_ABOUT, TilemancerFrame::OnAboutAbout)
wxEND_EVENT_TABLE()

wxIMPLEMENT_APP(TilemancerApp);

bool TilemancerApp::OnInit()
{
  TilemancerFrame *frame = new TilemancerFrame();
  frame->Show( true );
  return true;
}

wxMenu* BuildMenuFile() {
  wxMenu *menuFile = new wxMenu;

  menuFile->Append(wxID_NEW);
  menuFile->Append(wxID_OPEN);
  menuFile->Append(wxID_SAVE);
  // menuFile->Append(wxID_SAVEAS);

  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);
  return menuFile;
}

wxMenu* BuildMenuEdit() {
  wxMenu *menuFile = new wxMenu;

  menuFile->Append(wxID_UNDO);
  menuFile->Append(wxID_REDO);
  return menuFile;
}

wxMenu* BuildMenuPalette() {
  wxMenu *menuFile = new wxMenu;

  menuFile->Append(ID_PAL_NEW_COLOR, "&New color", "");
  menuFile->Append(ID_PAL_DUPLICATE_COLOR, "D&uplicate color", "");
  menuFile->Append(ID_PAL_DELETE_COLOR, "&Delete color", "");
  menuFile->Append(ID_PAL_LOAD, "&Load", "");
  menuFile->Append(ID_PAL_SAVE, "&Save", "");

  return menuFile;
}

wxMenu* BuildHelpMenu() {
  wxMenu *menuHelp = new wxMenu;
  menuHelp->Append(wxID_ABOUT);
  return menuHelp;
}

wxMenuBar* BuildMenuBar() {
  wxMenuBar *menuBar = new wxMenuBar;
  menuBar->Append(BuildMenuFile(), "&File" );
  menuBar->Append(BuildMenuEdit(), "&Edit" );
  menuBar->Append(BuildMenuPalette(), "&Palette" );
  menuBar->Append(BuildHelpMenu(), "&Help" );
  return menuBar;
}

TilemancerFrame::TilemancerFrame()
    : wxFrame(NULL, wxID_ANY, "Tilemancer", wxPoint(50, 50), wxSize(450, 340))
{
  SetMenuBar( BuildMenuBar() );
  CreateStatusBar();
  SetStatusText( "" );

  view = new TilemancerView(this);

  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  sizer->Add(view, wxSizerFlags(1).Expand());
  this->SetSizer(sizer);
}

void TilemancerFrame::OnFileNew(wxCommandEvent &event) {}
void TilemancerFrame::OnFileOpen(wxCommandEvent &event) {}
void TilemancerFrame::OnFileSave(wxCommandEvent &event) {}
void TilemancerFrame::OnFileExit(wxCommandEvent &event)
{
  Close( true );
}

void TilemancerFrame::OnEditUndo(wxCommandEvent &event) {}
void TilemancerFrame::OnEditRedo(wxCommandEvent &event) {}

void TilemancerFrame::OnPaletteNewColor(wxCommandEvent &event) {}
void TilemancerFrame::OnPaletteDuplicateColor(wxCommandEvent &event) {}
void TilemancerFrame::OnPaletteDeleteColor(wxCommandEvent &event) {}
void TilemancerFrame::OnPaletteLoad(wxCommandEvent &event) {}
void TilemancerFrame::OnPaletteSave(wxCommandEvent &event) {}

void TilemancerFrame::OnAboutAbout(wxCommandEvent &event)
{
  wxMessageBox( "This is a wxWidgets' Hello world sample",
                "About Hello World", wxOK | wxICON_INFORMATION );
}


#else

int main(int argc, char* args[]) { return tilemancer_main(); }

#endif
