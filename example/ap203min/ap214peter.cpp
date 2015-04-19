//*****************************************************************************
//  AP203 Minimum
//
//  This program is intended to serve as a tutorial example for programmers
// interested in learning about ISO 10303 (STEP), the STEPcode project, and
// the AP203 portion of STEP.
//
//  This program creates and connects the minimum set of entities required
// to form a valid AP203 STEP file.  Inspiration for this program came from
// Appendix B of 'Recommended Practices for AP 203' released to the public
// domain in 1998 by PDES Inc.  The recommended practices document is
// available online at:
//
// http://www.steptools.com/support/stdev_docs/express/ap203/recprac203v8.pdf
//
//  The recommended practices document states:
//
//     "This document has been developed by the PDES, Inc. Industry
//     consortium to aid in accelerating the implementation of the
//     STEP standard.  It has not been copyrighted to allow for the
//     free exchange of the information.  PDES, Inc. Requests that
//     anyone using this information provide acknowledgment that
//     PDES, Inc. was the original author."
//
//  In the same spirit, this program is released to the public domain.  Any
// part of this program may be freely copied in part or in full for any
// purpose.  No acknowledgment is required for the use of this code.
//
//  This program was written by Rob McDonald in October 2013.  Since that
// time, it has been maintained by the STEPcode project.
//
//****************************************************************************/


//  This program uses CMake to build Makefiles or other project files.  It
// includes a CMakeLists.txt program that also builds STEPcode as a sub-build.
// To compile, you must tell CMake where your STEPcode source tree is located
// by setting STEPCODE_ROOT_DIR.  To compile:
//
//  $ pwd
//  .../stepcode/example/ap203min
//  $ mkdir build
//  $ cd build
//  $ cmake .. -DSTEPCODE_ROOT_DIR=../../..
//  $ make
//  $ cd bin
//  $ ls
//  AP203Minimum
//  $./AP203Minimum
//  AP203Minimum    outfile.step

#include <STEPfile.h>
#include <sdai.h>
#include <STEPattribute.h>
#include <ExpDict.h>
#include <Registry.h>
#include <errordesc.h>

#include <STEPcomplex.h>
#include <SdaiHeaderSchema.h>

#include "schema.h"

#include <SdaiAUTOMOTIVE_DESIGN.h>

//#include <math.h>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#if 0
#  define DEBUG_PRODUCT_DEFINITION_SEARCH
#  define DEBUG_CHILD_REMOVAL
#  define DEBUG_PRODUCT_DEFINITION
#else
#  undef DEBUG_PRODUCT_DEFINITION_SEARCH
#  undef DEBUG_CHILD_REMOVAL
#  undef DEBUG_PRODUCT_DEFINITION
#endif

enum LenEnum { MM, CM, M, IN, FT, YD };
enum AngEnum { RAD, DEG };

