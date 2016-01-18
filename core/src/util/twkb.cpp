#include "twkb.h"

#include "platform.h"
#include "tile/tile.h"
#include "util/mapProjection.h"
#include "data/propertyItem.h"

namespace Tangram {


  void twkb::extractPoint(twkb::TWKB_PARSE_STATE *ts, Point& _out, const Tile& _tile) {

	  glm::dvec2 tmp;

	  /*X*/
	  ts->thi->coords[0] += twkb::buffer_read_svarint(ts->tb);
	  tmp.x = ts->thi->coords[0] / ts->thi->factors[0];
	  /*y*/
	  ts->thi->coords[1] += twkb::buffer_read_svarint(ts->tb);
	  tmp.x = ts->thi->coords[1] / ts->thi->factors[1];

	  _out.x = (tmp.x - _tile.getOrigin().x) * _tile.getInverseScale();
	  _out.y = (tmp.y - _tile.getOrigin().y) * _tile.getInverseScale();

  }



  void twkb::extractLine(twkb::TWKB_PARSE_STATE *ts, Line& _out, const Tile& _tile) {

	      int npoints = (int) twkb::buffer_read_uvarint(ts->tb);
	  
	  for (int i=0;i<npoints;i++)
	  {
		  _out.emplace_back();
		  twkb::extractPoint(ts, _out.back(), _tile);
	  }
  }



	  
	  
	  
  void twkb::extractPoly(twkb::TWKB_PARSE_STATE *ts, Polygon& _out, const Tile& _tile) {

	  int nrings = (int) twkb::buffer_read_uvarint(ts->tb);


	  for (int i=0; i<nrings; i++)
	  {
		  _out.emplace_back();
		  twkb::extractLine(ts, _out.back(), _tile);
// 		  int npoints = (int) twkb::buffer_read_uvarint(ts->tb);
// 		  for (int i=0;i<npoints;i++)
// 		  {
// 			  _out.emplace_back();
// 			  twkb::extractPoint(ts, _out.back(), _tile);
// 		  }
	  }
  }

	  
	  
  void twkb::extractFeature(twkb::TWKB_PARSE_STATE *ts,Feature& _out, const Tile& _tile) {

		  twkb::read_header (ts);
		  switch (ts->thi->type)
		  {
			  case POINTTYPE:
				  _out.geometryType = GeometryType::points;
				  _out.points.emplace_back();
				  twkb::extractPoint(ts, _out.points.back(), _tile);
				  break;
			  case LINETYPE:
				  _out.geometryType = GeometryType::lines;
				  _out.lines.emplace_back();
				  twkb::extractLine(ts, _out.lines.back(), _tile);
				  break;
			  case POLYGONTYPE:
				  _out.geometryType = GeometryType::polygons;
				  _out.polygons.emplace_back();
				  twkb::extractPoly(ts, _out.polygons.back(), _tile);
				  break;
			  case MULTIPOINTTYPE:
			  case MULTILINETYPE:
			  case MULTIPOLYGONTYPE:
			  case COLLECTIONTYPE:
				  //~ return decode_multi(&ts);
				  break;
		  }

  }

