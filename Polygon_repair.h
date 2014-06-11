/*
 Copyright (c) 2009-2014,
 Ken Arroyo Ohori    g.a.k.arroyoohori@tudelft.nl
 Hugo Ledoux         h.ledoux@tudelft.nl
 Martijn Meijers     b.m.meijers@tudelft.nl
 All rights reserved.
 
 This file is part of prepair: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 Licensees holding a valid commercial license may use this file in
 accordance with the commercial license agreement provided with
 the software.
 
 This file is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef POLYGONREPAIR_H

#include "definitions.h"

class Polygon_repair {
public:
  static void ogr_to_multi_polygon(OGRGeometry *in_geometry, Multi_polygon<Point> &out_geometry);
  static OGRGeometry *multi_polygon_to_ogr(Multi_polygon<Point> &in_geometry);
  
  void repair_odd_even(Multi_polygon<Point> &in_geometry, Multi_polygon<Point> &out_geometry, bool time_results = false);
  
private:
  Triangulation triangulation;
  
  void insert_constraints(Triangulation &triangulation, Multi_polygon<Point> &in_geometry, bool remove_overlapping_constraints = true);
  void tag_odd_even(Triangulation &triangulation);
  void reconstruct(Triangulation &triangulation, Multi_polygon<Point> &out_geometry);
  void get_boundary(Triangulation::Face_handle face, int edge, Linear_ring<Triangulation::Vertex_handle> &out_vertices);
};

#endif