STEPcomplex * Geometric_Context( Registry * registry, InstMgr * instance_list, const LenEnum & len, const AngEnum & angle, const char * tolstr ) {
    int instance_cnt = 0;
    STEPattribute * attr;
    STEPcomplex * stepcomplex;

    SdaiDimensional_exponents * dimensional_exp = new SdaiDimensional_exponents();
    dimensional_exp->length_exponent_( 0.0 );
    dimensional_exp->mass_exponent_( 0.0 );
    dimensional_exp->time_exponent_( 0.0 );
    dimensional_exp->electric_current_exponent_( 0.0 );
    dimensional_exp->thermodynamic_temperature_exponent_( 0.0 );
    dimensional_exp->amount_of_substance_exponent_( 0.0 );
    dimensional_exp->luminous_intensity_exponent_( 0.0 );
    instance_list->Append( ( SDAI_Application_instance * ) dimensional_exp, completeSE );
    instance_cnt++;

    STEPcomplex * ua_length;
    // First set up metric units if appropriate.  Default to mm.
    // If imperial units, set up mm to be used as base to define imperial units.
    Si_prefix pfx = Si_prefix__milli;
    switch( len ) {
        case CM:
            pfx = Si_prefix__centi;
            break;
        case M:
            pfx = Si_prefix_unset;
            break;
    }

    const char * ua_length_types[4] = { "length_unit", "named_unit", "si_unit", "*" };
    ua_length = new STEPcomplex( registry, ( const char ** ) ua_length_types, instance_cnt );
    stepcomplex = ua_length->head;
    while( stepcomplex ) {
        if (stepcomplex->EntityName() == NULL)
          {
            std::cout << "ERROR: Creating geometric context failed" << std::endl;
            return NULL;
          }
        if( !strcmp( stepcomplex->EntityName(), "Si_Unit" ) ) {
            stepcomplex->ResetAttributes();
            while( ( attr = stepcomplex->NextAttribute() ) != NULL ) {
                if( !strcmp( attr->Name(), "prefix" ) ) {
                    attr->Enum( new SdaiSi_prefix_var( pfx ) );
                }
                if( !strcmp( attr->Name(), "name" ) ) {
                    attr->Enum( new SdaiSi_unit_name_var( Si_unit_name__metre ) );
                }
            }
        }
        stepcomplex = stepcomplex->sc;
    }
    instance_list->Append( ( SDAI_Application_instance * ) ua_length, completeSE );
    instance_cnt++;

    // If imperial, create conversion based unit.
    if( len >= IN ) {
        STEPcomplex * len_mm = ua_length;

        char lenname[10];
        double lenconv;

        switch( len ) {
            case IN:
                strcat( lenname, "'INCH'\0" );
                lenconv = 25.4;
                break;
            case FT:
                strcat( lenname, "'FOOT'\0" );
                lenconv = 25.4 * 12.0;
                break;
            case YD:
                strcat( lenname, "'YARD'\0" );
                lenconv = 25.4 * 36.0;
                break;
        }

        SdaiUnit * len_unit = new SdaiUnit( ( SdaiNamed_unit * ) len_mm );

        SdaiMeasure_value * len_measure_value = new SdaiMeasure_value( lenconv, automotive_design::t_measure_value );
        len_measure_value->SetUnderlyingType( automotive_design::t_length_measure );

        SdaiLength_measure_with_unit * len_measure_with_unit = new SdaiLength_measure_with_unit();
        len_measure_with_unit->value_component_( len_measure_value );
        len_measure_with_unit->unit_component_( len_unit );
        instance_list->Append( ( SDAI_Application_instance * ) len_measure_with_unit, completeSE );
        instance_cnt++;

        SdaiDimensional_exponents * dimensional_exp_len = new SdaiDimensional_exponents();
        dimensional_exp_len->length_exponent_( 1.0 );
        dimensional_exp_len->mass_exponent_( 0.0 );
        dimensional_exp_len->time_exponent_( 0.0 );
        dimensional_exp_len->electric_current_exponent_( 0.0 );
        dimensional_exp_len->thermodynamic_temperature_exponent_( 0.0 );
        dimensional_exp_len->amount_of_substance_exponent_( 0.0 );
        dimensional_exp_len->luminous_intensity_exponent_( 0.0 );
        instance_list->Append( ( SDAI_Application_instance * ) dimensional_exp_len, completeSE );
        instance_cnt++;

        const char * ua_conv_len_types[4] = { "conversion_based_unit", "named_unit", "length_unit", "*" };
        ua_length = new STEPcomplex( registry, ( const char ** ) ua_conv_len_types, instance_cnt );
        stepcomplex = ua_length->head;
        while( stepcomplex ) {
            if (stepcomplex->EntityName() == NULL)
              {
                std::cout << "ERROR: Creating geometric context failed" << std::endl;
                return NULL;
              }
            if( !strcmp( stepcomplex->EntityName(), "Conversion_Based_Unit" ) ) {
                stepcomplex->ResetAttributes();
                while( ( attr = stepcomplex->NextAttribute() ) != NULL ) {
                    if( !strcmp( attr->Name(), "name" ) ) {
                        attr->StrToVal( lenname );
                    }
                    if( !strcmp( attr->Name(), "conversion_factor" ) ) {
                        attr->Entity( ( STEPentity * )( len_measure_with_unit ) );
                    }
                }
            }
            if( !strcmp( stepcomplex->EntityName(), "Named_Unit" ) ) {
                stepcomplex->ResetAttributes();
                while( ( attr = stepcomplex->NextAttribute() ) != NULL ) {
                    if( !strcmp( attr->Name(), "dimensions" ) ) {
                        attr->Entity( ( STEPentity * )( dimensional_exp_len ) );
                    }
                }
            }
            stepcomplex = stepcomplex->sc;
        }

        instance_list->Append( ( SDAI_Application_instance * ) ua_length, completeSE );
        instance_cnt++;
    }

    SdaiUncertainty_measure_with_unit * uncertainty = ( SdaiUncertainty_measure_with_unit * )registry->ObjCreate( "UNCERTAINTY_MEASURE_WITH_UNIT" );
    uncertainty->name_( "'DISTANCE_ACCURACY_VALUE'" );
    uncertainty->description_( "'Threshold below which geometry imperfections (such as overlaps) are not considered errors.'" );
    SdaiUnit * tol_unit = new SdaiUnit( ( SdaiNamed_unit * ) ua_length );
    uncertainty->ResetAttributes();
    {
        while( ( attr = uncertainty->NextAttribute() ) != NULL ) {
            if( !strcmp( attr->Name(), "unit_component" ) ) {
                attr->Select( tol_unit );
            }
            if( !strcmp( attr->Name(), "value_component" ) ) {
                attr->StrToVal( tolstr );
            }
        }
    }
    instance_list->Append( ( SDAI_Application_instance * ) uncertainty, completeSE );
    instance_cnt++;

    // First set up radians as base angle unit.
    const char * ua_plane_angle_types[4] = { "named_unit", "plane_angle_unit", "si_unit", "*" };
    STEPcomplex * ua_plane_angle = new STEPcomplex( registry, ( const char ** ) ua_plane_angle_types, instance_cnt );
    stepcomplex = ua_plane_angle->head;
    while( stepcomplex ) {
        if (stepcomplex->EntityName() == NULL)
          {
            std::cout << "ERROR: Creating geometric context failed" << std::endl;
            return NULL;
          }
        if( !strcmp( stepcomplex->EntityName(), "Si_Unit" ) ) {
            stepcomplex->ResetAttributes();
            while( ( attr = stepcomplex->NextAttribute() ) != NULL ) {
                if( !strcmp( attr->Name(), "name" ) ) {
                    attr->Enum( new SdaiSi_unit_name_var( Si_unit_name__radian ) );
                }
            }
        }
        stepcomplex = stepcomplex->sc;
    }
    instance_list->Append( ( SDAI_Application_instance * ) ua_plane_angle, completeSE );
    instance_cnt++;

#if 0
    // If degrees, create conversion based unit.
    if( angle == DEG ) {
        STEPcomplex * ang_rad = ua_plane_angle;

        const double angconv = ( 3.14159265358979323846264338327950 / 180.0 );

        SdaiUnit * p_ang_unit = new SdaiUnit( ( SdaiNamed_unit * ) ang_rad );

        SdaiMeasure_value * p_ang_measure_value = new SdaiMeasure_value( angconv, automotive_design::t_measure_value );
        p_ang_measure_value->SetUnderlyingType( automotive_design::t_plane_angle_measure );

        SdaiPlane_angle_measure_with_unit * p_ang_measure_with_unit = new SdaiPlane_angle_measure_with_unit();
        p_ang_measure_with_unit->value_component_( p_ang_measure_value );
        p_ang_measure_with_unit->unit_component_( p_ang_unit );
        instance_list->Append( ( SDAI_Application_instance * ) p_ang_measure_with_unit, completeSE );
        instance_cnt++;

        const char * ua_conv_angle_types[4] = { "conversion_based_unit", "named_unit", "plane_angle_unit", "*" };
        ua_plane_angle = new STEPcomplex( registry, ( const char ** ) ua_conv_angle_types, instance_cnt );
        stepcomplex = ua_plane_angle->head;
        while( stepcomplex ) {
            if( !strcmp( stepcomplex->EntityName(), "Conversion_Based_Unit" ) ) {
                stepcomplex->ResetAttributes();
                while( ( attr = stepcomplex->NextAttribute() ) != NULL ) {
                    if( !strcmp( attr->Name(), "name" ) ) {
                        attr->StrToVal( "'DEGREES'" );
                    }
                    if( !strcmp( attr->Name(), "conversion_factor" ) ) {
                        attr->Entity( ( STEPentity * )( p_ang_measure_with_unit ) );
                    }
                }
            }
            if( !strcmp( stepcomplex->EntityName(), "Named_Unit" ) ) {
                stepcomplex->ResetAttributes();
                while( ( attr = stepcomplex->NextAttribute() ) != NULL ) {
                    if( !strcmp( attr->Name(), "dimensions" ) ) {
                        //attr->Entity( ( STEPentity * )( dimensional_exp ) );
                        //attr->set_null();
                        attr->Entity( dimensional_exp );
                    }
                }
            }
            stepcomplex = stepcomplex->sc;
        }
        instance_list->Append( ( SDAI_Application_instance * ) ua_plane_angle, completeSE );
        instance_cnt++;
    }
#endif

    const char * ua_solid_angle_types[4] = { "named_unit", "si_unit", "solid_angle_unit", "*" };
    STEPcomplex * ua_solid_angle = new STEPcomplex( registry, ( const char ** ) ua_solid_angle_types, instance_cnt );
    stepcomplex = ua_solid_angle->head;
    while( stepcomplex ) {
        if (stepcomplex->EntityName() == NULL)
          {
            std::cout << "ERROR: Creating geometric context failed" << std::endl;
            return NULL;
          }
        if( !strcmp( stepcomplex->EntityName(), "Si_Unit" ) ) {
            stepcomplex->ResetAttributes();
            while( ( attr = stepcomplex->NextAttribute() ) != NULL ) {
                if( !strcmp( attr->Name(), "name" ) ) {
                    attr->Enum( new SdaiSi_unit_name_var( Si_unit_name__steradian ) );
                }
            }
        }
        stepcomplex = stepcomplex->sc;
    }
    instance_list->Append( ( SDAI_Application_instance * ) ua_solid_angle, completeSE );
    instance_cnt++;

    // All units set up, stored in: ua_length, ua_plane_angle, ua_solid_angle
    const char * entNmArr[5] = { "geometric_representation_context", "global_uncertainty_assigned_context", "global_unit_assigned_context", "representation_context", "*" };
    STEPcomplex * complex_entity = new STEPcomplex( registry, ( const char ** ) entNmArr, instance_cnt );
    stepcomplex = complex_entity->head;

    while( stepcomplex ) {
        if (stepcomplex->EntityName() == NULL)
          {
            std::cout << "ERROR: Creating geometric context failed" << std::endl;
            return NULL;
          }

        if( !strcmp( stepcomplex->EntityName(), "Geometric_Representation_Context" ) ) {
            stepcomplex->ResetAttributes();
            while( ( attr = stepcomplex->NextAttribute() ) != NULL ) {
                if( !strcmp( attr->Name(), "coordinate_space_dimension" ) ) {
                    attr->StrToVal( "3" );
                }
            }
        }

        if( !strcmp( stepcomplex->EntityName(), "Global_Uncertainty_Assigned_Context" ) ) {
            stepcomplex->ResetAttributes();
            while( ( attr = stepcomplex->NextAttribute() ) != NULL ) {
                if( !strcmp( attr->Name(), "uncertainty" ) ) {
                    EntityAggregate * unc_agg = new EntityAggregate();
                    unc_agg->AddNode( new EntityNode( ( SDAI_Application_instance * ) uncertainty ) );
                    attr->Aggregate( unc_agg );
                }
            }

        }

        if( !strcmp( stepcomplex->EntityName(), "Global_Unit_Assigned_Context" ) ) {
            stepcomplex->ResetAttributes();
            while( ( attr = stepcomplex->NextAttribute() ) != NULL ) {
                std::string attrval;
                if( !strcmp( attr->Name(), "units" ) ) {
                    EntityAggregate * unit_assigned_agg = new EntityAggregate();
                    unit_assigned_agg->AddNode( new EntityNode( ( SDAI_Application_instance * ) ua_length ) );
                    unit_assigned_agg->AddNode( new EntityNode( ( SDAI_Application_instance * ) ua_plane_angle ) );
                    unit_assigned_agg->AddNode( new EntityNode( ( SDAI_Application_instance * ) ua_solid_angle ) );
                    attr->Aggregate( unit_assigned_agg );
                }
            }
        }

        if( !strcmp( stepcomplex->EntityName(), "Representation_Context" ) ) {
            stepcomplex->ResetAttributes();
            while( ( attr = stepcomplex->NextAttribute() ) != NULL ) {
                if( !strcmp( attr->Name(), "context_identifier" ) ) {
                    attr->StrToVal( "'STANDARD'" );
                }
                if( !strcmp( attr->Name(), "context_type" ) ) {
                    attr->StrToVal( "'3D'" );
                }
            }
        }
        stepcomplex = stepcomplex->sc;
    }
    instance_list->Append( ( SDAI_Application_instance * ) complex_entity, completeSE );
    instance_cnt++;

    return complex_entity;
}


