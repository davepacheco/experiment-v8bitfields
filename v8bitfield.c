/*
 * bitfield.c: print out V8-related bitfields
 */

#include <assert.h>
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define	EXIT_USAGE 2

typedef enum { B_FALSE, B_TRUE } boolean_t;

typedef struct {
	const char *vv_name;
	int	    vv_flags;
} v8_enumvalue_t;

typedef enum {
	V8_BITFIELD_RAW,
	V8_BITFIELD_ENUM,
	V8_BITFIELD_FLAGS
} v8_bitfield_type_t;

typedef struct {
	const char		*vb_name;
	v8_bitfield_type_t	vb_type;
	size_t 			vb_off;
	size_t 			vb_nbits;
	v8_enumvalue_t		vb_values[16];
} v8_bitfield_spec_t;

typedef struct {
	const char		*vv_name;
	v8_bitfield_spec_t	*vv_descs[32];
} v8_bitfield_value_t;

/*
 * The specific definitions of these bitfields comes from the version of V8
 * bundled with Node v0.10.24.  You can find these in src/property-details.h
 * inside V8.
 */
v8_bitfield_spec_t v8_010_property_type = {
	"PropertyType", V8_BITFIELD_ENUM, 0, 3, {
		{ "NORMAL",		0 },
		{ "FIELD",		1 },
		{ "CONSTANT",		2 },
		{ "CALLBACKS",		3 },
		{ "HANDLER",		4 },
		{ "INTERCEPTOR",	5 },
		{ "TRANSITION",		6 },
		{ "NONEXISTENT",	7 },
		{ NULL }
	},
};

v8_bitfield_spec_t v8_010_property_attr = {
	"PropertyAttributes", V8_BITFIELD_FLAGS, 3, 3, {
		{ "NONE",		0 },
		{ "READ_ONLY",		1 << 0 },
		{ "DONT_ENUM",		1 << 1 },
		{ "DONT_DELETE",	1 << 2 },
		{ "ABSENT",		16     },
		{ NULL }
	}
};

v8_bitfield_spec_t v8_010_property_deleted = {
	"DeletedField", V8_BITFIELD_FLAGS, 6, 1, {
		{ "DELETED",	1 },
		{ NULL }
	}
};

v8_bitfield_spec_t v8_010_property_dictstorage = {
	"DictionaryStorage", V8_BITFIELD_RAW, 7, 24
};

v8_bitfield_spec_t v8_010_property_descstorage = {
	"DescriptorStorage", V8_BITFIELD_RAW, 7, 11
};

v8_bitfield_spec_t v8_010_property_descptr = {
	"DescriptorPointer", V8_BITFIELD_RAW, 18, 11
};

v8_bitfield_value_t v8_010_property_details = {
	"PropertyDetails", {
		&v8_010_property_type,
		&v8_010_property_attr,
		&v8_010_property_deleted,
		&v8_010_property_dictstorage,
		&v8_010_property_descstorage,
		&v8_010_property_descptr,
		NULL
	}
};

/*
 * The following values come from the analogous places in the Node v0.12 source.
 */
v8_bitfield_spec_t v8_012_property_type = {
	"PropertyType", V8_BITFIELD_ENUM, 0, 3, {
		{ "NORMAL",		0 },
		{ "FIELD",		1 },
		{ "CONSTANT",		2 },
		{ "CALLBACKS",		3 },
		{ "HANDLER",		4 },
		{ "INTERCEPTOR",	5 },
		{ "NONEXISTENT",	6 },
		{ NULL }
	}
};

v8_bitfield_spec_t v8_012_property_attr = {
	"PropertyAttributes", V8_BITFIELD_FLAGS, 3, 3, {
		{ "NONE",		0      },
		{ "READ_ONLY",		1 << 0 },
		{ "DONT_ENUM",		1 << 1 },
		{ "DONT_DELETE",	1 << 2 },
		{ "STRING",		8      },
		{ "SYMBOLIC",		16     },
		{ "PRIVATE_SYMBOL",	32     },
		{ "ABSENT",		64     },
		{ NULL }
	}
};

v8_bitfield_spec_t v8_012_property_deleted = {
	"DeletedField", V8_BITFIELD_FLAGS, 6, 1, {
		{ "DELETED",	1 },
		{ NULL }
	}
};

v8_bitfield_spec_t v8_012_property_dictstorage = {
	"DictionaryStorage", V8_BITFIELD_RAW, 7, 24
};

v8_bitfield_spec_t v8_012_property_repr = {
	"Representation", V8_BITFIELD_ENUM, 6, 4, {
		{ "None",	0 },
		{ "Integer8",	1 },
		{ "UInteger8",	2 },
		{ "Integer16",	3 },
		{ "UInteger16",	4 },
		{ "Smi",	5 },
		{ "Integer32",	6 },
		{ "Double",	7 },
		{ "HeapObject",	8 },
		{ "Tagged",	9 },
		{ "External",  10 },
		{ NULL }
	}
};

