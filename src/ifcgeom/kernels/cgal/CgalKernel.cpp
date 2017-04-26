/********************************************************************************
*                                                                              *
* This file is part of IfcOpenShell.                                           *
*                                                                              *
* IfcOpenShell is free software: you can redistribute it and/or modify         *
* it under the terms of the Lesser GNU General Public License as published by  *
* the Free Software Foundation, either version 3.0 of the License, or          *
* (at your option) any later version.                                          *
*                                                                              *
* IfcOpenShell is distributed in the hope that it will be useful,              *
* but WITHOUT ANY WARRANTY; without even the implied warranty of               *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                 *
* Lesser GNU General Public License for more details.                          *
*                                                                              *
* You should have received a copy of the Lesser GNU General Public License     *
* along with this program. If not, see <http://www.gnu.org/licenses/>.         *
*                                                                              *
********************************************************************************/

#include "../../../ifcgeom/IfcGeomShapeType.h"
#include "../../../ifcgeom/IfcGeom.h"

#include "CgalKernel.h"
#include "CgalConversionResult.h"

bool IfcGeom::CgalKernel::is_identity_transform(IfcUtil::IfcBaseClass* l) {
	Logger::Message(Logger::LOG_ERROR, "Not implemented is_identity_transform()");
	return false;
	/*
	// OpenCascade kernel code below

	IfcSchema::IfcAxis2Placement2D* ax2d;
	IfcSchema::IfcAxis2Placement3D* ax3d;

	IfcSchema::IfcCartesianTransformationOperator2D* op2d;
	IfcSchema::IfcCartesianTransformationOperator3D* op3d;
	IfcSchema::IfcCartesianTransformationOperator2DnonUniform* op2dnonu;
	IfcSchema::IfcCartesianTransformationOperator3DnonUniform* op3dnonu;

	if ((op2dnonu = l->as<IfcSchema::IfcCartesianTransformationOperator2DnonUniform>()) != 0) {
		gp_GTrsf2d gtrsf2d;
		convert(op2dnonu, gtrsf2d);
		return gtrsf2d.Form() == gp_Identity;
	} else if ((op2d = l->as<IfcSchema::IfcCartesianTransformationOperator2D>()) != 0) {
		gp_Trsf2d trsf2d;
		convert(op2d, trsf2d);
		return trsf2d.Form() == gp_Identity;
	} else if ((op3dnonu = l->as<IfcSchema::IfcCartesianTransformationOperator3DnonUniform>()) != 0) {
		gp_GTrsf gtrsf;
		convert(op3dnonu, gtrsf);
		return gtrsf.Form() == gp_Identity;
	} else if ((op3d = l->as<IfcSchema::IfcCartesianTransformationOperator3D>()) != 0) {
		gp_Trsf trsf;
		convert(op3d, trsf);
		return trsf.Form() == gp_Identity;
	} else if ((ax2d = l->as<IfcSchema::IfcAxis2Placement2D>()) != 0) {
		gp_Trsf2d trsf2d;
		convert(ax2d, trsf2d);
		return trsf2d.Form() == gp_Identity;
	} else if ((ax3d = l->as<IfcSchema::IfcAxis2Placement3D>()) != 0) {
		gp_Trsf trsf;
		convert(ax3d, trsf);
		return trsf.Form() == gp_Identity;
	} else {
		throw IfcParse::IfcException("Invalid valuation for IfcAxis2Placement / IfcCartesianTransformationOperator");
	}
	*/
}