SdaiCartesian_point *
MakePoint (Registry *registry, InstMgr *instance_list,
           const double &x, const double &y, const double &z)
{
  SdaiCartesian_point * pnt = (SdaiCartesian_point *) registry->ObjCreate ("CARTESIAN_POINT");
  pnt->name_ ("''");

  RealAggregate * coords = pnt->coordinates_ ();

  RealNode * xnode = new RealNode ();
  xnode->value = x;
  coords->AddNode (xnode);

  RealNode * ynode = new RealNode ();
  ynode->value = y;
  coords->AddNode (ynode);

  RealNode * znode = new RealNode ();
  znode->value = z;
  coords->AddNode (znode);

  instance_list->Append ((SDAI_Application_instance *)pnt, completeSE);

  return pnt;
}


SdaiDirection *
MakeDirection (Registry *registry, InstMgr *instance_list,
               const double &x, const double &y, const double &z)
{
  SdaiDirection * dir = (SdaiDirection *) registry->ObjCreate ("DIRECTION");
  dir->name_ ("''");

  RealAggregate * components = dir->direction_ratios_ ();

  RealNode * xnode = new RealNode ();
  xnode->value = x;
  components->AddNode (xnode);

  RealNode * ynode = new RealNode ();
  ynode->value = y;
  components->AddNode (ynode);

  RealNode * znode = new RealNode ();
  znode->value = z;
  components->AddNode (znode);

  instance_list->Append ((SDAI_Application_instance *) dir, completeSE);

  return dir;
}


SdaiAxis2_placement_3d *
MakeAxis (Registry *registry, InstMgr *instance_list,
          const double &px, const double &py, const double & pz,
          const double &ax, const double &ay, const double & az,
          const double &rx, const double &ry, const double & rz)
{
  SdaiCartesian_point * pnt = MakePoint (registry, instance_list, px, py, pz);
  SdaiDirection * axis = MakeDirection (registry, instance_list, ax, ay, az);
  SdaiDirection * refd = MakeDirection (registry, instance_list, rx, ry, rz);

  SdaiAxis2_placement_3d * placement = (SdaiAxis2_placement_3d *) registry->ObjCreate ("AXIS2_PLACEMENT_3D");
  placement->name_ ("''");
  placement->location_ (pnt);
  placement->axis_ (axis);
  placement->ref_direction_ (refd);

  instance_list->Append ((SDAI_Application_instance *) placement, completeSE);

  return placement;
}


SdaiAxis2_placement_3d *
DefaultAxis (Registry *registry, InstMgr *instance_list)
{
  return MakeAxis (registry, instance_list, 0.0, 0.0, 0.0,
                                            0.0, 0.0, 1.0,
                                            1.0, 0.0, 0.0);
}


typedef std::list<SdaiProduct_definition *> pd_list;


