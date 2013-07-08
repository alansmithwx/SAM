#include <wx/datstrm.h>
#include "object.h"


static ObjectCollection g_objectTypes;

void ObjectTypes::Register( Object *dummy )
{
	if ( !dummy ) return;
	ObjectCollection::iterator it = g_objectTypes.find( dummy->GetTypeName() );
	if ( it != g_objectTypes.end() )
	{
		delete it->second;
		it->second = dummy;
	}
	else
		g_objectTypes[ dummy->GetTypeName() ] = dummy;
}

Object *ObjectTypes::Create( const wxString &type )
{
	if ( Object *o = g_objectTypes.Lookup( type ) )
		return o->Duplicate();
	else
		return 0;
}

wxArrayString ObjectTypes::AllTypes()
{
	return g_objectTypes.GetNames();
}

ObjectCollection::ObjectCollection()
{
}

ObjectCollection::~ObjectCollection()
{
	for ( iterator it = begin();
		it != end();
		++it )
		delete it->second;
}

void ObjectCollection::Clear()
{
	for ( iterator it = begin();
		it != end();
		++it )
		delete it->second;

	clear();
}

void ObjectCollection::Add( const wxString &name, Object *obj )
{
	iterator it = find( name );
	if ( it != end() )
	{
		delete it->second;
		it->second = obj;		
	}
	else
		(*this)[name] = obj;
}

bool ObjectCollection::Delete( const wxString &name )
{
	iterator it = find( name );
	if ( it != end() )
	{
		delete it->second;
		erase( it );
		return true;
	}

	return false;
}

Object *ObjectCollection::Lookup( const wxString &name )
{
	iterator it = find( name );
	if ( it != end() )
		return it->second;
	else
		return 0;
}

wxArrayString ObjectCollection::GetNames()
{
	wxArrayString list;
	for ( iterator it = begin();
		it != end();
		++it )
		list.Add( it->first );
	return list;
}
	
void ObjectCollection::Write( wxOutputStream &output )
{
	wxArrayString list;
	wxDataOutputStream out(output);

	out.Write16( 0x2f ); // identifier code
	out.Write16( 1 ); // data format version
	
	out.Write32( size() );
	wxArrayString objs = GetNames();
	objs.Sort();
	for ( size_t i=0;i<objs.Count();i++ )
	{
		Object *obj = Lookup( objs[i] );
		assert( obj != 0 );

		out.WriteString( objs[i] );
		out.WriteString( obj->GetTypeName() );
		obj->Write( output );
	}

	out.Write16( 0x2f ); // identifier code to finish
}

bool ObjectCollection::Read( wxInputStream &input )
{
	wxDataInputStream in( input );

	wxUint16 code = in.Read16();
	wxUint16 ver = in.Read16();

	size_t count = in.Read32();
	for ( size_t i=0;i<count;i++ )
	{
		wxString name = in.ReadString();
		wxString type = in.ReadString();

		Object *obj = ObjectTypes::Create( type );
		if ( obj != 0 
			&& obj->Read( input ) )
		{
			Add( name, obj );
		}
	}

	return ( in.Read16() == code );
}


