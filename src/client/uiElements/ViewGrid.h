#ifndef VIEWGRID_H
#define VIEWGRID_H

#include <wx/wx.h>
#include "../../common/game_state/Ship.h"

class ViewGrid : public wxPanel{
public:
  enum gridtype {own, opp};
  
  ViewGrid(wxWindow *parent, gridtype type);

  wxPoint GetPosition() const {
    return _pos;
  }
  wxStaticBitmap** getGrid() const {
    return _grid;
  }

  void showShips(const std::vector<Ship>& ships);
  void showShots(const int shots[10][10]);

  void onMouseClick(wxMouseEvent &event);

private:
  wxPoint _pos;
  wxStaticBitmap **_grid;
  gridtype _type;
};

#endif // VIEWGRID_H