#ifndef VIEWGRID_H
#define VIEWGRID_H

#include <wx/wx.h>
#include "../../common/game_state/Ship.h"

class ViewGrid : public wxPanel{
public:
  ViewGrid(wxWindow *parent, wxPoint pos);

  wxStaticBitmap** getGrid() const {
    return _grid;
  }

  void placeShips(std::vector<Ship> ships);

private:
  wxStaticBitmap **_grid;
};

#endif // VIEWGRID_H