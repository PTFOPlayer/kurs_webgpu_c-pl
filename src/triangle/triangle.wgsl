

struct VertexOutput {
    @builtin(position) position: vec4f,
    @location(0) local_pos: vec2f,
}

@vertex
fn vertex(@builtin(vertex_index) in_vertex_index: u32) -> VertexOutput {
    var vertices: array<vec2f, 3> = array<vec2f, 3> (
        vec2f(-0.5, -0.5),
        vec2f(0.5, -0.5),
        vec2f(0.0, 0.5)
    );

    let p = vertices[in_vertex_index];
    
    var out: VertexOutput;
    out.position = vec4f(p, 0.0, 1.0);
    out.local_pos = p;
    return out;
}

@fragment
fn fragment(in: VertexOutput) -> @location(0) vec4f {
    let color = vec4f(
        in.local_pos.x + 0.5,
        in.local_pos.y + 0.5,
        abs(in.local_pos.x - in.local_pos.y),
        1.0
    );
    return color;
}