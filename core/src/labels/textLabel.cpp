#include "labels/textLabel.h"

namespace Tangram {

TextLabel::TextLabel(Label::Transform _transform, Type _type, glm::vec2 _dim,
                     TextBuffer& _mesh, Range _vertexRange, Label::Options _options)
    : Label(_transform, _dim, _type, static_cast<LabelMesh&>(_mesh), _vertexRange, _options)
{}

void TextLabel::updateBBoxes(float _zoomFract) {
    glm::vec2 t = glm::vec2(cos(m_transform.state.rotation), sin(m_transform.state.rotation));
    glm::vec2 tperp = glm::vec2(-t.y, t.x);
    glm::vec2 obbCenter;

    obbCenter = m_transform.state.screenPos + t * m_dim.x * 0.5f - tperp * (m_dim.y / 8);

    m_obb = OBB(obbCenter.x, obbCenter.y, m_transform.state.rotation, m_dim.x, m_dim.y);
    m_aabb = m_obb.getExtent();
}

void TextLabel::align(glm::vec2& _screenPosition, const glm::vec2& _ap1, const glm::vec2& _ap2) {
    glm::vec2 ap1ap2 = _ap2 - _ap1;
    glm::vec2 t = glm::normalize(-ap1ap2);

    switch (m_type) {
        case Type::debug:
        case Type::point:
            _screenPosition.x -= m_dim.x * 0.5f;
            break;
        case Type::line:
            _screenPosition = (_ap1 + _ap2) * 0.5f + t * m_dim.x * 0.5f;
            break;
    }

}

}
