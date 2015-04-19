#ifndef STEPAGGRGENERIC_H
#define STEPAGGRGENERIC_H

#include "STEPaggregate.h"
#include <sc_export.h>

/** \file STEPaggrGeneric.h
 * classes GenericAggregate, GenericAggrNode
 */

/**
 * This class supports LIST OF:
 *    SELECT_TYPE, BINARY_TYPE, GENERIC_TYPE, ENUM_TYPE, UNKNOWN_TYPE type
 *  FIXME this class, as well as SelectAggregate, for SELECTs?!
 */
class SC_CORE_EXPORT GenericAggregate  :  public STEPaggregate {
public:
#if 1 /* Could we avoid adding this by extending the STEPnode instead */
    virtual Severity ReadValue( istream & in, ErrorDescriptor * err,
                                const TypeDescriptor * elem_type,
                                InstMgrBase * insts, int addFileId = 0,
                                int assignVal = 1, int ExchangeFileFormat = 1,
                                const char * currSch = 0 );
#endif

    virtual SingleLinkNode * NewNode();
    virtual STEPaggregate & ShallowCopy( const STEPaggregate & );

    GenericAggregate();
    virtual ~GenericAggregate();
};
typedef        GenericAggregate *   GenericAggregateH;
typedef        GenericAggregate *   GenericAggregate_ptr;
typedef  const GenericAggregate *   GenericAggregate_ptr_c;
typedef        GenericAggregate_ptr GenericAggregate_var;

/**
 * This class is for the Nodes of GenericAggregates
 */
class SC_CORE_EXPORT GenericAggrNode  : public STEPnode {
public:
    SCLundefined value;
    //  INPUT
    virtual Severity StrToVal( const char * s, ErrorDescriptor * err,
                               const TypeDescriptor * elem_type,
                               InstMgrBase * insts, int addFileId = 0 );
    virtual Severity StrToVal( istream & in, ErrorDescriptor * err,
                               const TypeDescriptor * elem_type,
                               InstMgrBase * insts, int addFileId = 0 );

    virtual Severity STEPread( const char * s, ErrorDescriptor * err,
                               const TypeDescriptor * elem_type,
                               InstMgrBase * insts, int addFileId = 0 );
    virtual Severity STEPread( istream & in, ErrorDescriptor * err,
                               const TypeDescriptor * elem_type,
                               InstMgrBase * insts, int addFileId = 0 );

    //  OUTPUT
    virtual const char * asStr( std::string & s );
    virtual const char * STEPwrite( std::string & s, const char * = 0 );
    virtual void    STEPwrite( ostream & out = cout );

    //  CONSTRUCTORS
    GenericAggrNode( const char * str );
    GenericAggrNode( GenericAggrNode & gan );
    GenericAggrNode();
    ~GenericAggrNode();

    virtual SingleLinkNode   *  NewNode();
};


#endif //STEPAGGRGENERIC_H
