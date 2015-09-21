#include "style.h"

#include "scene/scene.h"
#include "scene/sceneLayer.h"
#include "scene/light.h"
#include "tile/tile.h"
#include "gl/vboMesh.h"
#include "view/view.h"

namespace Tangram {

    Style::Style(std::string _name, Blending _blendMode, GLenum _drawMode) :
    m_name(_name),
    m_blend(_blendMode),
    m_drawMode(_drawMode),
    m_contextLost(true) {
}

Style::~Style() {}

void Style::build(const std::vector<std::unique_ptr<Light>>& _lights) {

    constructVertexLayout();
    constructShaderProgram();

    switch (m_lightingType) {
        case LightingType::vertex:
            m_shaderProgram->addSourceBlock("defines", "#define TANGRAM_LIGHTING_VERTEX\n", false);
            break;
        case LightingType::fragment:
            m_shaderProgram->addSourceBlock("defines", "#define TANGRAM_LIGHTING_FRAGMENT\n", false);
            break;
        default:
            break;
    }

    m_material->injectOnProgram(*m_shaderProgram);

    for (auto& light : _lights) {
        light->injectOnProgram(*m_shaderProgram);
    }

}

void Style::setMaterial(const std::shared_ptr<Material>& _material) {

    m_material = _material;

}

void Style::setLightingType(LightingType _type){

    m_lightingType = _type;

}

void Style::buildFeature(Tile& _tile, const Feature& _feat, const DrawRule& _rule) const {

    auto& mesh = _tile.getMesh(*this);

    if (!mesh) {
        mesh.reset(newMesh());
    }

    switch (_feat.geometryType) {
        case GeometryType::points:
            for (auto& point : _feat.points) {
                buildPoint(point, _rule, _feat.props, *mesh, _tile);
            }
            break;
        case GeometryType::lines:
            for (auto& line : _feat.lines) {
                buildLine(line, _rule, _feat.props, *mesh, _tile);
            }
            break;
        case GeometryType::polygons:
            for (auto& polygon : _feat.polygons) {
                buildPolygon(polygon, _rule, _feat.props, *mesh, _tile);
            }
            break;
        default:
            break;
    }

}

void Style::onBeginDrawFrame(const View& _view, const Scene& _scene) {

    m_material->setupProgram(*m_shaderProgram);

    // Set up lights
    for (const auto& light : _scene.lights()) {
        light->setupProgram(_view, *m_shaderProgram);
    }

    m_shaderProgram->setUniformf("u_zoom", _view.getZoom());
    m_shaderProgram->setUniformf("u_resolution", _view.getWidth(), _view.getHeight());
    
    // Configure render state
    switch (m_blend) {
        case Blending::none:
            RenderState::blending(GL_FALSE);
            RenderState::blendingFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            RenderState::depthTest(GL_TRUE);
            RenderState::depthWrite(GL_TRUE);
            break;
        case Blending::add:
            RenderState::blending(GL_TRUE);
            RenderState::blendingFunc(GL_ONE, GL_ONE);
            RenderState::depthTest(GL_FALSE);
            RenderState::depthWrite(GL_TRUE);
            break;
        case Blending::multiply:
            RenderState::blending(GL_TRUE);
            RenderState::blendingFunc(GL_ZERO, GL_SRC_COLOR);
            RenderState::depthTest(GL_FALSE);
            RenderState::depthWrite(GL_TRUE);
            break;
        case Blending::overlay:
            RenderState::blending(GL_TRUE);
            RenderState::blendingFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            RenderState::depthTest(GL_FALSE);
            RenderState::depthWrite(GL_FALSE);
            break;
        case Blending::inlay:
            // TODO: inlay does not behave correctly for labels because they don't have a z position
            RenderState::blending(GL_TRUE);
            RenderState::blendingFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            RenderState::depthTest(GL_TRUE);
            RenderState::depthWrite(GL_FALSE);
            break;
        default:
            break;
    }
}

void Style::onBeginBuildTile(Tile& _tile) const {
    // No-op by default
}

void Style::onEndBuildTile(Tile& _tile) const {
    // No-op by default
}

void Style::buildPoint(const Point& _point, const DrawRule& _rule, const Properties& _props, VboMesh& _mesh, Tile& _tile) const {
    // No-op by default
}

void Style::buildLine(const Line& _line, const DrawRule& _rule, const Properties& _props, VboMesh& _mesh, Tile& _tile) const {
    // No-op by default
}

void Style::buildPolygon(const Polygon& _polygon, const DrawRule& _rule, const Properties& _props, VboMesh& _mesh, Tile& _tile) const {
    // No-op by default
}
    
bool Style::glContextLost() {
    bool contextLost = m_contextLost;
    if (m_contextLost) {
        m_contextLost = false;
    }
    return contextLost;
}

}
