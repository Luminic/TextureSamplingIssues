#version 450

uniform sampler2D textures[2];

layout (binding = 0, rgba32f) uniform image2D framebuffer;

layout (local_size_x = 32, local_size_y = 32) in;

void main() {
    ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size = imageSize(framebuffer);
    if (pix.x >= size.x || pix.y >= size.y) {
        return;
    }
    vec2 tex_coords = vec2(pix)/size;

    int index;
    vec4 col;
    if (tex_coords.x > tex_coords.y) {
        index = 0;
    } else {
        index = 1;
    }

    /* This works */
    // for (int i=0; i<=index; i++)
    //     if (i==index)
    //         col = textureLod(textures[index], vec2(0,0), 0);

    /* These doesn't */
    col = textureLod(textures[index], vec2(0,0), 0);
    // col = texelFetch(textures[index], ivec2(0,0), 0);

    imageStore(framebuffer, pix, col);
}
