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

#include "Polygon_repair.h"

void Polygon_repair::ogr_to_multi_polygon(OGRGeometry *in_geometry, Multi_polygon<Point> &out_geometry) {
  out_geometry.clear();
  switch (in_geometry->getGeometryType()) {
      
    case wkbLineString: {
      OGRLinearRing *in_geometry_as_linear_ring = static_cast<OGRLinearRing *>(in_geometry);
      Polygon<Point> &new_polygon = out_geometry.add_empty_polygon();
      Linear_ring<Point> &new_ring = new_polygon.outer_ring();
      for (int current_vertex = 0; current_vertex < in_geometry_as_linear_ring->getNumPoints(); ++current_vertex) {
        new_ring.add_vertex(Point(in_geometry_as_linear_ring->getX(current_vertex), in_geometry_as_linear_ring->getY(current_vertex)));
      } break;
    }
      
    case wkbPolygon: {
      OGRPolygon *in_geometry_as_polygon = static_cast<OGRPolygon *>(in_geometry);
      Polygon<Point> &new_polygon = out_geometry.add_empty_polygon();
      Linear_ring<Point> &new_outer_ring = new_polygon.outer_ring();
      for (int current_vertex = 0; current_vertex < in_geometry_as_polygon->getExteriorRing()->getNumPoints(); ++current_vertex) {
        new_outer_ring.add_vertex(Point(in_geometry_as_polygon->getExteriorRing()->getX(current_vertex), in_geometry_as_polygon->getExteriorRing()->getY(current_vertex)));
      } for (int current_inner_ring = 0; current_inner_ring < in_geometry_as_polygon->getNumInteriorRings(); ++current_inner_ring) {
        Linear_ring<Point> &new_inner_ring = new_polygon.add_empty_inner_ring();
        for (int current_vertex = 0; current_vertex < in_geometry_as_polygon->getInteriorRing(current_inner_ring)->getNumPoints(); ++current_vertex) {
          new_inner_ring.add_vertex(Point(in_geometry_as_polygon->getInteriorRing(current_inner_ring)->getX(current_vertex), in_geometry_as_polygon->getInteriorRing(current_inner_ring)->getY(current_vertex)));
        }
      } break;
    }
      
    case wkbMultiPolygon: {
      OGRMultiPolygon *in_geometry_as_multi_polygon = static_cast<OGRMultiPolygon *>(in_geometry);
      for (int current_polygon = 0; current_polygon < in_geometry_as_multi_polygon->getNumGeometries(); ++current_polygon) {
        OGRPolygon *polygon_from_in_geometry = static_cast<OGRPolygon *>(in_geometry_as_multi_polygon->getGeometryRef(current_polygon));
        Polygon<Point> &new_polygon = out_geometry.add_empty_polygon();
        Linear_ring<Point> &new_outer_ring = new_polygon.outer_ring();
        for (int current_vertex = 0; current_vertex < polygon_from_in_geometry->getExteriorRing()->getNumPoints(); ++current_vertex) {
          new_outer_ring.add_vertex(Point(polygon_from_in_geometry->getExteriorRing()->getX(current_vertex), polygon_from_in_geometry->getExteriorRing()->getY(current_vertex)));
        } for (int current_inner_ring = 0; current_inner_ring < polygon_from_in_geometry->getNumInteriorRings(); ++current_inner_ring) {
          Linear_ring<Point> &new_inner_ring = new_polygon.add_empty_inner_ring();
          for (int current_vertex = 0; current_vertex < polygon_from_in_geometry->getInteriorRing(current_inner_ring)->getNumPoints(); ++current_vertex) {
            new_inner_ring.add_vertex(Point(polygon_from_in_geometry->getInteriorRing(current_inner_ring)->getX(current_vertex), polygon_from_in_geometry->getInteriorRing(current_inner_ring)->getY(current_vertex)));
          }
        }
      } break;
    }
      
    default:
      std::cerr << "PolygonRepair::ogrToMultiPolygon: Cannot understand input." << std::endl;
      break;
  }
}

