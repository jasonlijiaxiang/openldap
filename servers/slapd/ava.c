/* $OpenLDAP$ */
/*
 * Copyright 1998-2000 The OpenLDAP Foundation, All Rights Reserved.
 * COPYING RESTRICTIONS APPLY, see COPYRIGHT file
 */
/* ava.c - routines for dealing with attribute value assertions */

#include "portable.h"

#include <stdio.h>

#include <ac/string.h>
#include <ac/socket.h>

#include "slap.h"

#ifdef SLAPD_SCHEMA_NOT_COMPAT

void
ava_free(
    AttributeAssertion *ava,
    int	freeit
)
{
	ad_free( ava->aa_desc, 1 );
	ber_bvfree( ava->aa_value );
	if ( freeit ) {
		ch_free( (char *) ava );
	}
}

int
get_ava(
    BerElement	*ber,
    AttributeAssertion	**ava,
	unsigned usage,
	const char **text
)
{
	int rc;
	struct berval type, value, *nvalue;
	AttributeAssertion *aa;

	rc = ber_scanf( ber, "{oo}", &type, &value );

	if( rc == LBER_ERROR ) {
		Debug( LDAP_DEBUG_ANY, "  get_ava ber_scanf\n", 0, 0, 0 );
		*text = "Error decoding attribute value assertion";
		return SLAPD_DISCONNECT;
	}

	aa = ch_malloc( sizeof( AttributeAssertion ) );
	aa->aa_desc = NULL;
	aa->aa_value = NULL;

	rc = slap_bv2ad( &type, &aa->aa_desc, text );

	if( rc != LDAP_SUCCESS ) {
		ch_free( type.bv_val );
		ch_free( value.bv_val );
		ch_free( aa );
		return rc;
	}

	rc = value_normalize( aa->aa_desc, usage, &value, &nvalue, text );
	ch_free( value.bv_val );

	if( rc != LDAP_SUCCESS ) {
		ch_free( type.bv_val );
		ad_free( aa->aa_desc, 1 );
		ch_free( aa );
		return rc;
	}

	aa->aa_value = nvalue;
	*ava = aa;

	return LDAP_SUCCESS;
}

#else

void
ava_free(
    Ava	*ava,
    int	freeit
)
{
	ch_free( (char *) ava->ava_type );
	ch_free( (char *) ava->ava_value.bv_val );
	if ( freeit ) {
		ch_free( (char *) ava );
	}
}

int
get_ava(
    BerElement	*ber,
    Ava		*ava,
	const char **text
)
{
	if ( ber_scanf( ber, "{ao}", &ava->ava_type, &ava->ava_value )
	    == LBER_ERROR ) {
		Debug( LDAP_DEBUG_ANY, "  get_ava ber_scanf\n", 0, 0, 0 );
		*text = "Error decoding attribute value assertion";
		return SLAPD_DISCONNECT;
	}

	attr_normalize( ava->ava_type );
	value_normalize( ava->ava_value.bv_val, attr_syntax( ava->ava_type ) );

	return LDAP_SUCCESS;
}

#endif