void
find_all_pd_with_sdr (InstMgr *instance_list, pd_list *pd_list)
{
  int search_index = 0;

  // Loop over the instances of SHAPE_DEFITION_REPRESENTATION in the file
  SdaiShape_definition_representation *sdr;
  while (ENTITY_NULL != (sdr = (SdaiShape_definition_representation *)
                               instance_list->GetApplication_instance ("Shape_definition_representation", search_index)))
    {
      STEPentity *sdr_used_representation = sdr->used_representation_ ();
      SdaiRepresented_definition *sdr_definition = sdr->definition_ ();
      SdaiProduct_definition_shape *pds = (SdaiProduct_definition_shape *)(SdaiProperty_definition_ptr)(*sdr_definition);
      SdaiProduct_definition *pd = *(SdaiCharacterized_product_definition_ptr)(*pds->definition_ ());
      SdaiProduct_definition_formation *pdf = pd->formation_ ();
      SdaiProduct *p = pdf->of_product_ ();

      pd_list->push_back (pd);

#ifdef DEBUG_PRODUCT_DEFINITION_SEARCH
      std::cout << "Got a SDR, #" << sdr->StepFileId ();
      std::cout << " used_representation (sr or descendant) = #" << sdr_used_representation->StepFileId ();
      std::cout << " definition (pds) = #" << pds->StepFileId ();
      std::cout << " pds->definition (pd) = #" << pd->StepFileId ();
      std::cout << " pd->formation (pdf) = #" << pdf->StepFileId ();
      std::cout << " pdf->product (p) = #" << p->StepFileId ();
      std::cout << std::endl;
      std::cout << "Product id = " << p->id_ ().c_str () << " name = " << p->name_ ().c_str ();
      std::cout << std::endl;
#endif

      SdaiShape_representation *sr = (SdaiShape_representation *)sdr_used_representation;

#ifdef DEBUG_PRODUCT_DEFINITION_SEARCH
      std::cout << "SR is actually of type " << ((STEPentity *)sr)->EntityName () << std::endl;
      std::cout << std::endl;
#endif

      int id = sdr->StepFileId ();
      MgrNode * mnode = instance_list->FindFileId (id);
      search_index = instance_list->GetIndex (mnode) + 1;
    }
}

/* entityName should be the name of entity Assembly_component_usage or one of its subtypes
 * typically this will be "Assembly_component_usage" or "Next_assembly_usage_occurance"
 */
void
find_and_remove_child_pd (InstMgr *instance_list, pd_list *pd_list, const char *entityName)
{
  int search_index = 0;

  SdaiAssembly_component_usage *acu;
  while (ENTITY_NULL != (acu = (SdaiAssembly_component_usage *)
                               instance_list->GetApplication_instance (entityName, search_index)))
    {
      SdaiProduct_definition *relating_pd = acu->relating_product_definition_ ();
      SdaiProduct_definition *related_pd = acu->related_product_definition_ ();

#ifdef DEBUG_CHILD_REMOVAL
      std::cout << "Product " << related_pd->formation_ ()->of_product_ ()->id_ ().c_str ();
      std::cout << " is a child of " << relating_pd->formation_ ()->of_product_ ()->id_ ().c_str ();
      std::cout << ".. removing it from list of possible root products";
      std::cout << std::endl;
#endif

      /* Remove related_pd from the list of viable product definitions */
      pd_list->remove (related_pd);

      int id = acu->StepFileId ();
      MgrNode * mnode = instance_list->FindFileId (id);
      search_index = instance_list->GetIndex (mnode) + 1;
    }
#ifdef DEBUG_CHILD_REMOVAL
  std::cout << std::endl;
#endif
}


SdaiShape_definition_representation *
find_sdr_for_pd (InstMgr *instance_list, SdaiProduct_definition *target_pd)
{
  int search_index = 0;

  // Loop over the instances of SHAPE_DEFITION_REPRESENTATION in the file
  SdaiShape_definition_representation *sdr;
  while (ENTITY_NULL != (sdr = (SdaiShape_definition_representation *)
                               instance_list->GetApplication_instance ("Shape_definition_representation", search_index)))
    {
      SdaiProduct_definition_shape *pds = (SdaiProduct_definition_shape *)(SdaiProperty_definition_ptr)(*sdr->definition_ ());
      SdaiProduct_definition *pd = *(SdaiCharacterized_product_definition_ptr)(*pds->definition_ ());

      /* Return the SHAPE_REPRESETATION (or subclass) associated with the first SHAPE_DEFINITION_REPRESENTATION for the required PRODUCT_DEFINITION */
      if (pd == target_pd)
        return sdr;
//        return (SdaiShape_representation *)sdr->used_representation_ ();

      int id = sdr->StepFileId ();
      MgrNode * mnode = instance_list->FindFileId (id);
      search_index = instance_list->GetIndex (mnode) + 1;
    }

  return NULL;
}


SdaiShape_representation *
find_sr_for_pd (InstMgr *instance_list, SdaiProduct_definition *target_pd)
{
  SdaiShape_definition_representation *sdr = find_sdr_for_pd (instance_list, target_pd);
  return (SdaiShape_representation *)sdr->used_representation_ ();
}


SdaiAxis2_placement_3d *
find_axis2_placement_3d_in_sr (SdaiShape_representation *sr)
{
  SingleLinkNode *iter = sr->items_ ()->GetHead ();

  while (iter != NULL)
    {
      SDAI_Application_instance *node = ((EntityNode *)iter)->node;

      if (strcmp (node->EntityName (), "Axis2_Placement_3d") == 0)
        return (SdaiAxis2_placement_3d *)node;

      iter = iter->NextNode ();
    }

  return NULL;
}


void
write_ap214 (Registry *registry, InstMgr *instance_list, const char *filename)
{
  // STEPfile takes care of reading and writing Part 21 files
  STEPfile sfile (*registry, *instance_list, "", false );

  // Build file header
  InstMgr *header_instances = sfile.HeaderInstances ();

  int filename_length = strlen (filename);
  char *step_fn = new char[filename_length + 3];
  step_fn[0] = '\'';
  strncpy (step_fn + 1, filename, filename_length);
  step_fn[filename_length + 1] = '\'';
  step_fn[filename_length + 2] = '\0';

  SdaiFile_name *fn = (SdaiFile_name *)sfile.HeaderDefaultFileName ();
  header_instances->Append ((SDAI_Application_instance *) fn, completeSE);
  fn->name_ (step_fn);
  fn->time_stamp_ ("''");
  fn->author_ ()->AddNode (new StringNode( "''" ));
  fn->organization_ ()->AddNode (new StringNode( "''" ));
  fn->preprocessor_version_ ("''");
  fn->originating_system_ ("''");
  fn->authorization_ ("''");

  SdaiFile_description *fd = (SdaiFile_description *)sfile.HeaderDefaultFileDescription ();
  header_instances->Append ((SDAI_Application_instance *)fd, completeSE);
  fd->description_()->AddNode (new StringNode ("''"));
  fd->implementation_level_ ("'1'");

  SdaiFile_schema *fs = (SdaiFile_schema *) sfile.HeaderDefaultFileSchema ();
  header_instances->Append ((SDAI_Application_instance *)fs, completeSE);
  fs->schema_identifiers_ ()->AddNode (new StringNode("'AUTOMOTIVE_DESIGN { 1 0 10303 214 3 1 1 }'")); /* XXX: NOT SURE IF THIS IS CORRECT! */

  //sfile.WriteExchangeFile (filename);
  sfile.WriteExchangeFile (filename, false); /* Turn off validation to try and force a save for bad data */
  if (sfile.Error ().severity () < SEVERITY_USERMSG)
    {
      sfile.Error ().PrintContents (std::cout);
    }

  delete [] step_fn;
}


