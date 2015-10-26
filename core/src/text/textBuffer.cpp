#include "textBuffer.h"

#include "labels/textLabel.h"
#include "gl/texture.h"
#include "gl/vboMesh.h"

namespace Tangram {

TextBuffer::TextBuffer(std::shared_ptr<VertexLayout> _vertexLayout)
    : LabelMesh(_vertexLayout, GL_TRIANGLES) {
    m_dirtyTransform = false;
    addVertices({}, {});
}

TextBuffer::~TextBuffer() {
}

std::vector<TextBuffer::WordBreak> TextBuffer::findWordBreaks(const std::string& _text) {
    std::vector<WordBreak> breaks;

    for (int i = 0, quad = 0; i < _text.size(); ++i) {
        if (std::isspace(_text[i])) {
            int lookAHead = i;
            while (lookAHead++ < _text.size() && !std::isspace(_text[lookAHead])) {}
            breaks.push_back({int(quad + breaks.size() + 1), lookAHead});
        } else {
            quad++;
        }
    }

    return breaks;
}

bool TextBuffer::addLabel(const TextStyle::Parameters& _params, Label::Transform _transform,
                          Label::Type _type, FontContext& _fontContext) {

    if (_params.fontId < 0 || _params.fontSize <= 0.f) {
        return false;
    }

    if (!_fontContext.lock()) {
        return false;
    }

    /// Apply text transforms
    const std::string* renderText;
    std::string text;

    if (_params.transform == TextTransform::none) {
        renderText = &_params.text;
    } else {
        text = _params.text;
        std::locale loc;

        // perfom text transforms
        switch (_params.transform) {
            case TextTransform::capitalize:
                text[0] = toupper(text[0], loc);
                if (text.size() > 1) {
                    for (size_t i = 1; i < text.length(); ++i) {
                        if (text[i - 1] == ' ') {
                            text[i] = std::toupper(text[i], loc);
                        }
                    }
                }
                break;
            case TextTransform::lowercase:
                for (size_t i = 0; i < text.length(); ++i) {
                    text[i] = std::tolower(text[i], loc);
                }
                break;
            case TextTransform::uppercase:
                // TOOD : use to wupper when any wide character is detected
                for (size_t i = 0; i < text.length(); ++i) {
                    text[i] = std::toupper(text[i], loc);
                }
                break;
            default:
                break;
        }
        renderText = &text;
    }

    /// Rasterize the glyphs
    std::vector<FONSquad>& quads = _fontContext.rasterize(*renderText, _params.fontId,
                                                          _params.fontSize,
                                                          _params.blurSpread);
    size_t numGlyphs = quads.size();

    if (numGlyphs == 0) {
        _fontContext.unlock();
        return false;
    }

    auto& vertices = m_vertices[0];
    int vertexOffset = vertices.size();
    int numVertices = numGlyphs * 4;

    // Stroke width is normalized by the distance of the SDF spread, then scaled
    // to a char, then packed into the "alpha" channel of stroke. The .25 scaling
    // probably has to do with how the SDF is generated, but honestly I'm not sure
    // what it represents.
    uint32_t strokeWidth = _params.strokeWidth / _params.blurSpread * 255. * .25;
    uint32_t stroke = (_params.strokeColor & 0x00ffffff) + (strokeWidth << 24);

    auto breaks = findWordBreaks(_params.text);
    const auto& metrics = _fontContext.getMetrics();

    float yOffset = 0.f, xOffset = 0.f;
    float yPadding = metrics.lineHeight;
    glm::vec2 bbox;

    int nLine = 1, lastBreak = 0;

    /// Generate the quads
    for (int i = 0; i < quads.size(); ++i) {
        auto& q = quads[i];

        // Apply word wrapping based on the word breaks
        if (i > 1) {
            for (auto& b : breaks) {
                if (i == b.start && b.end - lastBreak > 15) {
                    auto& previousQuad = quads[i - 1];
                    float spaceLength = q.x0 - previousQuad.x1;

                    yOffset += yPadding;
                    xOffset -= q.x0 + quads[0].x0 * 0.5f - spaceLength;

                    lastBreak = b.start;
                    nLine++;
                }
            }
        }

        q.x0 += xOffset;
        q.x1 += xOffset;
        q.y0 += yOffset;
        q.y1 += yOffset;

        vertices.push_back({{q.x0, q.y0}, {q.s0, q.t0}, _params.fill, stroke});
        vertices.push_back({{q.x0, q.y1}, {q.s0, q.t1}, _params.fill, stroke});
        vertices.push_back({{q.x1, q.y0}, {q.s1, q.t0}, _params.fill, stroke});
        vertices.push_back({{q.x1, q.y1}, {q.s1, q.t1}, _params.fill, stroke});

        // Adjust the bounding box on x
        bbox.x = std::max(bbox.x, q.x1);
    }

    // Adjust the bounding box on y
    bbox.y = metrics.lineHeight * nLine;

    _fontContext.unlock();

    m_labels.emplace_back(new TextLabel(_transform, _type, bbox, *this,
                                        { vertexOffset, numVertices }, _params.labelOptions));

    // TODO: change this in TypeMesh::adVertices()
    m_nVertices = vertices.size();

    return true;
}

}
