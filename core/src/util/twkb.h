#pragma once

#include "data/tileData.h"

#include <vector>


/*Maximum number of dimmensions that a twkb geoemtry 
can hold according to the specification*/
#define TWKB_IN_MAXCOORDS 4

/*twkb types*/
#define	POINTTYPE			1
#define	LINETYPE			2
#define	POLYGONTYPE			3
#define	MULTIPOINTTYPE		4
#define	MULTILINETYPE		5
#define	MULTIPOLYGONTYPE	6
#define	COLLECTIONTYPE		7

namespace Tangram {

class Tile;

namespace twkb {

/***************************************************************
			DECODING TWKB						*/
/*Holds a buffer with the twkb-data during read*/
typedef struct
{	
	uint8_t handled_buffer; /*Indicates if this program is resposible for freeing*/
	uint64_t BufOffsetFromBof;	//Only osed when reading from file
	uint8_t *start_pos;
	uint8_t *read_pos;
	uint8_t *end_pos;
	uint8_t *max_end_pos;
	
}TWKB_BUF;

typedef struct
{
	float bbox_min[TWKB_IN_MAXCOORDS];
	float bbox_max[TWKB_IN_MAXCOORDS];
}BBOX;


typedef struct
{
	uint8_t has_bbox;
	uint8_t has_size;
	uint8_t has_idlist;
	uint8_t has_z;
	uint8_t has_m;
	uint8_t is_empty;
	uint8_t type;
	
	/* Precision factors to convert ints to double */
	uint8_t n_decimals[TWKB_IN_MAXCOORDS];
	/* Precision factors to convert ints to double */
	double factors[TWKB_IN_MAXCOORDS];

	uint32_t ndims; /* Number of dimensions */
 /* An array to keep delta values from 4 dimensions */
	int64_t coords[TWKB_IN_MAXCOORDS];
	
	BBOX *bbox;
	size_t next_offset;
} TWKB_HEADER_INFO;

/* Used for passing the parse state between the parsing functions.*/
typedef struct
{
	TWKB_BUF *tb; /* Points to start of TWKB */
	//~ buffer_collection *rb;
	TWKB_HEADER_INFO *thi;
	
} TWKB_PARSE_STATE;


void extractPoint(twkb::TWKB_PARSE_STATE *ts, Point& _out, const Tile& _tile);

void extractLine(twkb::TWKB_PARSE_STATE *ts, Line& _out, const Tile& _tile);

void extractPoly(twkb::TWKB_PARSE_STATE *ts, Polygon& _out, const Tile& _tile);

void extractFeature(twkb::TWKB_PARSE_STATE *ts, Feature& _out, const Tile& _tile);

void extractLayer(int32_t _sourceId, TWKB_PARSE_STATE *ts, Layer& _out, const Tile& _tile);

int read_header (twkb::TWKB_PARSE_STATE *ts);

uint64_t varint_u64_read(twkb::TWKB_BUF *tb);
int64_t unzigzag64(uint64_t val);
uint64_t buffer_read_uvarint(twkb::TWKB_BUF *tb);
int64_t buffer_read_svarint(twkb::TWKB_BUF *tb);
uint8_t buffer_read_byte(TWKB_BUF *tb);
void buffer_jump_varint(twkb::TWKB_BUF *tb,int n);
}

}