OGRGeometry *Polygon_repair::multi_polygon_to_ogr(Multi_polygon<Point> &in_geometry) {
  OGRGeometry *out_geometry;
  
  if (in_geometry.number_of_polygons() == 0) {
    out_geometry = new OGRPolygon();
  }
  
  else if (in_geometry.number_of_polygons() == 1) {
    OGRPolygon *new_polygon = new OGRPolygon();
    Linear_ring<Point> &outer_ring = in_geometry.first_polygon().outer_ring();
    OGRLinearRing *new_ring = new OGRLinearRing();
    for (Linear_ring<Point>::vertex_iterator current_vertex = outer_ring.vertices_begin(); current_vertex != outer_ring.vertices_end(); ++current_vertex) {
      new_ring->addPoint(CGAL::to_double(current_vertex->x()), CGAL::to_double(current_vertex->y()));
    } new_polygon->addRingDirectly(new_ring);
    for (Polygon<Point>::inner_ring_iterator current_ring = in_geometry.first_polygon().inner_rings_begin(); current_ring != in_geometry.first_polygon().inner_rings_end(); ++current_ring) {
      new_ring = new OGRLinearRing();
      for (Linear_ring<Point>::vertex_iterator current_vertex = current_ring->vertices_begin(); current_vertex != current_ring->vertices_end(); ++current_vertex) {
        new_ring->addPoint(CGAL::to_double(current_vertex->x()), CGAL::to_double(current_vertex->y()));
      } new_polygon->addRingDirectly(new_ring);
    } out_geometry = new_polygon;
  }
  
  else {
    OGRMultiPolygon *new_multi_polygon = new OGRMultiPolygon();
    for (Multi_polygon<Point>::polygon_iterator current_polygon = in_geometry.polygons_begin(); current_polygon != in_geometry.polygons_end(); ++current_polygon) {
      OGRPolygon *new_polygon = new OGRPolygon();
      Linear_ring<Point> &outer_ring = current_polygon->outer_ring();
      OGRLinearRing *new_ring = new OGRLinearRing();
      for (Linear_ring<Point>::vertex_iterator current_vertex = outer_ring.vertices_begin(); current_vertex != outer_ring.vertices_end(); ++current_vertex) {
        new_ring->addPoint(CGAL::to_double(current_vertex->x()), CGAL::to_double(current_vertex->y()));
      } new_polygon->addRingDirectly(new_ring);
      for (Polygon<Point>::inner_ring_iterator current_ring = current_polygon->inner_rings_begin(); current_ring != current_polygon->inner_rings_end(); ++current_ring) {
        new_ring = new OGRLinearRing();
        for (Linear_ring<Point>::vertex_iterator current_vertex = current_ring->vertices_begin(); current_vertex != current_ring->vertices_end(); ++current_vertex) {
          new_ring->addPoint(CGAL::to_double(current_vertex->x()), CGAL::to_double(current_vertex->y()));
        } new_polygon->addRingDirectly(new_ring);
      } new_multi_polygon->addGeometryDirectly(new_polygon);
    } out_geometry = new_multi_polygon;
  }
  
  return out_geometry;
}

void Polygon_repair::repair_odd_even(Multi_polygon<Point> &in_geometry, Multi_polygon<Point> &out_geometry, bool time_results) {
  triangulation.clear();
  time_t this_time, total_time;
  this_time = time(NULL);
  insert_constraints(triangulation, in_geometry);
  total_time = time(NULL)-this_time;
  if (time_results) std::cout << "Triangulation: " << total_time/60 << " minutes " << total_time%60 << " seconds." << std::endl;
  this_time = time(NULL);
  tag_odd_even(triangulation);
  total_time = time(NULL)-this_time;
  if (time_results) std::cout << "Tagging: " << total_time/60 << " minutes " << total_time%60 << " seconds." << std::endl;
  this_time = time(NULL);
  reconstruct(triangulation, out_geometry);
  total_time = time(NULL)-this_time;
  if (time_results) std::cout << "Reconstruction: " << total_time/60 << " minutes " << total_time%60 << " seconds." << std::endl;
}

