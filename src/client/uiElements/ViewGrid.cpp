#include "ViewGrid.h"

ViewGrid::ViewGrid(wxWindow *parent, wxPoint pos) : wxPanel(parent, wxID_ANY, pos, wxSize(400, 400)) {
  wxColor backgroundColor = wxColor(255, 255, 0);
  this->SetBackgroundColour(backgroundColor);

  size_t x = pos.x;
  size_t y = pos.y;

  constexpr int gridSize = 10;

  //wxStaticBitmap* grid[10][10];
  this->_grid = new wxStaticBitmap*[gridSize*gridSize];

  wxBitmap emptyTile = wxBitmap(wxImage("../assets/empty_tile.png"));
  for (int i = 0; i < gridSize; i++) {
    for (int j = 0; j < gridSize; j++) {
      _grid[i*gridSize+j] = new wxStaticBitmap(parent, wxID_ANY, emptyTile, wxPoint(x + i*40, y + j*40), wxSize(40, 40), 0);
    }
  }

  auto *gridLines = new wxBitmap(wxImage("../assets/grid_lines.png"));
  auto *gridImage = new wxStaticBitmap(parent, wxID_ANY, *gridLines, wxPoint(x, y), wxSize(400, 400));
}

void ViewGrid::placeShips(std::vector<Ship> ships) {
    for (auto ship : ships) {
        auto orientation = ship.getOrientation();
        int l = ship.getLength();
        Coordinate c = ship.getPosition();
        int x = c.x;
        int y = c.y;
        if (orientation == Ship::Orientation::Horizontal) {
            for (int i = 0; i < l; i++) {
                int idx = (x + i) * 10 + y;
                if (x + i < 10 && y < 10)
                _grid[idx]->SetBitmap(wxBitmap(wxImage("../assets/ship_tile.png")));
            }
        } else if (orientation == Ship::Orientation::Vertical) {
            for (int i = 0; i < l; i++) {
                int idx = x * 10 + y + i;
                if (x < 10 && y + i < 10)
                _grid[idx]->SetBitmap(wxBitmap(wxImage("../assets/ship_tile.png")));
            }
        }
    }
}