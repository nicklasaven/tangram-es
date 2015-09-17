#pragma once

//Define global constants
#define R_EARTH 6378137.0

#include "tile/tileID.h"
#include "geom.h"

namespace Tangram {

enum class ProjectionType {
    mercator
};

class MapProjection {
protected:
    /* m_type: type of map projection: example: mercator*/
    ProjectionType m_type;
public:

    MapProjection(ProjectionType _type) : m_type(_type) {};

    /*
     * LonLat to ProjectionType-Meter
     * Arguments:
     *   _lonlat: glm::dvec2 having lon and lat info
     * Return value:
     *    meter (glm::dvec2).
     */
    virtual glm::dvec2 LonLatToMeters(const glm::dvec2 _lonLat) const = 0;

    /*
     * ProjectionType-Meters to Lon Lat
     *  Arguments:
     *    _meter: glm::dvec2 having the projection units in meters
     *  Return value:
     *    lonlat (glm::dvec2)
     */
    virtual glm::dvec2 MetersToLonLat(const glm::dvec2 _meters) const = 0;

    /*
     * Converts a pixel coordinate at a given zoom level of pyramid to projection meters
     * Screen pixels to Meters
     *  Arguments:
     *    _pix: pixels defined as glm::dvec2
     *    _zoom: zoom level to determine the projection-meters
     *  Return value:
     *    projection-meters (glm::dvec2)
     */
    virtual glm::dvec2 PixelsToMeters(const glm::dvec2 _pix, const int _zoom) const = 0;

    /*
     * Converts projection meters to pyramid pixel coordinates in given zoom level.
     * Meters to Screen Pixels
     * Arguments:
     *   _meters: projection-meters (glm::dvec2)
     *   _zoom: zoom level to determine pixels
     * Return Value:
     *   pixels (glm::dvec2)
     */
    virtual glm::dvec2 MetersToPixel(const glm::dvec2 _meters, const int _zoom) const = 0;

    /*
     * Returns a tile covering region in given pixel coordinates.
     * Argument:
     *   _pix: pixel
     * Return Value:
     *   Tile coordinates (x and y) glm::ivec2
     */
    virtual glm::ivec2 PixelsToTileXY(const glm::dvec2 _pix) const = 0;

    /*
     * Returns tile for given projection coordinates.
     *  Arguments:
     *    _meters: projection-meters (glm::dvec2)
     *    _zoom: zoom level for which tile coordinates need to be determined
     *  Return Value:
     *    Tile coordinates (x and y) (glm::ivec2)
     */
    virtual glm::ivec2 MetersToTileXY(const glm::dvec2 _meters, const int _zoom) const = 0;

    /*
     * Move the origin of pixel coordinates to top-left corner
     * Arguments:
     *   _pix: pixels
     *   _zoom: zoom level
     * Return Value:
     *   glm::dvec2 newPixels at top-left corner
     */
    virtual glm::dvec2 PixelsToRaster(const glm::dvec2 _pix, const int _zoom) const = 0;

    /*
     * Returns bounds of the given tile
     *  Arguments:
     *    _tileCoord: glm::ivec3 (x,y and zoom)
     *  Return value:
     *    bounds in projection-meters (BoundingBox)
     */
    virtual BoundingBox TileBounds(const TileID _tileCoord) const = 0;

    /*
     * bounds of space in lon lat
     *  Arguments:
     *    _tileCoord: glm::ivec3 (x,y and zoom)
     *  Return value:
     *    bounds in lon lat (BoundingBox)
     */
    virtual BoundingBox TileLonLatBounds(const TileID _tileCoord) const = 0;

    /*
     * Returns center of the given tile
     *  Arguments:
     *    _tileCoord: glm::ivec3 (x,y and zoom)
     *  Return value:
     *    center in projection-meters (glm::dvec2)
     *       x,y : position in projection meters
     */
    virtual glm::dvec2 TileCenter(const TileID _tileCoord) const = 0;

    /*
     * Returns the projection type of a given projection instance
     *   (example: ProjectionType::Mercator)
     */
    virtual ProjectionType GetMapProjectionType() const {return m_type;}

    virtual double HalfCircumference() const = 0;

    virtual ~MapProjection() {}
};

class MercatorProjection : public MapProjection {
    /*
     * Following define the boundary covered by this mercator projection
     */
    constexpr static double HALF_CIRCUMFERENCE = 1.0;
    constexpr static double CIRCUMFERENCE = 2.0;

public:
    /*
     * Constructor for MercatorProjection
     * _type: type of map projection, example ProjectionType::Mercator
     */
    MercatorProjection();

    virtual glm::dvec2 LonLatToMeters(const glm::dvec2 _lonLat) const override;
    virtual glm::dvec2 MetersToLonLat(const glm::dvec2 _meters) const override;
    virtual glm::dvec2 PixelsToMeters(const glm::dvec2 _pix, const int _zoom) const override;
    virtual glm::dvec2 MetersToPixel(const glm::dvec2 _meters, const int _zoom) const override;
    virtual glm::ivec2 PixelsToTileXY(const glm::dvec2 _pix) const override;
    virtual glm::ivec2 MetersToTileXY(const glm::dvec2 _meters, const int _zoom) const override;
    virtual glm::dvec2 PixelsToRaster(const glm::dvec2 _pix, const int _zoom) const override;
    virtual BoundingBox TileBounds(const TileID _tileCoord) const override;
    virtual BoundingBox TileLonLatBounds(const TileID _tileCoord) const override;
    virtual glm::dvec2 TileCenter(const TileID _tileCoord) const override;
    virtual ~MercatorProjection() {}
    virtual double HalfCircumference() const override { return HALF_CIRCUMFERENCE; };

};

}
