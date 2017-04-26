#include "CgalKernel.h"

bool IfcGeom::CgalKernel::convert_wire_to_face(const cgal_wire_t& wire, cgal_face_t& face) {
  face.outer = wire;
  return true;
}

void IfcGeom::CgalKernel::remove_duplicate_points_from_loop(cgal_wire_t& polygon) {
  std::set<cgal_point_t> points;
  for (int i = 0; i < polygon.size(); ++i) {
    if (points.count(polygon[i])) {
      polygon.erase(polygon.begin()+i);
      --i;
    } else points.insert(polygon[i]);
  }
}

CGAL::Polyhedron_3<Kernel> IfcGeom::CgalKernel::create_polyhedron(std::list<cgal_face_t> &face_list) {
  
  // Naive creation
  cgal_shape_t polyhedron;
  PolyhedronBuilder builder(&face_list);
  polyhedron.delegate(builder);
  
  // Stitch edges
  //  std::cout << "Before: " << polyhedron.size_of_vertices() << " vertices and " << polyhedron.size_of_facets() << " facets" << std::endl;
  cgal_shape_t original_polyhedron(polyhedron);
  CGAL::Polygon_mesh_processing::stitch_borders(polyhedron);
  if (!polyhedron.is_valid()) {
    Logger::Message(Logger::LOG_ERROR, "create_polyhedron: Polyhedron not valid!");
    std::ofstream fresult;
    fresult.open("/Users/ken/Desktop/invalid.off");
    fresult << original_polyhedron << std::endl;
    fresult.close();
    throw 10;
  } if (polyhedron.is_closed()) {
    if (!CGAL::Polygon_mesh_processing::is_outward_oriented(polyhedron)) {
      CGAL::Polygon_mesh_processing::reverse_face_orientations(polyhedron);
    }
  }
  
  //  std::cout << "After: " << polyhedron.size_of_vertices() << " vertices and " << polyhedron.size_of_facets() << " facets" << std::endl;
  
  return polyhedron;
}

