#include "STEPaggrGeneric.h"
#include "typeDescriptor.h"
#include <sstream>

/** \file STEPaggrGeneric.cc
 * implement classes GenericAggregate, GenericAggrNode
 */

GenericAggregate::GenericAggregate() {
}

GenericAggregate::~GenericAggregate() {
}

SingleLinkNode * GenericAggregate::NewNode() {
    return new GenericAggrNode();
}

STEPaggregate & GenericAggregate::ShallowCopy( const STEPaggregate & a ) {
    Empty();

    SingleLinkNode * next = a.GetHead();
    SingleLinkNode * copy;

    while( next ) {
        copy = new GenericAggrNode( *( GenericAggrNode * )next );
        AddNode( copy );
        next = next->NextNode();
    }
    if( head ) {
        _null = 0;
    } else {
        _null = 1;
    }
    return *this;

}

/* XXX: GenericAggregate::ReadValue is almost exact copy of the STEPaggregate::ReadValue function.
 *
 * The only differences are:
 *
 *  -    STEPNode * item = 0;
 *  +    GenericAggrNode * item = 0;
 *
 *          if( exchangeFileFormat ) {
 *  -           item->STEPread( in, &errdesc );
 *  +           item->STEPread( in, &errdesc, elem_type, insts, addFileId );
 *          } else {
 *  -           item->StrToVal( in, &errdesc );
 *  +           item->StrToVal( in, &errdesc, elem_type, insts, addFileId );
 *          }
 *
 * Can we change the signature of STEPnode::STEPread and STEPnode::StrToVal
 * to take an elem_type, insts and addFileId field instead of doing this?
 */

/// if exchangeFileFormat == 1 then paren delims are required.

Severity GenericAggregate::ReadValue( istream & in, ErrorDescriptor * err,
                                      const TypeDescriptor * elem_type, InstMgrBase * insts,
                                      int addFileId, int assignVal, int exchangeFileFormat,
                                      const char * ) {
    (void) insts; //not used in ReadValue() for this class
    (void) addFileId; //not used in ReadValue() for this class

    ErrorDescriptor errdesc;
    char errmsg[BUFSIZ];
    int value_cnt = 0;
    std::string buf;

    if( assignVal ) {
        Empty();    // read new values and discard existing ones
    }

    char c;

    in >> ws; // skip white space

    c = in.peek(); // does not advance input

    if( in.eof() || c == '$' ) {
        _null = 1;
        err->GreaterSeverity( SEVERITY_INCOMPLETE );
        return SEVERITY_INCOMPLETE;
    }

    if( c == '(' ) {
        in.get( c );
    } else if( exchangeFileFormat ) {
        // error did not find opening delim
        // cannot recover so give up and let STEPattribute recover
        err->GreaterSeverity( SEVERITY_INPUT_ERROR );
        return SEVERITY_INPUT_ERROR;
    } else if( !in.good() ) {
        // this should actually have been caught by skipping white space above
        err->GreaterSeverity( SEVERITY_INCOMPLETE );
        return SEVERITY_INCOMPLETE;
    }

    GenericAggrNode * item = 0;

    in >> ws;
    // take a peek to see if there are any elements before committing to an
    // element
    c = in.peek(); // does not advance input
    if( c == ')' ) {
        in.get( c );
    }
    // if not assigning values only need one node. So only one node is created.
    // It is used to read the values
    else if( !assignVal ) {
        item = ( GenericAggrNode * )NewNode();
    }

    // ')' is the end of the aggregate
    while( in.good() && ( c != ')' ) ) {
        value_cnt++;
        if( assignVal ) { // create a new node each time through the loop
            item = ( GenericAggrNode * )NewNode();
        }

        errdesc.ClearErrorMsg();

        if( exchangeFileFormat ) {
            item->STEPread( in, &errdesc, elem_type, insts, addFileId );
        } else {
            item->StrToVal( in, &errdesc, elem_type, insts, addFileId );
        }

        elem_type->AttrTypeName( buf );
        // read up to the next delimiter and set errors if garbage is
        // found before specified delims (i.e. comma and quote)
        CheckRemainingInput( in, &errdesc, buf, ",)" );

        if( errdesc.severity() < SEVERITY_INCOMPLETE ) {
            sprintf( errmsg, "  index:  %d\n", value_cnt );
            errdesc.PrependToDetailMsg( errmsg );
            err->AppendFromErrorArg( &errdesc );
        }
        if( assignVal ) { // pass the node to STEPaggregate
            AddNode( item );
        }

        in >> ws; // skip white space (although should already be skipped)
        in.get( c ); // read delim

        // CheckRemainingInput should have left the input right at the delim
        // so that it would be read in in.get() above.  Since it did not find
        // the delim this does not know how to find it either!
        if( ( c != ',' ) && ( c != ')' ) ) {
            // cannot recover so give up and let STEPattribute recover
            err->GreaterSeverity( SEVERITY_INPUT_ERROR );
            return SEVERITY_INPUT_ERROR;
        }
    }
    if( c == ')' ) {
        _null = 0;
    } else { // expectation for end paren delim has not been met
        err->GreaterSeverity( SEVERITY_INPUT_ERROR );
        err->AppendToUserMsg( "Missing close paren for aggregate value" );
        return SEVERITY_INPUT_ERROR;
    }
    return err->severity();
}

GenericAggrNode::GenericAggrNode( const char * str ) {
    value = str;
}

GenericAggrNode::GenericAggrNode( GenericAggrNode & gan ) {
    value = gan.value;
}

GenericAggrNode::GenericAggrNode() {
}

GenericAggrNode::~GenericAggrNode() {
}

SingleLinkNode * GenericAggrNode::NewNode() {
    return new GenericAggrNode();
}

Severity GenericAggrNode::StrToVal( const char * s, ErrorDescriptor * err,
                                    const TypeDescriptor * elem_type,
                                    InstMgrBase * insts, int addFileId ) {
    return value.STEPread( s, err, elem_type, insts, addFileId );
}

//TODO
Severity GenericAggrNode::StrToVal( istream & in, ErrorDescriptor * err,
                                    const TypeDescriptor * elem_type,
                                    InstMgrBase * insts, int addFileId ) {
    return value.STEPread( in, err, elem_type, insts, addFileId );
}

Severity GenericAggrNode::STEPread( const char * s, ErrorDescriptor * err,
                                    const TypeDescriptor * elem_type,
                                    InstMgrBase * insts, int addFileId ) {
    istringstream in( ( char * ) s );
    return value.STEPread( in, err, elem_type, insts, addFileId );
}

Severity GenericAggrNode::STEPread( istream & in, ErrorDescriptor * err,
                                    const TypeDescriptor * elem_type,
                                    InstMgrBase * insts, int addFileId ) {
    return value.STEPread( in, err, elem_type, insts, addFileId );
}

const char * GenericAggrNode::asStr( std::string & s ) {
    s.clear();
    value.asStr( s );
    return const_cast<char *>( s.c_str() );
}

const char * GenericAggrNode::STEPwrite( std::string & s, const char * currSch ) {
    (void) currSch; //unused
    return value.STEPwrite( s );
}

void GenericAggrNode::STEPwrite( ostream & out ) {
    value.STEPwrite( out );
}