v8_bitfield_spec_t v8_012_property_descptr = {
	"DescriptorPointer", V8_BITFIELD_RAW, 10, 10
};

v8_bitfield_spec_t v8_012_property_fieldindex = {
	"FieldIndex", V8_BITFIELD_RAW, 20, 10
};

v8_bitfield_value_t v8_012_property_details = {
	"PropertyDetails", {
		&v8_012_property_type,
		&v8_012_property_attr,
		&v8_012_property_deleted,
		&v8_012_property_dictstorage,
		&v8_012_property_repr,
		&v8_012_property_descptr,
		&v8_012_property_fieldindex,
		NULL
	}
};

static void v8bitfield_dumpcfg(v8_bitfield_value_t *);
static void v8bitfield_dumpcfg_one(v8_bitfield_spec_t *);
static void v8bitfield_dump_value(v8_bitfield_value_t *, unsigned long);
static void v8bitfield_dump_value_one(v8_bitfield_spec_t *, unsigned long);
static const char *v8bitfield_type_label(v8_bitfield_spec_t *);

int
main(int argc, char *argv[])
{
	unsigned long value;
	char *valstr;
	char *endptr;
	boolean_t opt_c;

	if (argc < 2 || argc > 3 ||
	    (argc == 3 && strcmp(argv[1], "-c") != 0)) {
		errx(EXIT_USAGE, "usage: %s [-c] VALUE", argv[0]);
	}

	if (argc == 3) {
		valstr = argv[2];
		opt_c = B_TRUE;
	} else {
		valstr = argv[1];
		opt_c = B_FALSE;
	}

	value = strtoul(valstr, &endptr, 0);
	if (*endptr != '\0') {
		errx(EXIT_USAGE, "non-numeric value: \"%s\"", valstr);
	}

	/* Interpret as an SMI before decoding fields. */
	value >>= 1;
	if (opt_c)
		v8bitfield_dumpcfg(&v8_012_property_details);
	v8bitfield_dump_value(&v8_012_property_details, value);
}

static void
v8bitfield_dumpcfg_one(v8_bitfield_spec_t *bp)
{
	int i;

	(void) printf("    %s: from bit %ld for %ld bits (%s)\n",
	    bp->vb_name, bp->vb_off, bp->vb_nbits, v8bitfield_type_label(bp));
	for (i = 0; bp->vb_values[i].vv_name != NULL; i++) {
		v8_enumvalue_t *vp = &bp->vb_values[i];
		(void) printf("    %20s = 0x%x\n", vp->vv_name, vp->vv_flags);
	}
}

static void
v8bitfield_dumpcfg(v8_bitfield_value_t *vp)
{
	int i;
	(void) printf("%s:\n", vp->vv_name);
	for (i = 0; vp->vv_descs[i] != NULL; i++) {
		v8bitfield_dumpcfg_one(vp->vv_descs[i]);
	}
}

static const char *
v8bitfield_type_label(v8_bitfield_spec_t *bp)
{
	switch (bp->vb_type) {
	case V8_BITFIELD_RAW:
		return ("raw value");

	case V8_BITFIELD_ENUM:
		return ("exclusive values");

	case V8_BITFIELD_FLAGS:
		return ("overlapping flags");

	default:
		return ("unknown");
	}
}

static void
v8bitfield_dump_value(v8_bitfield_value_t *vp, unsigned long value)
{
	int i;
	(void) printf("value 0x%lx as %s:\n", value, vp->vv_name);
	for (i = 0; vp->vv_descs[i] != NULL; i++) {
		v8bitfield_dump_value_one(vp->vv_descs[i], value);
	}
}

static void
v8bitfield_dump_value_one(v8_bitfield_spec_t *vp, unsigned long value)
{
	unsigned long mask, decoded;
	int i;

	mask = (1 << vp->vb_nbits) - 1;
	decoded = (value >> vp->vb_off) & mask;

	(void) printf("    %20s: ", vp->vb_name);
	if (vp->vb_type == V8_BITFIELD_RAW) {
		(void) printf("0x%lx\n", decoded);
		return;
	}

	for (i = 0; vp->vb_values[i].vv_name != NULL; i++) {
		if (vp->vb_type == V8_BITFIELD_ENUM) {
			if (decoded != vp->vb_values[i].vv_flags)
				continue;

			(void) printf("%s\n", vp->vb_values[i].vv_name);
			return;
		}

		assert(vp->vb_type == V8_BITFIELD_FLAGS);
		if ((decoded & vp->vb_values[i].vv_flags) == 0)
			continue;

		(void) printf("%s ", vp->vb_values[i].vv_name);
	}

	if (vp->vb_type == V8_BITFIELD_ENUM)
		(void) printf("UNKNOWN VALUE\n");
	else
		(void) printf("\n");
}