CGAL::Polyhedron_3<Kernel> IfcGeom::CgalKernel::create_polyhedron(CGAL::Nef_polyhedron_3<Kernel> &nef_polyhedron) {
  if (nef_polyhedron.is_simple()) {
//    std::cout << "Simple Nef polyhedron with " << nef_polyhedron.number_of_volumes() << " volumes." << std::endl;
    try {
      CGAL::Polyhedron_3<Kernel> polyhedron;
      nef_polyhedron.convert_to_polyhedron(polyhedron);
      return polyhedron;
    } catch (...) {
      Logger::Message(Logger::LOG_ERROR, "Conversion from Nef to polyhedron failed!");
      throw 10;
    }
  } else {
    
    // Attempt using an iteration over all halffacets
//    std::map<CGAL::Nef_polyhedron_3<Kernel>::Vertex_const_handle, unsigned long> points_map;
//    std::list<std::vector<unsigned long>> faces_list;
//    std::set<CGAL::Nef_polyhedron_3<Kernel>::Halffacet_const_handle> visited_halffacets;
//    for (CGAL::Nef_polyhedron_3<Kernel>::Halffacet_const_iterator current_halffacet = nef_polyhedron.halffacets_begin();
//         current_halffacet != nef_polyhedron.halffacets_end();
//         ++current_halffacet) {
//      if (visited_halffacets.count(current_halffacet->twin())) continue;
//      else visited_halffacets.insert(current_halffacet);
//      if (current_halffacet->incident_volume()->mark() == current_halffacet->twin()->incident_volume()->mark()) continue;
////      std::cout << "Hf" << std::endl;
//      unsigned int cycle_number = 1;
//      for (CGAL::Nef_polyhedron_3<Kernel>::Halffacet_cycle_const_iterator current_halffacet_cycle = current_halffacet->facet_cycles_begin();
//           current_halffacet_cycle != current_halffacet->facet_cycles_end();
//           ++current_halffacet_cycle) {
//        if (current_halffacet_cycle.is_shalfloop()) {
//          std::cout << "Hf -> Hfc (SHl)" << std::endl;
//          continue;
//        } if (cycle_number > 1) {
//          std::cout << "MORE THAN ONE SFACET CYCLE!!!" << std::endl;
//        } // std::cout << "Hf -> Hfc (SHe)" << std::endl;
//        faces_list.push_back(std::vector<unsigned long>());
//        CGAL::Nef_polyhedron_3<Kernel>::SHalfedge_const_handle first_shalfedge(current_halffacet_cycle);
//        CGAL::Nef_polyhedron_3<Kernel>::SHalfedge_const_handle current_shalfedge = first_shalfedge;
//        do {
////          std::cout << "Hf -> Hfc (SHe) -> Sv" << std::endl;
//          if (points_map.count(current_shalfedge->source()->center_vertex()) == 0) points_map[current_shalfedge->source()->center_vertex()] = points_map.size()+1;
//          faces_list.back().push_back(points_map[current_shalfedge->source()->center_vertex()]);
//          current_shalfedge = current_shalfedge->next();
//        } while (current_shalfedge != first_shalfedge);
//        ++cycle_number;
//      }
//    }
    
    // Attempt using an iteration over all volumes, then shells, ...
//    for (CGAL::Nef_polyhedron_3<Kernel>::Volume_const_iterator current_volume = nef_polyhedron.volumes_begin();
//         current_volume != nef_polyhedron.volumes_end();
//         ++current_volume) {
//      std::cout << "V" << std::endl;
//      for (CGAL::Nef_polyhedron_3<Kernel>::Shell_entry_const_iterator current_shell = current_volume->shells_begin();
//           current_shell != current_volume->shells_end();
//           ++current_shell) {
//        std::cout << "V -> S" << std::endl;
//        CGAL::Nef_polyhedron_3<Kernel>::SFace_const_handle current_sface(current_shell);
//      }
//    }
    
    // Debug output
//    std::cout << "Nef with " << nef_polyhedron.number_of_vertices() << " vertices, " << nef_polyhedron.number_of_edges() << " edges, " << nef_polyhedron.number_of_facets() << " facets, " << nef_polyhedron.number_of_volumes() << std::endl;
//    std::cout << "Is empty? " << nef_polyhedron.is_empty() << std::endl;
//    std::ofstream ftest;
//    ftest.open("/Users/ken/Desktop/non_simple_nef.obj");
//    for (auto &point: points_map) ftest << "v " << point.first->point() << std::endl;
//    for (auto &face: faces_list) {
//      ftest << "f";
//      for (auto &vertex: face) ftest << " " << vertex;
//      ftest << std::endl;
//    } ftest.close();
    
    std::string error_message("Nef polyhedron with ");
    error_message += std::to_string(nef_polyhedron.number_of_volumes());
    error_message += " volumes not simple: cannot create polyhedron!";
    Logger::Message(Logger::LOG_ERROR, error_message.c_str());
    throw 10;
  }
}

CGAL::Nef_polyhedron_3<Kernel> IfcGeom::CgalKernel::create_nef_polyhedron(std::list<cgal_face_t> &face_list) {
  CGAL::Polyhedron_3<Kernel> polyhedron = create_polyhedron(face_list);
  CGAL::Polygon_mesh_processing::triangulate_faces(polyhedron);
  CGAL::Nef_polyhedron_3<Kernel> nef_polyhedron;
  try {
    nef_polyhedron = CGAL::Nef_polyhedron_3<Kernel>(polyhedron);
  } catch (...) {
    Logger::Message(Logger::LOG_ERROR, "Conversion to Nef polyhedron failed!");
    throw 10;
  } return nef_polyhedron;
}

CGAL::Nef_polyhedron_3<Kernel> IfcGeom::CgalKernel::create_nef_polyhedron(CGAL::Polyhedron_3<Kernel> &polyhedron) {
  if (polyhedron.is_valid()) {
    CGAL::Polygon_mesh_processing::triangulate_faces(polyhedron);
    CGAL::Nef_polyhedron_3<Kernel> nef_polyhedron;
    try {
      nef_polyhedron = CGAL::Nef_polyhedron_3<Kernel>(polyhedron);
    } catch (...) {
      Logger::Message(Logger::LOG_ERROR, "Conversion to Nef polyhedron failed!");
      throw 10;
    } return nef_polyhedron;
  } else {
    Logger::Message(Logger::LOG_ERROR, "Polyhedron not valid: cannot create Nef polyhedron!");
    return CGAL::Nef_polyhedron_3<Kernel>();
  }
}