IfcGeom::NativeElement<double>* IfcGeom::CgalKernel::create_brep_for_representation_and_product(
	const IteratorSettings& settings, IfcSchema::IfcRepresentation* representation, IfcSchema::IfcProduct* product)
{
	IfcGeom::Representation::Native* shape;
	IfcGeom::ConversionResults shapes, shapes2;

	if (!convert_shapes(representation, shapes)) {
		return 0;
	}

	if (settings.get(IteratorSettings::APPLY_LAYERSETS)) {
		Logger::Message(Logger::LOG_ERROR, "Not implemented APPLY_LAYERSETS");
	}

	int parent_id = -1;
	try {
		IfcSchema::IfcObjectDefinition* parent_object = get_decomposing_entity(product);
		if (parent_object) {
			parent_id = parent_object->entity->id();
		}
	} catch (...) {}

	const std::string name = product->hasName() ? product->Name() : "";
	const std::string guid = product->GlobalId();

	cgal_placement_t trsf;
	try {
		 convert(product->ObjectPlacement(), trsf);
	} catch (...) {}

	// Does the IfcElement have any IfcOpenings?
	// Note that openings for IfcOpeningElements are not processed
	IfcSchema::IfcRelVoidsElement::list::ptr openings = find_openings(product);

	const std::string product_type = IfcSchema::Type::ToString(product->type());
	ElementSettings element_settings(settings, getValue(GV_LENGTH_UNIT), product_type);

  if (!settings.get(IfcGeom::IteratorSettings::DISABLE_OPENING_SUBTRACTIONS) && openings && openings->size()) {
    IfcGeom::ConversionResults opened_shapes;
    convert_openings(product,openings,shapes,trsf,opened_shapes);
    if (settings.get(IteratorSettings::USE_WORLD_COORDS)) {
      for ( IfcGeom::ConversionResults::iterator it = opened_shapes.begin(); it != opened_shapes.end(); ++ it ) {
        it->prepend(new CgalPlacement(trsf));
      }
      trsf = cgal_placement_t();
    }
    shape = new IfcGeom::Representation::Native(element_settings, representation->entity->id(), opened_shapes);
  } else if (settings.get(IteratorSettings::USE_WORLD_COORDS)) {
    for ( IfcGeom::ConversionResults::iterator it = shapes.begin(); it != shapes.end(); ++ it ) {
      it->prepend(new CgalPlacement(trsf));
    }
    trsf = cgal_placement_t();
    shape = new IfcGeom::Representation::Native(element_settings, representation->entity->id(), shapes);
  } else {
    shape = new IfcGeom::Representation::Native(element_settings, representation->entity->id(), shapes);
  }

	std::string context_string = "";
	if (representation->hasRepresentationIdentifier()) {
		context_string = representation->RepresentationIdentifier();
	} else if (representation->ContextOfItems()->hasContextType()) {
		context_string = representation->ContextOfItems()->ContextType();
	}

	return new NativeElement<double>(
		product->entity->id(),
		parent_id,
		name,
		product_type,
		guid,
		context_string,
		new CgalPlacement(trsf),
		boost::shared_ptr<IfcGeom::Representation::Native>(shape)
	);
}

IfcGeom::NativeElement<double>* IfcGeom::CgalKernel::create_brep_for_processed_representation(
	const IteratorSettings& /*settings*/, IfcSchema::IfcRepresentation* representation, IfcSchema::IfcProduct* product,
	IfcGeom::NativeElement<double>* brep)
{
	int parent_id = -1;
	try {
		IfcSchema::IfcObjectDefinition* parent_object = get_decomposing_entity(product);
		if (parent_object) {
			parent_id = parent_object->entity->id();
		}
	} catch (...) {}

	const std::string name = product->hasName() ? product->Name() : "";
	const std::string guid = product->GlobalId();

	cgal_placement_t trsf;
	try {
		 convert(product->ObjectPlacement(), trsf);
	} catch (...) {}

	std::string context_string = "";
	if (representation->hasRepresentationIdentifier()) {
		context_string = representation->RepresentationIdentifier();
	} else if (representation->ContextOfItems()->hasContextType()) {
		context_string = representation->ContextOfItems()->ContextType();
	}

	const std::string product_type = IfcSchema::Type::ToString(product->type());

	return new NativeElement<double>(
		product->entity->id(),
		parent_id,
		name,
		product_type,
		guid,
		context_string,
		new CgalPlacement(trsf),
		brep->geometry_pointer()
	);
}