STEPcomplex *
MakeRrwtSrr (Registry *registry,
             InstMgr *inst_mgr,
             const char *description,
             const char *name,
             SdaiRepresentation_item *rep_1,
             SdaiRepresentation_item *rep_2,
             SdaiTransformation *transformation_operator)
{
  STEPattribute *attr;
  const char * rrwt_srr_types[4] = { "representation_relationship",
                                     "representation_relationship_with_transformation",
                                     "shape_representation_relationship",
                                     "*" };

  STEPcomplex *rrwt_srr = new STEPcomplex (registry, (const char **) rrwt_srr_types, 0 /* XXX: FileID ??? */);

  STEPcomplex *stepcomplex = rrwt_srr->head;
  while (stepcomplex)
    {
      if (!strcmp( stepcomplex->EntityName (), "Representation_Relationship"))
        {
          stepcomplex->ResetAttributes();
          while ((attr = stepcomplex->NextAttribute ()) != NULL)
            {
              if (!strcmp( attr->Name(), "description"))
                attr->String (new SDAI_String (description));
              else if( !strcmp( attr->Name(), "name" ) )
                attr->String (new SDAI_String (name));
              else if( !strcmp( attr->Name(), "rep_1" ) )
                attr->Entity (rep_1);
              else if( !strcmp( attr->Name(), "rep_2" ) )
                attr->Entity (rep_2);
            }
        }
      else if (!strcmp( stepcomplex->EntityName (), "Representation_Relationship_With_Transformation"))
        {
          stepcomplex->ResetAttributes();
          while ((attr = stepcomplex->NextAttribute ()) != NULL)
            {
              if (!strcmp( attr->Name(), "transformation_operator"))
                attr->Select (transformation_operator);
            }
        }
      stepcomplex = stepcomplex->sc;
    }

  return rrwt_srr;
}


SdaiProduct_definition *
create_parent_assembly (Registry *registry,
                        InstMgr *instance_list)
{
  // Build file data.  The entities have been created and added in order such that no entity
  // references a later entity.  This is not required, but has been done to give a logical
  // flow to the source and the resulting STEP file.

  // Global units and tolerance.
  STEPcomplex * context = Geometric_Context( registry, instance_list, MM, DEG, "LENGTH_MEASURE(0.0001)" );

  if (context == NULL)
    {
      std::cout << "ERROR: Problem creating parent assembly" << std::endl;
      return NULL;
    }

  // Primary coordinate system.
  SdaiAxis2_placement_3d * orig_transform = DefaultAxis( registry, instance_list );

  // Basic context through product and shape representation
  SdaiApplication_context * app_context = ( SdaiApplication_context * ) registry->ObjCreate( "APPLICATION_CONTEXT" );
  instance_list->Append( ( SDAI_Application_instance * ) app_context, completeSE );
  app_context->application_( "'core data for automotive mechanical design processes'" );

  SdaiProduct_context * prod_context = ( SdaiProduct_context * ) registry->ObjCreate( "PRODUCT_CONTEXT" );
  instance_list->Append( ( SDAI_Application_instance * ) prod_context, completeSE );
  prod_context->name_( "''" );
  prod_context->discipline_type_( "'mechanical'" );
  prod_context->frame_of_reference_( app_context );

  SdaiApplication_protocol_definition * app_protocol = ( SdaiApplication_protocol_definition * ) registry->ObjCreate( "APPLICATION_PROTOCOL_DEFINITION" );
  instance_list->Append( ( SDAI_Application_instance * ) app_protocol, completeSE );
  app_protocol->status_( "'international standard'" );
  app_protocol->application_protocol_year_( 2010 ); /* XXX: NOT SURE IF THIS IS CORRECT! */
  app_protocol->application_interpreted_model_schema_name_( "'automotive_design'" );
  app_protocol->application_( app_context );

  SdaiProduct * prod = ( SdaiProduct * ) registry->ObjCreate( "PRODUCT" );
  instance_list->Append( ( SDAI_Application_instance * ) prod, completeSE );
  prod->id_( "''" );
  prod->name_( "'prodname'" );
  prod->description_( "''" );
  prod->frame_of_reference_()->AddNode( new EntityNode( ( SDAI_Application_instance * ) prod_context ) );

  SdaiProduct_related_product_category * prodcat = ( SdaiProduct_related_product_category * ) registry->ObjCreate( "PRODUCT_RELATED_PRODUCT_CATEGORY" );
  instance_list->Append( ( SDAI_Application_instance * ) prodcat, completeSE );
  prodcat->name_( "'assembly'" );
  prodcat->description_( "''" );
  prodcat->products_()->AddNode( new EntityNode( ( SDAI_Application_instance * ) prod ) );

  SdaiProduct_definition_formation_with_specified_source * prod_def_form = ( SdaiProduct_definition_formation_with_specified_source * ) registry->ObjCreate( "PRODUCT_DEFINITION_FORMATION_WITH_SPECIFIED_SOURCE" );
  instance_list->Append( ( SDAI_Application_instance * ) prod_def_form, completeSE );
  prod_def_form->id_( "''" );
  prod_def_form->description_( "''" );
  prod_def_form->of_product_( prod );
  prod_def_form->make_or_buy_( Source__made );

  SdaiProduct_definition_context * prod_def_context = ( SdaiProduct_definition_context * ) registry->ObjCreate( "PRODUCT_DEFINITION_CONTEXT" );
  instance_list->Append( ( SDAI_Application_instance * ) prod_def_context, completeSE );
  prod_def_context->name_( "''" );
  prod_def_context->life_cycle_stage_( "'design'" );
  prod_def_context->frame_of_reference_( app_context );

  SdaiProduct_definition * prod_def = ( SdaiProduct_definition * ) registry->ObjCreate( "PRODUCT_DEFINITION" );
  instance_list->Append( ( SDAI_Application_instance * ) prod_def, completeSE );
  prod_def->id_( "''" );
  prod_def->description_( "''" );
  prod_def->frame_of_reference_( prod_def_context );
  prod_def->formation_( prod_def_form );

  SdaiProduct_definition_shape * pshape = ( SdaiProduct_definition_shape * ) registry->ObjCreate( "PRODUCT_DEFINITION_SHAPE" );
  instance_list->Append( ( SDAI_Application_instance * ) pshape, completeSE );
  pshape->name_( "''" );
  pshape->description_( "'ProductShapeDescription'" );
  pshape->definition_( new SdaiCharacterized_definition( new SdaiCharacterized_product_definition( prod_def ) ) );

  SdaiShape_representation * shape_rep = ( SdaiShape_representation * ) registry->ObjCreate( "SHAPE_REPRESENTATION" );
  instance_list->Append( ( SDAI_Application_instance * ) shape_rep, completeSE );
  shape_rep->name_( "''" ); // Document?
  shape_rep->items_()->AddNode( new EntityNode( ( SDAI_Application_instance * ) orig_transform ) );
  shape_rep->context_of_items_( ( SdaiRepresentation_context * ) context );

  SdaiShape_definition_representation * shape_def_rep = ( SdaiShape_definition_representation * ) registry->ObjCreate( "SHAPE_DEFINITION_REPRESENTATION" );
  instance_list->Append( ( SDAI_Application_instance * ) shape_def_rep, completeSE );
  shape_def_rep->definition_( new SdaiRepresented_definition ( pshape ) );
  shape_def_rep->used_representation_( shape_rep );

  return prod_def;
}


