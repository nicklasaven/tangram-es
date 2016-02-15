#pragma once
#include "util/twkb.h"
#include "dataSource.h"

#include "sqlite3.h"

#define MAX_ZOOM_LEVELS 16



namespace Tangram {

class TWKBSource : public DataSource {


protected:

 //   virtual std::shared_ptr<TileData> parse(const Tile& _tile, std::vector<char>& _rawData) const override;
    virtual std::shared_ptr<Tangram::TileData> parse(const Tangram::TileTask& _task,const Tangram::MapProjection& _projection) const override;
	
    int get_blob(twkb::TWKB_BUF *tb,sqlite3_stmt *res, int icol) const;
    int getTileId(int z, int tileX,int tileY) const;

public:

    TWKBSource(const std::string& _name, const std::string& _urlTemplate, int32_t _maxZoom);
    
};

}