bool IfcGeom::CgalKernel::convert_openings(const IfcSchema::IfcProduct* entity, const IfcSchema::IfcRelVoidsElement::list::ptr& openings,
                                                  const IfcGeom::ConversionResults& entity_shapes, const cgal_placement_t& entity_trsf, IfcGeom::ConversionResults& cut_shapes) {
  
  std::string error_file_path;
  for (unsigned int error_number = 1; error_number < 1000; ++error_number) {
    error_file_path = std::string("/Users/ken/Desktop/error/error");
    error_file_path += std::to_string(error_number);
    error_file_path += ".off";
    std::ifstream file_path_test(error_file_path);
    if (!file_path_test.good()) break;
  }
  
  std::list<cgal_shape_t> opening_shapelist;
  
  for ( IfcSchema::IfcRelVoidsElement::list::it it = openings->begin(); it != openings->end(); ++ it ) {
    IfcSchema::IfcRelVoidsElement* v = *it;
    IfcSchema::IfcFeatureElementSubtraction* fes = v->RelatedOpeningElement();
    if ( fes->is(IfcSchema::Type::IfcOpeningElement) ) {
      if (!fes->hasRepresentation()) continue;
      
      // Convert the IfcRepresentation of the IfcOpeningElement
      cgal_placement_t opening_trsf;
      if (fes->hasObjectPlacement()) {
        try {
          convert(fes->ObjectPlacement(),opening_trsf);
        } catch (...) {}
      }
      
      // Move the opening into the coordinate system of the IfcProduct
      opening_trsf = entity_trsf.inverse() * opening_trsf;
      
      IfcSchema::IfcProductRepresentation* prodrep = fes->Representation();
      IfcSchema::IfcRepresentation::list::ptr reps = prodrep->Representations();
      
      IfcGeom::ConversionResults opening_shapes;
						
      for ( IfcSchema::IfcRepresentation::list::it it2 = reps->begin(); it2 != reps->end(); ++ it2 ) {
        convert_shapes(*it2,opening_shapes);
      }
      
      for ( unsigned int i = 0; i < opening_shapes.size(); ++ i ) {
        cgal_placement_t gtrsf;
        if (opening_shapes[i].Placement()) {
          gtrsf = *(CgalPlacement*)opening_shapes[i].Placement();
        }
        gtrsf = opening_trsf * gtrsf;
        cgal_shape_t opening_shape(((CgalShape*)opening_shapes[i].Shape())->shape());
        for (auto &vertex: vertices(opening_shape)) vertex->point() = vertex->point().transform(gtrsf);
        opening_shapelist.push_back(opening_shape);
      }
      
    }
  }
  
  // Iterate over the shapes of the IfcProduct
  for ( IfcGeom::ConversionResults::const_iterator it3 = entity_shapes.begin(); it3 != entity_shapes.end(); ++ it3 ) {
    const cgal_shape_t& entity_shape_unlocated(((CgalShape*)it3->Shape())->shape());
    cgal_shape_t entity_shape(entity_shape_unlocated);
    if (it3->Placement()) {
      const cgal_placement_t& entity_shape_gtrsf = *(CgalPlacement*)it3->Placement();
      for (auto &vertex: vertices(entity_shape)) vertex->point() = vertex->point().transform(entity_shape_gtrsf);
    }
    
    cgal_shape_t original_entity_shape(entity_shape);
    if (!entity_shape.is_valid()) {
      Logger::Message(Logger::LOG_ERROR, "Conversion to Nef will fail. Invalid entity:", entity->entity);
      std::ofstream ferror;
      ferror.open(error_file_path);
      ferror << original_entity_shape << std::endl;
      ferror.close();
      return false;
    } if (!entity_shape.is_closed()) {
      // TODO: There can be substractions to remove parts of non-volumetric objects. Maybe iterate over all faces of an entity and put them in a Nef_polyhedron_3 through Boolean union? Highly inefficient but maybe desirable...
      Logger::Message(Logger::LOG_ERROR, "Subtraction of openings not supported for non-closed entity:", entity->entity);
      std::ofstream ferror;
      ferror.open(error_file_path);
      ferror << original_entity_shape << std::endl;
      ferror.close();
      return false;
    } bool success = false;
    try {
      success = CGAL::Polygon_mesh_processing::triangulate_faces(entity_shape);
    } catch (...) {
      Logger::Message(Logger::LOG_ERROR, "Triangulation of entity crashed:", entity->entity);
      std::ofstream ferror;
      ferror.open(error_file_path);
      ferror << original_entity_shape << std::endl;
      ferror.close();
      return false;
    } if (!success) {
      Logger::Message(Logger::LOG_ERROR, "Triangulation of entity failed:", entity->entity);
      std::ofstream ferror;
      ferror.open(error_file_path);
      ferror << original_entity_shape << std::endl;
      ferror.close();
      return false;
    } if (!entity_shape.is_valid()) {
      Logger::Message(Logger::LOG_ERROR, "Conversion to Nef will fail. Invalid triangulated entity:", entity->entity);
      std::ofstream ferror;
      ferror.open(error_file_path);
      ferror << original_entity_shape << std::endl;
      ferror.close();
      return false;
    } if (CGAL::Polygon_mesh_processing::does_self_intersect(entity_shape)) {
      Logger::Message(Logger::LOG_ERROR, "Conversion to Nef will fail. Self-intersecting entity:", entity->entity);
      std::ofstream ferror;
      ferror.open(error_file_path);
      ferror << original_entity_shape << std::endl;
      ferror.close();
      return false;
    } CGAL::Nef_polyhedron_3<Kernel> nef_brep_cut_result;
    try {
      nef_brep_cut_result = CGAL::Nef_polyhedron_3<Kernel>(entity_shape);
    } catch (...) {
      Logger::Message(Logger::LOG_ERROR, "Could not convert entity to Nef:", entity->entity);
      std::ofstream ferror;
      ferror.open(error_file_path);
      ferror << original_entity_shape << std::endl;
      ferror.close();
      return false;
    } try {
      cgal_shape_t brep_cut_result;
      nef_brep_cut_result.convert_to_polyhedron(brep_cut_result);
    } catch (...) {
      Logger::Message(Logger::LOG_WARNING, "Final conversion will likely fail. Could not convert entity from Nef:", entity->entity);
      std::ofstream ferror;
      ferror.open(error_file_path);
      ferror << original_entity_shape << std::endl;
      ferror.close();
      return false;
    }
    
    for (auto &opening: opening_shapelist) {
      
      cgal_shape_t original_opening_shape(opening);
      if (!opening.is_valid()) {
        Logger::Message(Logger::LOG_ERROR, "Conversion to Nef will fail. Invalid opening in entity:", entity->entity);
//        std::ofstream fentity;
//        fentity.open("/Users/ken/Desktop/entity.off");
//        fentity << original_entity_shape << std::endl;
//        fentity.close();
        std::ofstream ferror;
        ferror.open(error_file_path);
        ferror << original_opening_shape << std::endl;
        ferror.close();
        return false;
      } if (!opening.is_closed()) {
        Logger::Message(Logger::LOG_ERROR, "Subtraction of opening makes no sense. Not closed opening in entity:", entity->entity);
//        std::ofstream fentity;
//        fentity.open("/Users/ken/Desktop/entity.off");
//        fentity << original_entity_shape << std::endl;
//        fentity.close();
        std::ofstream ferror;
        ferror.open(error_file_path);
        ferror << original_opening_shape << std::endl;
        ferror.close();
        return false;
      } success = false;
      try {
        success = CGAL::Polygon_mesh_processing::triangulate_faces(opening);
      } catch (...) {
        Logger::Message(Logger::LOG_ERROR, "Triangulation of opening of entity crashed:", entity->entity);
//        std::ofstream fentity;
//        fentity.open("/Users/ken/Desktop/entity.off");
//        fentity << original_entity_shape << std::endl;
//        fentity.close();
        std::ofstream ferror;
        ferror.open(error_file_path);
        ferror << original_opening_shape << std::endl;
        ferror.close();
        return false;
      } if (!success) {
        Logger::Message(Logger::LOG_ERROR, "Triangulation of opening of entity failed:", entity->entity);
//        std::ofstream fentity;
//        fentity.open("/Users/ken/Desktop/entity.off");
//        fentity << original_entity_shape << std::endl;
//        fentity.close();
        std::ofstream ferror;
        ferror.open(error_file_path);
        ferror << original_opening_shape << std::endl;
        ferror.close();
        return false;
      } if (!opening.is_valid()) {
        Logger::Message(Logger::LOG_ERROR, "Conversion to Nef will fail. Invalid triangulated opening in entity:", entity->entity);
//        std::ofstream fentity;
//        fentity.open("/Users/ken/Desktop/entity.off");
//        fentity << original_entity_shape << std::endl;
//        fentity.close();
        std::ofstream ferror;
        ferror.open(error_file_path);
        ferror << original_opening_shape << std::endl;
        ferror.close();
        return false;
      } if (CGAL::Polygon_mesh_processing::does_self_intersect(entity_shape)) {
        Logger::Message(Logger::LOG_ERROR, "Conversion to Nef will fail. Self-intersecting opening of entity:", entity->entity);
//        std::ofstream fentity;
//        fentity.open("/Users/ken/Desktop/entity.off");
//        fentity << original_entity_shape << std::endl;
//        fentity.close();
        std::ofstream ferror;
        ferror.open(error_file_path);
        ferror << original_opening_shape << std::endl;
        ferror.close();
        return false;
      } CGAL::Nef_polyhedron_3<Kernel> nef_opening;
      try {
        nef_opening = CGAL::Nef_polyhedron_3<Kernel>(opening);
      } catch (...) {
        Logger::Message(Logger::LOG_ERROR, "Could not convert opening of entity to Nef:", entity->entity);
//        std::ofstream fentity;
//        fentity.open("/Users/ken/Desktop/entity.off");
//        fentity << original_entity_shape << std::endl;
//        fentity.close();
        std::ofstream ferror;
        ferror.open(error_file_path);
        ferror << original_opening_shape << std::endl;
        ferror.close();
        return false;
      } try {
        cgal_shape_t opening_shape;
        nef_opening.convert_to_polyhedron(opening_shape);
      } catch (...) {
        Logger::Message(Logger::LOG_WARNING, "Final conversion will likely fail. Could not convert opening of entity from Nef:", entity->entity);
//        std::ofstream fentity;
//        fentity.open("/Users/ken/Desktop/entity.off");
//        fentity << original_entity_shape << std::endl;
//        fentity.close();
        std::ofstream ferror;
        ferror.open(error_file_path);
        ferror << original_opening_shape << std::endl;
        ferror.close();
        return false;
      } try {
        nef_brep_cut_result -= nef_opening;
      } catch (...) {
        Logger::Message(Logger::LOG_ERROR, "Could not subtract Nef opening of entity:", entity->entity);
//        std::ofstream fentity;
//        fentity.open("/Users/ken/Desktop/entity.off");
//        fentity << original_entity_shape << std::endl;
//        fentity.close();
        std::ofstream ferror;
        ferror.open(error_file_path);
        ferror << original_opening_shape << std::endl;
        ferror.close();
        return false;
      }
    }
    
    try {
      nef_brep_cut_result.convert_to_polyhedron(entity_shape);
    } catch (...) {
      Logger::Message(Logger::LOG_ERROR, "Could not convert entity with openings from Nef:", entity->entity);
      std::ofstream ferror;
      ferror.open(error_file_path);
      ferror << original_entity_shape << std::endl;
      ferror.close();
      return false;
    } cut_shapes.push_back(IfcGeom::ConversionResult(new CgalShape(entity_shape), &it3->Style()));
    
  } return true;
}
