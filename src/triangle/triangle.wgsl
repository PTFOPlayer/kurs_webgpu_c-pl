

@vertex
fn vertex(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4f {
    var vertices: array<vec2f, 3> = array<vec2f, 3> (
        vec2f(-0.5, -0.5),
        vec2f(0.5, -0.5),
        vec2f(0.0, 0.5)
    );

    let p = vertices[in_vertex_index];
    return vec4f(p, 0.0, 1.0);
}

@fragment
fn fragment() -> @location(0) vec4f {
    return vec4f(0.0, 0.0, 1.0, 1.0);
}