typedef std::list<SDAI_Application_instance *> ai_list;


static void
find_all_typename_entity_with_id_from (InstMgr *instance_list, ai_list *ai_list, const char *typename_, int from_id)
{
  int search_index = 0;

  // Loop over the instances of SHAPE_DEFITION_REPRESENTATION in the file
  SDAI_Application_instance *ai;
  while (ENTITY_NULL != (ai = (SDAI_Application_instance *)
                               instance_list->GetApplication_instance (typename_, search_index)))
    {
      int id = ai->StepFileId ();

      if (id >= from_id)
        {
          ai_list->push_back (ai);

          std::cout << "Found a " << typename_ << " instance (#" << id << ") to fixup" << std::endl;
        }

      MgrNode * mnode = instance_list->FindFileId (id);
      search_index = instance_list->GetIndex (mnode) + 1;
    }
}

static void
find_all_complex_with_member_and_id_from (InstMgr *instance_list, ai_list *ai_list, const char *typename_, int from_id)
{
  MgrNode * node;
  SDAI_Application_instance * se;

  int count = instance_list->InstanceCount ();
  for (int j = 0; j < count; ++j)
    {
      node = instance_list->GetMgrNode (j);
      se = node->GetApplication_instance ();
      if (!se->IsComplex ())
        continue;

      int id = se->StepFileId ();
      if (id < from_id)
        continue;

      STEPcomplex *stepcomplex = ((STEPcomplex *)se)->head;
      while (stepcomplex)
        {
          if (!StrCmpIns (stepcomplex->EntityName (), typename_))
            {
              ai_list->push_back (stepcomplex);

              std::cout << "Found a complex instance (#" << stepcomplex->StepFileId () << ") with " << typename_ << " to fixup" << std::endl;
            }

          stepcomplex = stepcomplex->sc;
        }
    }
}


static void
fixup_2d_aggregate_in_b_spline_surface (Registry *registry, InstMgr *instance_list, int file_increment)
{
  const char *entityname = "b_spline_surface";
  ai_list to_fixup;

  find_all_typename_entity_with_id_from (instance_list, &to_fixup, entityname, file_increment);
  find_all_complex_with_member_and_id_from (instance_list, &to_fixup, entityname, file_increment);

  /* NB: We need to use late binding APIs to manipulate attributes, as we may be dealing with complex entities */
  /* XXX: TODO: FIXUP THESE ENTITIES */
}


static void
fixup_2d_aggregate_in_rectangular_composite_surface (Registry *registry, InstMgr *instance_list, int file_increment)
{
  const char *entityname = "rectangular_composite_surface";
  ai_list to_fixup;

  find_all_typename_entity_with_id_from (instance_list, &to_fixup, entityname, file_increment);
  find_all_complex_with_member_and_id_from (instance_list, &to_fixup, entityname, file_increment);

  /* XXX: TODO: FIXUP THESE ENTITIES */
}


static void
fixup_2d_aggregates (Registry *registry, InstMgr *instance_list, int file_increment)
{
  std::cout << "Checking and fixing up 2D aggregates (not fully implemented in SCL)" << std::endl;
  fixup_2d_aggregate_in_b_spline_surface (registry, instance_list, file_increment);
  fixup_2d_aggregate_in_rectangular_composite_surface (registry, instance_list, file_increment);
  /* NB: THE 2D AGGREGATE in the rational_b_spline_surface ENTITY IS OF REAL TYPE, SO DOESN'T NEED REPAIR */
}

SdaiProduct_definition *
append_model_from_file (Registry *registry,
                        InstMgr *instance_list,
                        const char *filename)
{
  int max_existing_file_id = instance_list->MaxFileId ();

  /* XXX: The following line is coppied from STEPfile.inline.cc, and we rely on it matching the algorithm there! */
//  int file_increment = ( int )( ( ceil( ( max_existing_file_id + 99.0 ) / 1000.0 ) + 1.0 ) * 1000.0 ); /* XXX: RELYING ON SCL NOT CHANGING */
//  std::cout << "INFO: Expecting a to add " << file_increment << " to entity names" << std::endl;

  STEPfile sfile = STEPfile (*registry, *instance_list, "", false);

  sfile.AppendExchangeFile (filename);

  Severity severity = sfile.Error().severity();
  if (severity != SEVERITY_NULL)
    {
      sfile.Error().PrintContents (std::cout);
      std::cout << "WARNING: Error reading from file '" << filename << "'" << std::endl;
//      return NULL;
#warning HANDLE OTHER ERRORS BETTER?
    }

  //fixup_2d_aggregates (registry, instance_list, file_increment);

  pd_list all_pd_list;
  pd_list pd_list;

  // Find all PRODUCT_DEFINITION entities with a SHAPE_DEFINITION_REPRESETNATION
  find_all_pd_with_sdr (instance_list, &all_pd_list);

  // Find and copy over any PRODUCT_DEFINITION in our list which have entity numbers from the append
  for (pd_list::iterator iter = all_pd_list.begin(); iter != all_pd_list.end(); iter++)
    if ((*iter)->StepFileId () > max_existing_file_id)
      pd_list.push_back (*iter);

  /*  Try to determine the root product */
  find_and_remove_child_pd (instance_list, &pd_list, "Next_assembly_usage_occurrence"); // Remove any PD which are children of another via NAUO
  find_and_remove_child_pd (instance_list, &pd_list, "Assembly_component_usage");       // Remove any PD which are children of another via ACU

#ifdef DEBUG_PRODUCT_DEFINITION_SEARCH
  std::cout << "Hopefully left with the root product definition" << std::endl;
  for (pd_list::iterator iter = pd_list.begin(); iter != pd_list.end(); iter++)
    std::cout << "Product definition list item #" << (*iter)->StepFileId () << std::endl;
  std::cout << std::endl;
#endif

  // If we didn't find a suitable PD, give up now
  if (pd_list.size() == 0)
    {
      std::cout << "ERROR: Did not find a PRODUCT_DEFINITION (with associated SHAPE_DEFINITION_REPRESENTATION)" << std::endl;
      return NULL;
    }

  if (pd_list.size() > 1)
    std::cout << "WARNING: Found more than one PRODUCT_DEFINITION that might be the root" << std::endl;

  // Use the first PD meeting the criterion. Hopefully there should just be one, but if not, we pick the first.
  return *pd_list.begin();
}


