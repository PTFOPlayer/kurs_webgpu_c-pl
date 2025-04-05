@group(0)
@binding(0)
var<storage, read_write> out: array<u32>; 
@group(0)
@binding(1)
var<storage, read> in: array<u32>;

fn collatz(in: u32) -> u32 {
    var iterations: u32 = 0;
    var n = in;
    loop {
        if (n <= 1u) {
            break;
        }
        
        iterations = iterations + 1;
        if(n % 2 == 0) {
            n = n / 2u;
        } else {
            n = (3*n) + 1;
        }
    }

    return iterations;
}

@compute
@workgroup_size(1)
fn main(@builtin(global_invocation_id) global_id: vec3<u32>) {
    out[global_id.x] = collatz(in[global_id.x]);
}