void Polygon_repair::insert_constraints(Triangulation &triangulation, Multi_polygon<Point> &in_geometry, bool remove_overlapping_constraints) {
  Triangulation::Vertex_handle va, vb;
  Triangulation::Face_handle face_of_edge;
  int index_of_edge;
  
  for (Multi_polygon<Point>::polygon_iterator current_polygon = in_geometry.polygons_begin(); current_polygon != in_geometry.polygons_end(); ++current_polygon) {
    
    // Outer
    Linear_ring<Point>::vertex_iterator current_vertex = current_polygon->outer_ring().vertices_begin();
    vb = triangulation.insert(*current_vertex);
    ++current_vertex;
    while (current_vertex != current_polygon->outer_ring().vertices_end()) {
      va = vb;
      vb = triangulation.insert(*current_vertex, triangulation.incident_faces(va));
      if (va == vb) {
        std::cout << "Duplicate vertex: skipped." << std::endl;
        ++current_vertex;
        continue;
      } if (remove_overlapping_constraints && triangulation.is_edge(va, vb, face_of_edge, index_of_edge)) {
        if (triangulation.is_constrained(std::pair<Triangulation::Face_handle, int>(face_of_edge, index_of_edge))) {
          std::cout << "Inserting constraint<" << va->point() << ", " << vb->point() << ">" << std::endl;
          triangulation.insert_constraint(va, vb);
          triangulation.remove_constraint(va, vb);
        } else {
          std::cout << "Inserting constraint<" << va->point() << ", " << vb->point() << ">" << std::endl;
          triangulation.insert_constraint(va, vb);
        }
      } else {
        std::cout << "Inserting constraint<" << va->point() << ", " << vb->point() << ">" << std::endl;
        triangulation.insert_constraint(va, vb);
      } ++current_vertex;
    }
    
    // Inner
    for (Polygon<Point>::inner_ring_iterator current_ring = current_polygon->inner_rings_begin(); current_ring != current_polygon->inner_rings_end(); ++current_ring) {
      current_vertex = current_ring->vertices_begin();
      vb = triangulation.insert(*current_vertex);
      ++current_vertex;
      while (current_vertex != current_ring->vertices_end()) {
        va = vb;
        vb = triangulation.insert(*current_vertex, triangulation.incident_faces(va));
        if (va == vb) {
          std::cout << "Duplicate vertex: skipped." << std::endl;
          ++current_vertex;
          continue;
        } if (remove_overlapping_constraints && triangulation.is_edge(va, vb, face_of_edge, index_of_edge)) {
          if (triangulation.is_constrained(std::pair<Triangulation::Face_handle, int>(face_of_edge, index_of_edge))) {
            std::cout << "Inserting constraint<" << va->point() << ", " << vb->point() << ">" << std::endl;
            triangulation.insert_constraint(va, vb);
            triangulation.remove_constraint(va, vb);
          } else {
            std::cout << "Inserting constraint<" << va->point() << ", " << vb->point() << ">" << std::endl;
            triangulation.insert_constraint(va, vb);
          }
        } else {
          std::cout << "Inserting constraint<" << va->point() << ", " << vb->point() << ">" << std::endl;
          triangulation.insert_constraint(va, vb);
        } ++current_vertex;
      }
    }
  }
}

void Polygon_repair::tag_odd_even(Triangulation &triangulation) {
	
  // Clean tags
  for (Triangulation::Face_handle current_face = triangulation.all_faces_begin(); current_face != triangulation.all_faces_end(); ++current_face)
    current_face->info().clear();
  
  // Initialise tagging
  std::stack<Triangulation::Face_handle> interior_stack, exterior_stack;
  exterior_stack.push(triangulation.infinite_face());
  std::stack<Triangulation::Face_handle> *current_stack = &exterior_stack;
  std::stack<Triangulation::Face_handle> *dual_stack = &interior_stack;
  bool labelling_interior = false;
  
  
  // Until we finish
  while (!interior_stack.empty() || !exterior_stack.empty()) {
    
    // Give preference to whatever we're already doing
    while (!current_stack->empty()) {
      Triangulation::Face_handle current_face = current_stack->top();
			current_stack->pop();
      if (current_face->info().been_tagged()) continue;
			current_face->info().is_in_interior(labelling_interior);
      for (int current_edge = 0; current_edge < 3; ++current_edge) {
        if (!current_face->neighbor(current_edge)->info().been_tagged()) {
          if (current_face->is_constrained(current_edge))
            dual_stack->push(current_face->neighbor(current_edge));
          else
            current_stack->push(current_face->neighbor(current_edge));
        }
      }
    }
    
    // Flip
    if (!labelling_interior) {
      current_stack = &interior_stack;
      dual_stack = &exterior_stack;
    } else {
      current_stack = &exterior_stack;
      dual_stack = &interior_stack;
    } labelling_interior = !labelling_interior;
	}
}