void
convert_model_to_assy_capable (Registry *registry,
                               InstMgr *instance_list,
                               SdaiProduct_definition *pd)
{
  SdaiShape_definition_representation *sdr = find_sdr_for_pd (instance_list, pd);
  SdaiShape_representation *sr = (SdaiShape_representation *)sdr->used_representation_ ();

  // If sr is an exact match for the step entity SHAPE_REPRESENTATION (not a subclass), return - we are already in the correct form
  if (strcmp (sr->EntityName (), "Shape_Representation") == 0)
    return;

  // sr must be a subclass of Shape Representation, not an exact match for "Shape_Representation
  // we need to adjust the shape representation structure to insert a SHAPE_REPRESENTATION, then
  // relate this to the original shape representation.

#ifdef DEBUG_PRODUCT_DEFINITION
  std::cout << "Going to shoe-horn this into an assembly compatible structure" << std::endl;
#endif

  SdaiAxis2_placement_3d *child_origin = find_axis2_placement_3d_in_sr (sr);
  if (child_origin == NULL)
    {
      std::cout << "WARNING: Could not find AXIS2_PLACEMENT_3D entity in SHAPE_REPRESENTATION - ABORTING CONVERSION TO ASSEMBLY CAPABLE" << std::endl;
      return;
    }

  SdaiShape_representation *new_sr = (SdaiShape_representation *) registry->ObjCreate ("SHAPE_REPRESENTATION");
  instance_list->Append ((SDAI_Application_instance * ) new_sr, completeSE);
  new_sr->name_ ( "''" ); // Document?
  new_sr->items_ ()->AddNode (new EntityNode ((SDAI_Application_instance *) child_origin));
  new_sr->context_of_items_ ((SdaiRepresentation_context *) sr->context_of_items_());

  // Replace the shape representation from the model by our new entity
  sdr->used_representation_ (new_sr);

  // NEED SHAPE_REPRESENTATION_RELATIONSHIP linking new_sr and sr
  SdaiShape_representation_relationship *new_srr = (SdaiShape_representation_relationship *) registry->ObjCreate ("SHAPE_REPRESENTATION_RELATIONSHIP");
  instance_list->Append ((SDAI_Application_instance * ) new_srr, completeSE);
  new_srr->name_ ( "'NONE'" );
  new_srr->description_ ( "'NONE'" );
  new_srr->rep_1_ (new_sr);
  new_srr->rep_2_ (sr);
}


void
assemble_instance_of_model (Registry *registry,
                            InstMgr *instance_list,
                            SdaiProduct_definition *parent_pd,
                            SdaiProduct_definition *child_pd,
                            SdaiAxis2_placement_3d *location_in_parent)
{
  SdaiShape_representation *parent_sr = find_sr_for_pd (instance_list, parent_pd);
  SdaiShape_representation *child_sr = find_sr_for_pd (instance_list, child_pd);
  SdaiAxis2_placement_3d *child_origin = find_axis2_placement_3d_in_sr (child_sr);

  parent_sr->items_ ()->AddNode (new EntityNode (location_in_parent));

  // NAUO
  char *nauo_id = strdup ("'NAUO1'"); // XXX: SHOULD BE UNIQUE!

  SdaiNext_assembly_usage_occurrence *nauo = (SdaiNext_assembly_usage_occurrence *) registry->ObjCreate ("NEXT_ASSEMBLY_USAGE_OCCURRENCE");
  instance_list->Append ((SDAI_Application_instance *)nauo, completeSE);
  nauo->description_ ("''");
  nauo->id_ (nauo_id);
  nauo->name_ ("''");
  nauo->relating_product_definition_ (parent_pd);
  nauo->related_product_definition_ (child_pd);
  free (nauo_id);

  // PDS
  SdaiProduct_definition_shape *pds = (SdaiProduct_definition_shape *) registry->ObjCreate ("PRODUCT_DEFINITION_SHAPE");
  instance_list->Append ((SDAI_Application_instance *) pds, completeSE);
  pds->name_ ("''");
  pds->description_ ("'ProductShapeDescription'");
  pds->definition_ (new SdaiCharacterized_definition (new SdaiCharacterized_product_definition (nauo)));

  // IDT
  SdaiItem_defined_transformation *idt = (SdaiItem_defined_transformation *) registry->ObjCreate ("ITEM_DEFINED_TRANSFORMATION");
  instance_list->Append ((SDAI_Application_instance *) idt, completeSE);
  idt->description_ ("''");
  idt->name_ ("''");
  idt->transform_item_1_ (location_in_parent); // Axis in the parent shape where the child origin should place
  idt->transform_item_2_ (child_origin);       // Child origin in the child shape

  // RRWT_SRR COMPLEX
  STEPcomplex *rrwt_srr = MakeRrwtSrr (registry, instance_list,
                                       "'NONE'",                             // RR   description
                                       "'NONE'",                             // RR   name
                                       (SdaiRepresentation_item *)parent_sr, // RR   rep_1,
                                       (SdaiRepresentation_item *)child_sr,  // RR   rep_2,
                                       new SdaiTransformation (idt));        // RRWT transformation_operator
  instance_list->Append ((SDAI_Application_instance *) rrwt_srr, completeSE);

  // CDSR
  SdaiContext_dependent_shape_representation *cdsr = (SdaiContext_dependent_shape_representation *) registry->ObjCreate ("CONTEXT_DEPENDENT_SHAPE_REPRESENTATION");
  instance_list->Append ((SDAI_Application_instance *) cdsr, completeSE);
  cdsr->representation_relation_ ((SdaiShape_representation_relationship *)rrwt_srr);
  cdsr->represented_product_relation_ (pds);
}


void
print_pd_debug (InstMgr *instance_list, SdaiProduct_definition *pd)
{
#ifdef DEBUG_PRODUCT_DEFINITION
  std::cout << "The product we are going to embed is called " << pd->formation_ ()->of_product_ ()->id_ ().c_str () << std::endl;

  SdaiShape_definition_representation *sdr = find_sdr_for_pd (instance_list, pd);

  SdaiShape_representation *sr = find_sr_for_pd (instance_list, pd);
  if (sr == NULL)
    {
      std::cout << "Could not find shape representation for the part!" << std::endl;
      return;
    }
  std::cout << "The shape representation (#" << sr->StepFileId () << ") for the product has type " << sr->EntityName ();
  std::cout << std::endl;
#endif
}


void
lazy_assemble (Registry *registry,
               InstMgr *instance_list,
               SdaiProduct_definition *parent_pd,
               const char *filename,
               const double &px, const double &py, const double & pz,
               const double &ax, const double &ay, const double & az,
               const double &rx, const double &ry, const double & rz)
{
  SdaiProduct_definition *model_pd = append_model_from_file (registry, instance_list, filename);
  if (model_pd == NULL)
    return;
  SdaiAxis2_placement_3d *child_location = MakeAxis (registry, instance_list, px, py, pz, ax, ay, az, rx, ry, rz);
  assemble_instance_of_model (registry, instance_list, parent_pd, model_pd, child_location);
}


