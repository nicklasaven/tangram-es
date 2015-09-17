#include "polygonStyle.h"

#include "tangram.h"
#include "util/builders.h"
#include "gl/shaderProgram.h"
#include "tile/tile.h"
#include "util/mapProjection.h"

#include <cmath>

namespace Tangram {

PolygonStyle::PolygonStyle(std::string _name, Blending _blendMode, GLenum _drawMode) : Style(_name, _blendMode, _drawMode) {
}

void PolygonStyle::constructVertexLayout() {

    // TODO: Ideally this would be in the same location as the struct that it basically describes
    m_vertexLayout = std::shared_ptr<VertexLayout>(new VertexLayout({
        {"a_position", 3, GL_FLOAT, false, 0},
        {"a_normal", 3, GL_FLOAT, false, 0},
        {"a_texcoord", 2, GL_FLOAT, false, 0},
        {"a_color", 4, GL_UNSIGNED_BYTE, true, 0},
        {"a_layer", 1, GL_FLOAT, false, 0}
    }));

}

void PolygonStyle::constructShaderProgram() {

    std::string vertShaderSrcStr = stringFromResource("polygon.vs");
    std::string fragShaderSrcStr = stringFromResource("polygon.fs");

    m_shaderProgram->setSourceStrings(fragShaderSrcStr, vertShaderSrcStr);
}

PolygonStyle::Parameters PolygonStyle::parseRule(const DrawRule& _rule) const {
    Parameters p;
    _rule.get(StyleParamKey::color, p.color);
    _rule.get(StyleParamKey::order, p.order);
    _rule.get(StyleParamKey::extrude, p.extrude);

    return p;
}

void PolygonStyle::buildLine(const Line& _line, const DrawRule& _rule, const Properties& _props, VboMesh& _mesh, Tile& _tile) const {
    std::vector<PolygonVertex> vertices;

    Parameters params = parseRule(_rule);

    GLuint abgr = params.color;
    GLfloat layer = params.order;

    PolyLineBuilder builder = {
        [&](const glm::vec3& coord, const glm::vec2& normal, const glm::vec2& uv) {
            float halfWidth =  0.2f;

            glm::vec3 point(coord.x + normal.x * halfWidth, coord.y + normal.y * halfWidth, coord.z);
            vertices.push_back({ point, glm::vec3(0.0f, 0.0f, 1.0f), uv, abgr, layer });
        },
        [&](size_t sizeHint){ vertices.reserve(sizeHint); }
    };

    Builders::buildPolyLine(_line, builder);

    auto& mesh = static_cast<PolygonStyle::Mesh&>(_mesh);
    mesh.addVertices(std::move(vertices), std::move(builder.indices));
}

void PolygonStyle::buildPolygon(const Polygon& _polygon, const DrawRule& _rule, const Properties& _props, VboMesh& _mesh, Tile& _tile) const {

    std::vector<PolygonVertex> vertices;

    Parameters params = parseRule(_rule);

    GLuint abgr = params.color;
    GLfloat layer = params.order;
    auto& extrude = params.extrude;

    if (Tangram::getDebugFlag(Tangram::DebugFlags::proxy_colors)) {
        abgr = abgr << (_tile.getID().z % 6);
    }

    const static std::string key_height("height");
    const static std::string key_min_height("min_height");

    float meterScale = _tile.getScale() * MercatorProjection::METERS_AT_EQUATOR_SCALE_FACTOR;

    float height = _props.getNumeric(key_height) / meterScale;
    float minHeight = _props.getNumeric(key_min_height) / meterScale;

    PolygonBuilder builder = {
        [&](const glm::vec3& coord, const glm::vec3& normal, const glm::vec2& uv){
            vertices.push_back({ coord, normal, uv, abgr, layer });
        },
        [&](size_t sizeHint){ vertices.reserve(sizeHint); }
    };

    auto& mesh = static_cast<PolygonStyle::Mesh&>(_mesh);

    if (extrude[0] != 0.0f || extrude[1] != 0.0f) {
        height = std::isnan(extrude[1])
            ? ( std::isnan(extrude[0]) ? height : extrude[0] )
            : extrude[1];

        minHeight = std::isnan(extrude[1]) ? minHeight : extrude[0];

        Builders::buildPolygonExtrusion(_polygon, minHeight, height, builder);
        mesh.addVertices(std::move(vertices), std::move(builder.indices));

        // TODO add builder.clear() ?;
        builder.numVertices = 0;

    } else {
        height = 0.0f;
    }

    Builders::buildPolygon(_polygon, height, builder);
    mesh.addVertices(std::move(vertices), std::move(builder.indices));
}

}