void Polygon_repair::reconstruct(Triangulation &triangulation, Multi_polygon<Point> &out_geometry) {
  if (triangulation.number_of_faces() < 1) {
    return;
  }
  
  // Reconstruct
  for (Triangulation::Finite_faces_iterator seeding_face = triangulation.finite_faces_begin(); seeding_face != triangulation.finite_faces_end(); ++seeding_face) {
    
    if (!seeding_face->info().is_in_interior() || seeding_face->info().been_reconstructed()) continue;
    seeding_face->info().been_reconstructed(true);
    if (!seeding_face->info().been_reconstructed()) {
      std::cout << "ERROR! Should be marked as reconstructed!!!" << std::endl;
    }
    
    // Get boundary
    Linear_ring<Triangulation::Vertex_handle> vertices;
    if (seeding_face->neighbor(2)->info().is_in_interior() && !seeding_face->neighbor(2)->info().been_reconstructed()) {
      seeding_face->neighbor(2)->info().been_reconstructed(true);
      Linear_ring<Triangulation::Vertex_handle> l2;
      get_boundary(seeding_face->neighbor(2), seeding_face->neighbor(2)->index(seeding_face), l2);
      vertices.splice_vertices(vertices.vertices_end(), l2);
    } vertices.add_vertex(seeding_face->vertex(0));
    if (seeding_face->neighbor(1)->info().is_in_interior() && !seeding_face->neighbor(1)->info().been_reconstructed()) {
      seeding_face->neighbor(1)->info().been_reconstructed(true);
      Linear_ring<Triangulation::Vertex_handle> l1;
      get_boundary(seeding_face->neighbor(1), seeding_face->neighbor(1)->index(seeding_face), l1);
      vertices.splice_vertices(vertices.vertices_end(), l1);
    } vertices.add_vertex(seeding_face->vertex(2));
    if (seeding_face->neighbor(0)->info().is_in_interior() && !seeding_face->neighbor(0)->info().been_reconstructed()) {
      seeding_face->neighbor(0)->info().been_reconstructed(true);
      Linear_ring<Triangulation::Vertex_handle> l0;
      get_boundary(seeding_face->neighbor(0), seeding_face->neighbor(0)->index(seeding_face), l0);
      vertices.splice_vertices(vertices.vertices_end(), l0);
    } vertices.add_vertex(seeding_face->vertex(1));
    
    // Find cutting vertices
    std::set<Triangulation::Vertex_handle> visited_vertices;
    std::set<Triangulation::Vertex_handle> repeated_vertices;
    for (std::list<Triangulation::Vertex_handle>::iterator current_vertex = vertices.vertices_begin(); current_vertex != vertices.vertices_end(); ++current_vertex) {
      if (!visited_vertices.insert(*current_vertex).second) repeated_vertices.insert(*current_vertex);
    } visited_vertices.clear();
    
    // Cut and join rings in the correct order
    std::list<Linear_ring<Triangulation::Vertex_handle> > rings;
    std::list<Linear_ring<Triangulation::Vertex_handle> > chains_stack;
    std::set<Triangulation::Vertex_handle> vertices_where_chains_begin;
    rings.push_back(Linear_ring<Triangulation::Vertex_handle>());
    std::list<Linear_ring<Triangulation::Vertex_handle> >::reverse_iterator new_chain = rings.rbegin();
    for (Linear_ring<Triangulation::Vertex_handle>::vertex_iterator current_vertex = vertices.vertices_begin(); current_vertex != vertices.vertices_end(); ++current_vertex) {
      
      // New chain
      if (repeated_vertices.count(*current_vertex) > 0) {
        // Closed by itself
        if (new_chain->first_vertex() == *current_vertex) {
          // Degenerate (insufficient vertices to be valid)
          if (new_chain->number_of_vertices() < 3) {
            std::cerr << "Reconstruction: polygon with less than 3 vertices. Skipped." << std::endl;
            new_chain->clear();
          }
          else {
            Linear_ring<Triangulation::Vertex_handle>::vertex_iterator second_element = new_chain->vertices_begin();
            ++second_element;
            // Degenerate (zero area)
            if (new_chain->last_vertex() == *second_element) {
              // std::cerr << "Reconstruction: bridge?. Skipped." << std::endl;
              new_chain->clear();
            }
            // Valid
            else {
              rings.push_back(Linear_ring<Triangulation::Vertex_handle>());
              new_chain = rings.rbegin();
            }
          }
        }
        // Open by itself
        else {
          // Closed with others in stack
          if (vertices_where_chains_begin.count(*current_vertex)) {
            while (new_chain->first_vertex() != *current_vertex) {
              new_chain->splice_vertices(new_chain->vertices_begin(), chains_stack.back());
              chains_stack.pop_back();
            } vertices_where_chains_begin.erase(*current_vertex);
            // Degenerate (insufficient vertices to be valid)
            if (new_chain->number_of_vertices() < 3) {
              std::cerr << "Reconstruction: polygon with less than 3 vertices. Skipped." << std::endl;
              new_chain->clear();
            }
            else {
              std::list<Triangulation::Vertex_handle>::iterator second_element = new_chain->vertices_begin();
              ++second_element;
              // Degenerate (zero area)
              if (new_chain->last_vertex() == *second_element) {
                // std::cerr << "Reconstruction: bridge?. Skipped." << std::endl;
                new_chain->clear();
              }
              // Valid
              else {
                rings.push_back(Linear_ring<Triangulation::Vertex_handle>());
                new_chain = rings.rbegin();
              }
            }
          }
          // Open
          else {
            // Not first chain
            if (repeated_vertices.count(new_chain->first_vertex()) > 0) vertices_where_chains_begin.insert(new_chain->first_vertex());
            chains_stack.push_back(Linear_ring<Triangulation::Vertex_handle>());
            chains_stack.back().splice_vertices(chains_stack.back().vertices_begin(), rings.back());
          }
        }
      } new_chain->add_vertex(*current_vertex);
    }
    // Final ring
    while (chains_stack.size() > 0) {
      new_chain->splice_vertices(new_chain->vertices_begin(), chains_stack.back());
      chains_stack.pop_back();
    }
    // Degenerate (insufficient vertices to be valid)
    if (new_chain->number_of_vertices() < 3) {
      std::cerr << "Reconstruction: polygon with less than 3 vertices. Skipped." << std::endl;
      new_chain->clear();
    }
    else {
      std::list<Triangulation::Vertex_handle>::iterator second_element = new_chain->vertices_begin();
      ++second_element;
      // Degenerate (zero area)
      if (new_chain->last_vertex() == *second_element) {
        // std::cerr << "Reconstruction: bridge?. Skipped." << std::endl;
        new_chain->clear();
      }
      // Valid
      else {
        rings.push_back(Linear_ring<Triangulation::Vertex_handle>());
        new_chain = rings.rbegin();
      }
    } // Remove last ring if too small
    if (new_chain->number_of_vertices() < 3) rings.pop_back();
    
    // Make rings
    if (rings.size() == 0) continue;
    Polygon<Point> &new_polygon = out_geometry.add_empty_polygon();
    for (std::list<Linear_ring<Triangulation::Vertex_handle> >::iterator current_ring = rings.begin(); current_ring != rings.end(); ++current_ring) {
      if (!current_ring->is_clockwise()) {
        for (Linear_ring<Triangulation::Vertex_handle>::vertex_iterator current_vertex = current_ring->vertices_begin(); current_vertex != current_ring->vertices_end(); ++current_vertex) {
          new_polygon.outer_ring().add_vertex((*current_vertex)->point());
        } break;
      }
    } for (std::list<Linear_ring<Triangulation::Vertex_handle> >::iterator current_ring = rings.begin(); current_ring != rings.end(); ++current_ring) {
      if (current_ring->is_clockwise()) {
        Linear_ring<Point> &new_ring = new_polygon.add_empty_inner_ring();
        for (Linear_ring<Triangulation::Vertex_handle>::vertex_iterator current_vertex = current_ring->vertices_begin(); current_vertex != current_ring->vertices_end(); ++current_vertex) {
          new_ring.add_vertex((*current_vertex)->point());
        }
      }
    }
  }
}

