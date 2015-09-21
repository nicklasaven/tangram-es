#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <iostream>
#include <vector>
#include <string>

#include "data/filters.h"
#include "yaml-cpp/yaml.h"
#include "scene/sceneLoader.h"
#include "scene/scene.h"
#include "util/variant.h"

using namespace Tangram;
using YAML::Node;

Scene scene;
SceneLoader sceneLoader;

TEST_CASE( "Style Uniforms Parsing and Injection Test: Float uniform value", "[StyleUniforms][core][yaml]") {

    Node node = YAML::Load(R"END(
        u_float: 0.5
        )END");

    auto uniformValues = sceneLoader.parseStyleUniform(node["u_float"], scene);
    REQUIRE(uniformValues.size() == 1);
    REQUIRE(uniformValues[0].is<float>());
    REQUIRE(uniformValues[0].get<float>() == 0.5);
    //REQUIRE(uniformValues.first == "float");
}

TEST_CASE( "Style Uniforms Parsing and Injection Test: Boolean uniform value", "[StyleUniforms][core][yaml]") {

    Node node = YAML::Load(R"END(
        u_true: true
        u_false: false
        )END");

    auto uniformValues = sceneLoader.parseStyleUniform(node["u_true"], scene);
    REQUIRE(uniformValues.size() == 1);
    REQUIRE(uniformValues[0].is<bool>());
    REQUIRE(uniformValues[0].get<bool>() == 1);
    //REQUIRE(uniformValues.first == "int");

    uniformValues = sceneLoader.parseStyleUniform(node["u_false"], scene);
    REQUIRE(uniformValues.size() == 1);
    REQUIRE(uniformValues[0].is<bool>());
    REQUIRE(uniformValues[0].get<bool>() == 0);
    //REQUIRE(uniformValues.first == "int");
}

TEST_CASE( "Style Uniforms Parsing and Injection Test: vec2, vec3, vec4 uniform value", "[StyleUniforms][core][yaml]") {

    Node node = YAML::Load(R"END(
        u_vec2: [0.1, 0.2]
        u_vec3: [0.1, 0.2, 0.3]
        u_vec4: [0.1, 0.2, 0.3, 0.4]
        )END");


    auto uniformValues = sceneLoader.parseStyleUniform(node["u_vec2"], scene);
    REQUIRE(uniformValues.size() == 1);
    REQUIRE(uniformValues[0].is<glm::vec2>());
    REQUIRE(uniformValues[0].get<glm::vec2>().x == 0.1f);
    REQUIRE(uniformValues[0].get<glm::vec2>().y == 0.2f);
    //REQUIRE(uniformValues.first == "vec2");

    uniformValues = sceneLoader.parseStyleUniform(node["u_vec3"], scene);
    REQUIRE(uniformValues.size() == 1);
    REQUIRE(uniformValues[0].is<glm::vec3>());
    REQUIRE(uniformValues[0].get<glm::vec3>().x == 0.1f);
    REQUIRE(uniformValues[0].get<glm::vec3>().y == 0.2f);
    REQUIRE(uniformValues[0].get<glm::vec3>().z == 0.3f);
    //REQUIRE(uniformValues.first == "vec3");

    uniformValues = sceneLoader.parseStyleUniform(node["u_vec4"], scene);
    REQUIRE(uniformValues.size() == 1);
    REQUIRE(uniformValues[0].is<glm::vec4>());
    REQUIRE(uniformValues[0].get<glm::vec4>().x == 0.1f);
    REQUIRE(uniformValues[0].get<glm::vec4>().y == 0.2f);
    REQUIRE(uniformValues[0].get<glm::vec4>().z == 0.3f);
    REQUIRE(uniformValues[0].get<glm::vec4>().w == 0.4f);
    //REQUIRE(uniformValues.first == "vec4");
}

TEST_CASE( "Style Uniforms Parsing and Injection Test: textures uniform value", "[StyleUniforms][core][yaml]") {

    Node node = YAML::Load(R"END(
        u_tex : texture1.png
        u_tex2 : [texture2.png, texture3.png, texture4.png]
        )END");

    auto uniformValues = sceneLoader.parseStyleUniform(node["u_tex"], scene);
    REQUIRE(uniformValues.size() == 1);
    REQUIRE(uniformValues[0].is<std::string>());
    REQUIRE(uniformValues[0].get<std::string>() == "texture1.png");
    //REQUIRE(uniformValues.first == "sampler2D");

    uniformValues = sceneLoader.parseStyleUniform(node["u_tex2"], scene);
    REQUIRE(uniformValues.size() == 3);
    REQUIRE(uniformValues[0].is<std::string>());
    REQUIRE(uniformValues[1].is<std::string>());
    REQUIRE(uniformValues[2].is<std::string>());
    REQUIRE(uniformValues[0].get<std::string>() == "texture2.png");
    REQUIRE(uniformValues[1].get<std::string>() == "texture3.png");
    REQUIRE(uniformValues[2].get<std::string>() == "texture4.png");
}