  void twkb::extractLayer(int32_t _sourceId, TWKB_PARSE_STATE *ts, Layer& _out, const Tile& _tile) 
  {
	  _out.features.emplace_back(_sourceId);
	  twkb::extractFeature(ts, _out.features.back(), _tile);
   }

  
  
 
 
 
int twkb::read_header(twkb::TWKB_PARSE_STATE *ts)
{
    uint8_t flag;
    int8_t precision;
    uint8_t has_ext_dims;
    ts->thi->ndims=2;
    uint i;
    /*Here comes a byte containing type info and precission*/
    flag = twkb::buffer_read_byte(ts->tb);
    ts->thi->type=flag&0x0F;
    precision=twkb::unzigzag64((flag&0xF0)>>4);
    ts->thi->factors[0]=ts->thi->factors[1]= pow(10, (double)precision);
    ts->thi->n_decimals[0]=ts->thi->n_decimals[1]= precision>0?precision:0; /*We save number of decimals too, to get it right in text based formats in a simple way*/

    //Flags for options

    flag = twkb::buffer_read_byte(ts->tb);
    ts->thi->has_bbox   =  flag & 0x01;
    ts->thi->has_size   = (flag & 0x02) >> 1;
    ts->thi->has_idlist = (flag & 0x04) >> 2;
    has_ext_dims = (flag & 0x08) >> 3;

    if ( has_ext_dims )
    {
        flag = twkb::buffer_read_byte(ts->tb);

	ts->thi->has_z    = (flag & 0x01);
	ts->thi->has_m    = (flag & 0x02) >> 1;
        /* If Z*/	    
        if(ts->thi->has_z)
        {
		ts->thi->ndims++;
		precision = (flag & 0x1C) >> 2;
		ts->thi->factors[2]= pow(10, (double)precision);
		ts->thi->n_decimals[2]=precision>0?precision:0; 
        }

        /* If M*/
        if(ts->thi->has_m)
        {
            ts->thi->ndims++;
            precision = (flag & 0xE0) >> 5;
            ts->thi->factors[2+ts->thi->has_z]= pow(10, (double)precision);
		ts->thi->n_decimals[2+ts->thi->has_z]=precision>0?precision:0; 
        }
    }

    if(ts->thi->has_size)
    {
		/*We need to first read the value and then add the position of the cursor in the file.
		The size we get from the twkb-file is the size from after the size-info to the end of the twkb*/
	    ts->thi->next_offset = twkb::buffer_read_uvarint(ts->tb);
	  //  ts->thi->next_offset+= twkb::getReadPos(ts->tb);
    }


    if(ts->thi->has_bbox)
    {
	for (i=0;i<ts->thi->ndims;i++)
	{
		ts->thi->bbox->bbox_min[i]=twkb::buffer_read_svarint(ts->tb)/ts->thi->factors[i];
		ts->thi->bbox->bbox_max[i]=twkb::buffer_read_svarint(ts->tb)/ts->thi->factors[i] + ts->thi->bbox->bbox_min[i];
	}	    	    
    }

    return 0;
}


/**
Reads an unsigned varInt value
*/
uint64_t twkb::varint_u64_read(twkb::TWKB_BUF *tb)
{
	//~ printf("buffer to read = %d\n",tb->end_pos - tb->read_pos);
    uint64_t nVal = 0;
    int nShift = 0;
    uint8_t nByte;	
	
    /* Check so we don't read beyond the twkb 
	and if we do; try reading more from file 
	if there is one*/
    while( tb->read_pos < tb->end_pos) //  || !readmore(tb) )
    {
	nByte = *(tb->read_pos);
        /* Hibit is set, so this isn't the last byte */
        if (nByte & 0x80)
        {
            /* We get here when there is more to read in the input varInt */
            /* Here we take the least significant 7 bits of the read */
            /* byte and put it in the most significant place in the result variable. */
            nVal |= ((uint64_t)(nByte & 0x7f)) << nShift;
            /* move the "cursor" of the input buffer step (8 bits) */
            (tb->read_pos)++;
            /* move the cursor in the resulting variable (7 bits) */
            nShift += 7;
        }
        else
        {
            /* move the "cursor" one step */
            (tb->read_pos)++;
            return nVal | ((uint64_t)nByte << nShift);
        }
    }
//    fprintf(stderr,"TWKB-buffer seems to be corrupt. We have read beyond the buffer last byte is %p\n",tb->read_pos);
   exit(EXIT_FAILURE);
}

/**
makes the unsigned value signed (if the input vas signed)
*/
int64_t
twkb::unzigzag64(uint64_t val)
{
    if ( val & 0x01 )
        return -1 * (int64_t)((val+1) >> 1);
    else
        return (int64_t)(val >> 1);
}



/**
Just a wrapper. Have no function now
It is the function exposed for reading unsigned varint
*/
uint64_t
twkb::buffer_read_uvarint(twkb::TWKB_BUF *tb)
{
    return varint_u64_read(tb);
}

/**
Just a wrapper. Have no function now
It is the function exposed for reading signed varint
*/
int64_t
twkb::buffer_read_svarint(twkb::TWKB_BUF *tb)
{
    uint64_t val,uval ;
    uval= varint_u64_read(tb);
    val =  twkb::unzigzag64(uval);

    return val;
}

/**
Read 1 byte from the twkb-buffer
*/
uint8_t
twkb::buffer_read_byte(TWKB_BUF *tb)
{
// 	if(tb->end_pos - tb->read_pos < 1)
// 	{
// 		readmore(tb);
// 	}
	uint8_t r= *(tb->read_pos++);

	return r;
	
}

/**
Jump n VarInts in the buffer.
A little more efficient than reading them properly
*/
void
twkb::buffer_jump_varint(twkb::TWKB_BUF *tb,int n)
{
    int i;
    for (i=0; i<n; i++)
    {
         do 
 	{
// 		if(tb->read_pos == tb->end_pos)
//			readmore(tb);
	}while(*(tb->read_pos++) & 0x80) ;
    }
    return;
}



}
