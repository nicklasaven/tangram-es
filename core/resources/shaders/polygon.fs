#pragma tangram: extensions

#ifdef GL_ES
precision highp float;
#endif

#pragma tangram: defines

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;
uniform mat3 u_normalMatrix;
uniform vec3 u_map_position;
uniform vec3 u_tile_origin;
uniform vec2 u_resolution;
uniform float u_time;
uniform float u_meters_per_pixel;
uniform float u_device_pixel_ratio;
uniform mat3 u_inverseNormalMatrix;

#pragma tangram: uniforms

varying vec4 v_world_position;
varying vec4 v_position;
varying vec4 v_color;
varying vec3 v_normal;
varying vec2 v_texcoord;

#ifdef TANGRAM_LIGHTING_VERTEX
    varying vec4 v_lighting;
#endif

#pragma tangram: material
#pragma tangram: lighting
#pragma tangram: global

vec3 worldNormal() {
    return normalize(u_inverseNormalMatrix * v_normal);
}

void main(void) {

    // Initialize globals
    #pragma tangram: setup

    vec4 color = v_color;
    vec3 normal = v_normal;

    #ifdef TANGRAM_MATERIAL_NORMAL_TEXTURE
        calculateNormal(normal);
    #endif

    // Modify normal before lighting
    #pragma tangram: normal

    // Modify color and material properties before lighting
    #ifndef TANGRAM_LIGHTING_VERTEX
        #pragma tangram: color
    #endif

    #ifdef TANGRAM_LIGHTING_FRAGMENT
        color = calculateLighting(v_position.xyz, normal, color);
    #else
        #ifdef TANGRAM_LIGHTING_VERTEX
            color = v_lighting;
        #endif
    #endif

    // Modify color after lighting (filter-like effects that don't require a additional render passes)
    #pragma tangram: filter

    //color.rgb = pow(color.rgb, vec3(1.0/2.2)); // gamma correction
    gl_FragColor = color;
}