void Polygon_repair::get_boundary(Triangulation::Face_handle face, int edge, Linear_ring<Triangulation::Vertex_handle> &out_vertices) {
  // Check clockwise edge
  if (face->neighbor(face->cw(edge))->info().is_in_interior() && !face->neighbor(face->cw(edge))->info().been_reconstructed()) {
		face->neighbor(face->cw(edge))->info().been_reconstructed(true);
		Linear_ring<Triangulation::Vertex_handle> v1;
    get_boundary(face->neighbor(face->cw(edge)), face->neighbor(face->cw(edge))->index(face), v1);
		out_vertices.splice_vertices(out_vertices.vertices_end(), v1);
	}
	
	// Add central vertex
  out_vertices.add_vertex(face->vertex(edge));
	
	// Check counterclockwise edge
  if (face->neighbor(face->ccw(edge))->info().is_in_interior() && !face->neighbor(face->ccw(edge))->info().been_reconstructed()) {
		face->neighbor(face->ccw(edge))->info().been_reconstructed(true);
		Linear_ring<Triangulation::Vertex_handle> v2;
    get_boundary(face->neighbor(face->ccw(edge)), face->neighbor(face->ccw(edge))->index(face), v2);
		out_vertices.splice_vertices(out_vertices.vertices_end(), v2);
	}
}