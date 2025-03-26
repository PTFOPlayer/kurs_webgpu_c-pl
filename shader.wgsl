@group(0)
@binding(0)
var<storage, read_write> x: array<f32>; 

fn times10(x: f32) -> f32 {
    var result = (x*10);
    return result;
}

@compute
@workgroup_size(1)
fn main(@builtin(global_invocation_id) global_id: vec3<u32>) {
    x[global_id.x] = times10(x[global_id.x]);
}