int
main (void)
{
  // The registry contains information about types present in the current schema; SchemaInit is a function in the schema-specific SDAI library
  Registry * registry = new Registry( SchemaInit );

  // The InstMgr holds instances that have been created or that have been loaded from a file
  InstMgr * instance_list = new InstMgr();

  // Increment FileId so entities start at #1 instead of #0.
  instance_list->NextFileId();

#if 1
  SdaiProduct_definition *assembly_pd = create_parent_assembly (registry, instance_list);
#endif
  SdaiProduct_definition *model_pd;
  SdaiAxis2_placement_3d *child_location;

#if 0
  SdaiProduct_definition *assembly_pd;
  model_pd = append_model_from_file (registry, instance_list, "buggy.step");
  assembly_pd = create_parent_assembly (registry, instance_list);
  if (assembly_pd == NULL)
    {
      return -1;
    }
  if (model_pd != NULL)
    {
      child_location = MakeAxis (registry, instance_list, 0.0, 0.0, 0.0, // POINT
                                                          0.0, 0.0, 1.0,  // AXIS
                                                          1.0, 0.0, 0.0); // REF DIRECTION
      assemble_instance_of_model (registry, instance_list, assembly_pd, model_pd, child_location);
    }
#endif

#if 0
  model_pd = append_model_from_file (registry, instance_list, "assy.step");
  if (model_pd != NULL)
    {
      print_pd_debug (instance_list, model_pd);
      convert_model_to_assy_capable (registry, instance_list, model_pd);

      for (int i = -2; i <= 2; i++)
        {
          // Translation coordinate system.
          child_location = MakeAxis (registry, instance_list, i * 50.0, 0.0, 0.0, // POINT
                                                              0.0, 0.0, 1.0,  // AXIS
                                                              1.0, 0.0, 0.0); // REF DIRECTION
          assemble_instance_of_model (registry, instance_list, assembly_pd, model_pd, child_location);
        }
    }

  model_pd = append_model_from_file (registry, instance_list, "/home/pcjc2/source/stepcode/example/ap203min/build/bin/part.step");
  if (model_pd != NULL)
    {
      print_pd_debug (instance_list, model_pd);
      convert_model_to_assy_capable (registry, instance_list, model_pd);

      child_location = MakeAxis (registry, instance_list,
                                 0.0, 0.0, 100.0,
                                 0.0, 0.0, 1.0,
                                 1.0, 0.0, 0.0);

      assemble_instance_of_model (registry, instance_list, assembly_pd, model_pd, child_location);

      child_location = MakeAxis (registry, instance_list,
                                 0.0, 0.0, -100.0,
                                 0.0, 0.0, 1.0,
                                 1.0, 0.0, 0.0);
      assemble_instance_of_model (registry, instance_list, assembly_pd, model_pd, child_location);
    }
#endif

#if 1
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/APTOS_RPLJ-11CBEJ-FG_HDRV_Therminal_3-81mmP.step",                              000.0, 000.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/BGA121C65P11X11_800X800X143_JEDEC_MO-275-CCBE-1_Freescale_MAPBGA_121.step",     050.0, 000.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/CAPPRD500W60D1250H2000-V-000-300.step",                                         100.0, 000.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/CAPPRD750W80D1600H2500-HL-000-300-100.step",                                    150.0, 000.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/CAPRB1000W60L1000T400H1300-180_Vishay_H_Series.step",                         200.0, 000.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0); // CORRUPTED BY CONVERSION?
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/CAPRR1000W60L1300T400H900_EPCOS_B32921.step",                                   250.0, 000.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/CAPRR3750W100L4150T2800H4250_EPCOS_B32926.step",                                000.0, 050.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/CNC_Tech_3020-14-0100-00_IDC14_V.step",                                         050.0, 050.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/Degson_DG126-5.0-07P-14_Screw_Terminal.step",                                 100.0, 050.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0); // CORRUPTED BY CONVERSION?
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/DIPS762W51P254L2032H419Q16_DIP_SOCKET_16_.300.step",                            150.0, 050.0, 0.0, /**/  1.0, 0.0, 0.0, /**/ 0.0, 1.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/FCI_67996-440HLF_HDRV40W64P254_2X20_5080X508X838-584-381.step",                 200.0, 050.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/FCI_68000-102HLF_HDRV2W64P254_1X2_508X421X839-584-241.step",                    250.0, 050.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/HARWIN_M52-040000P1045_HDRV10W46P127_1X10_1270X500X777.step",                   000.0, 100.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/Harwin_M52-040000S0545_HDRV_2X5_127mmP_SM.step",                                050.0, 100.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/INDRD1143W102D1676H2134-300_Vishay_IHB-1.step",                                 100.0, 100.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/JEDEC_TO-262_5_LEADS_I2PAK-BL.step",                                            150.0, 100.0, 0.0, /**/  0.0, 0.0, -1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/JST_B7PS-VH(LF)(SN)_HDR_3.96mm_Pitch_RA.step",                                  200.0, 100.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/JST_B7P-VH(LF)(SN)_HDR_TH_3.96mm_pitch_V.step",                                 250.0, 100.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/Multicomp_2119R5-14G-301523_HDRRA14W40P127_2X7_889X150X340-300-230-200.step",   000.0, 150.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/Multicomp_2119R5-50G-301523_HDRRA50W40P127_2X25_3175X150X340-300-230-200.step", 050.0, 150.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/Multicomp_2206RPA-09G_HDRRA9W46P127_1X9_1143X254X127-285-200-287.step",         100.0, 150.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/MULTICOMP_2217R-02-WH2_HDR_KK_RA.step",                                         150.0, 150.0, 0.0, /**/  1.0, 0.0, 0.0, /**/ 0.0, 1.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/MULTICOMP_2217S-03-WH2_HDR_KK_V.step",                                          200.0, 150.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/OSCCC160X200X050-4_NDK_NX2016SA.step",                                          250.0, 150.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/PLCC127P1244X1244X457-28N_JEDEC_MO-047AB.step",                                 000.0, 200.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/QFN50P350X450X80-24_JEDEC_MO-220WFSD.step",                                   050.0, 200.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0); // CORRUPTED BY CONVERSION?
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/QFP80P1400X1400X120-52N_JEDEC_MS-026ADB.step",                                  100.0, 200.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/RESC1608X50N_EIA_0603_METRIC_1608_157X085X050.step",                            150.0, 200.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/SOD9959X265N_JEDEC_DO-215AB.step",                                              200.0, 200.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/SON40P300X300X80-15N_MAXIM_T1433-1.step",                                       250.0, 200.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/SOP65P780X200-22N_JEDEC_MO-150AF.step",                                         000.0, 250.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/TE_1-1634688-4_IDC14_V.step",                                                   050.0, 250.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/TE_1-1634689-0_IDC10_RA.step",                                                  100.0, 250.0, 0.0, /**/  -1.0, 0.0, 0.0, /**/ 0.0, 1.0, 0.0);
  lazy_assemble (registry, instance_list, assembly_pd, "pcb-3d/XTAL_RALTRON_HC-49U_3PIN-V.step",                                               150.0, 250.0, 0.0, /**/ 0.0, 0.0, 1.0, /**/ 1.0, 0.0, 0.0);
#endif

  write_ap214 (registry, instance_list, "outfile.step");

//  header_instances->DeleteInstances();
  instance_list->DeleteInstances();
  delete registry;
  delete instance_list;
}
