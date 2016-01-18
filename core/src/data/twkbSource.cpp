
#include "twkbSource.h"

#include "tileData.h"
#include "tile/tile.h"
#include "tile/tileID.h"
#include "tile/tileTask.h"
#include "platform.h"


namespace Tangram {

TWKBSource::TWKBSource(const std::string& _name, const std::string& _urlTemplate) :
    DataSource(_name, _urlTemplate) {
      type=1;
}


int getTileId(int z, int tileX,int tileY)
{
	int i,min=0;	
	
	for (i=MAX_ZOOM_LEVELS-1;i>=0;i--)
	{
		min=min | ((1&(tileX>>i))<<2*i);
		min=min | ((1&(tileY>>i))<<(2*i+1));
	}
	
	return min << z*2;
}


int get_blob(twkb::TWKB_BUF *tb,sqlite3_stmt *res, int icol)
{
	/*twkb-buffer*/
	uint8_t *buf;	
	size_t buf_len;
	const sqlite3_blob *db_blob;	
	
		db_blob = (sqlite3_blob*) sqlite3_column_blob(res, icol);
	
		buf_len = sqlite3_column_bytes(res, icol);
		buf = (uint8_t*) malloc(buf_len);
		memcpy(buf, db_blob,buf_len);
		
	    
	    
		tb->start_pos = tb->read_pos=buf;
		tb->end_pos=buf+buf_len;
			
		return 0;
		
	
}


      

//std::shared_ptr<TileData> TWKBSource::parse(const Tangram::TileTask& _task, const Tangram::MapProjection& _projection) 	 {
 std::shared_ptr<Tangram::TileData> parse(const Tangram::TileTask& _task,const Tangram::MapProjection& _projection) {

//std::shared_ptr<TileData> TWKBSource::parse(const TileTask& _task, const MapProjection& _projection)  {

    std::shared_ptr<Tangram::TileData> tileData = std::make_shared<TileData>();
    twkb::TWKB_PARSE_STATE ts;
    twkb::TWKB_BUF tb;

    /*Sqlite*/
    sqlite3 *db;
    sqlite3_stmt *res;
    char *err_msg = 0;
    int tileId_min;
    int tileId_max;
    TileID tileID = _task.tileId();
    int tileX = tileID.x;
    int tileY = tileID.y;
    int tileZ = tileID.z;
    /*14 is just hardcoded test-number. It is the zoom-level of the layer and z is how many steps up or down the requested tile is.
    * TODO get layer Z-value from konfig
    * TODO handle smaller zoom-levels than layer zoom-level*/
    int z=14-tileZ;	

    int rc = sqlite3_open("/home/nicklas/test.sqlite", &db);

    if (rc != SQLITE_OK) 
    {		
      fprintf(stderr, "Cannot open database: %s\n", 
      sqlite3_errmsg(db));
      sqlite3_close(db);		
      return NULL;
    }




    tileId_min = getTileId(z, tileX,tileY);

    tileId_max = tileId_min +  (1<<(2*z))-1;

    char *sqltxt = "SELECT twkb,id FROM veger where tileid>=?1 and tileid <= ?2";
    rc = sqlite3_prepare_v2(db, sqltxt, -1, &res, 0);


    sqlite3_bind_int(res, 1, tileId_min);
    sqlite3_bind_int(res, 2, tileId_max);
      
		

    if (rc != SQLITE_OK) {		
    fprintf(stderr, "Cannot open database: %s\n", 
	    sqlite3_errmsg(db));
    sqlite3_close(db);		
    return NULL;
    }
      //  char *sql = (char *) "select twkb from admingrense;";
	    
    //   rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);
	//rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    
    if (rc != SQLITE_OK ) {
	
	fprintf(stderr, "Failed to select data\n");
	fprintf(stderr, "SQL error: %s\n", err_msg);

	sqlite3_free(err_msg);
	sqlite3_close(db);
	
	return NULL;
  } 
	    
	    
	    
	while (sqlite3_step(res)==SQLITE_ROW)
	{
		if(get_blob(&tb,res,0))
		{
			fprintf(stderr, "Failed to select data\n");

			sqlite3_close(db);
			return NULL;
		}			
		
		ts.tb=&tb;

		
		while (ts.tb->read_pos<ts.tb->end_pos)
		{
			tileData->layers.emplace_back(std::string("test"));
//			twkb::extractLayer(m_id, &ts, tileData->layers.back(), _tile);				
		}

		    free(tb.start_pos);
		    //~ n++;
	}//while(!sqlite3_blob_reopen(db_res, n));
	
	sqlite3_close(db);


    return tileData;

}

}
