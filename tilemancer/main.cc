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

#define USE_WX

#include "tilemancer/tilemancer.h"

#ifdef USE_WX

#include "wx.h"

class TilemancerApp: public wxApp
{
 public:
  virtual bool OnInit();
};

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
 wxDECLARE_EVENT_TABLE();
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
